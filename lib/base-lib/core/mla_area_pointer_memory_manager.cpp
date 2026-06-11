//
// Created by Jules.
//

#include "mla_area_pointer_memory_manager.h"

struct mla_area_pointer_header_t {
    mla_area_page_header_t* page;
    mla_size_t itemSize;
    mla_pointer_cleanup_hook_t cleanupHook;
    mla_dynamic_data_t cleanupHookUserData;
};

/**
 * @brief Aligns a size up to the nearest multiple of 8.
 *
 * 8-byte alignment is a common requirement for modern CPUs to perform
 * efficient memory access and to avoid alignment faults on certain
 * architectures.
 *
 * Logic: (size + 7) & ~7
 * Adding 7 ensures that any value not already a multiple of 8 moves
 * into the next "8-byte block" range. The bitwise AND with ~7 (which
 * is ...11111000 in binary) then clears the lower 3 bits, effectively
 * rounding down to the start of that block, which is the nearest
 * multiple of 8.
 */
static inline mla_size_t mla_internal_area_align_up(mla_size_t size) {
    return (size + 7) & ~static_cast<mla_size_t>(7);
}

static void mla_internal_area_lock(mla_atomic_int32_t& lock) {
    while (!mla_atomic_compare_exchange(lock, 0, 1)) {
        // Spin
    }
}

static void mla_internal_area_unlock(mla_atomic_int32_t& lock) {
    mla_atomic_exchange(lock, 0);
}

static mla_area_page_header_t* mla_internal_area_allocate_page(mla_size_t size) {

    mla_platform_pointer_t rawPtr = mla_platform_malloc(size);
    if (rawPtr == nullptr) {
        return nullptr;
    }
    mla_memset(rawPtr, 0, size);
    mla_area_page_header_t* page = reinterpret_cast<mla_area_page_header_t*>(rawPtr);
    page->page_size = size;
    page->refCount = 0;
    page->OtherTaskRefCount.value = 0;
    page->CurrentPosition.value = (mla_int32_t)mla_internal_area_align_up(sizeof(mla_area_page_header_t));
    page->NextPage = nullptr;
    page->creatorTaskId = mla_current_task_id;
    return page;
}

mla_pointer_t mla_internal_area_pointer_memory_manager_malloc(mla_pointer_memory_manager_t& memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {
    (void)filename;
    (void)function_name;
    mla_area_pointer_memory_manager_t& area_manager = reinterpret_cast<mla_area_pointer_memory_manager_t&>(memory_manager);

    mla_size_t headerSize = mla_internal_area_align_up(sizeof(mla_area_pointer_header_t));
    mla_size_t totalNeeded = headerSize + mla_internal_area_align_up(size);

    while (true) {
        mla_area_page_header_t* page = area_manager.currentPage;

        if (page != nullptr) {
            // Try lock-free reservation of space in the current page
            mla_int32_t currentPos = page->CurrentPosition.value;
            while ((mla_size_t)currentPos + totalNeeded <= page->page_size) {

                if (mla_atomic_compare_exchange(page->CurrentPosition, currentPos, currentPos + (mla_int32_t)totalNeeded)) {
                    // Success! Reserved space.
                    mla_platform_pointer_t itemPtr = reinterpret_cast<mla_byte_t*>(page) + currentPos;

                    mla_area_pointer_header_t* header = reinterpret_cast<mla_area_pointer_header_t*>(itemPtr);
                    header->page = page;
                    header->itemSize = totalNeeded;
                    header->cleanupHook = cleanup_hook;
                    header->cleanupHookUserData = cleanup_data;

                    return {
                        mla_dynamic_data_from_pointer(itemPtr),
                        &memory_manager
                    };
                }
                // Contention or update happened, retry with new currentPos
                currentPos = page->CurrentPosition.value;
            }
        }

        // Need new page
        mla_internal_area_lock(area_manager.lock);
        // Double check if page changed while waiting for lock
        if (area_manager.currentPage == page) {
            mla_size_t pageSize = area_manager.defaultPageSize;
            if (totalNeeded + mla_internal_area_align_up(sizeof(mla_area_page_header_t)) > pageSize) {
                pageSize = totalNeeded + mla_internal_area_align_up(sizeof(mla_area_page_header_t));
            }
            mla_area_page_header_t* newPage = mla_internal_area_allocate_page(pageSize);
            if (newPage == nullptr) {
                mla_internal_area_unlock(area_manager.lock);
                return mla_pointer_null();
            }
            newPage->NextPage = area_manager.currentPage;
            area_manager.currentPage = newPage;
        }
        mla_internal_area_unlock(area_manager.lock);
        // Continue loop to allocate from the newly created page
    }
}

mla_platform_pointer_t mla_internal_area_pointer_memory_manager_get_platform_pointer(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    if (payload.asPointer == nullptr) {
        return nullptr;
    }
    mla_size_t headerSize = mla_internal_area_align_up(sizeof(mla_area_pointer_header_t));
    return static_cast<mla_byte_t*>(payload.asPointer) + headerSize;
}

void mla_internal_area_pointer_memory_manager_incReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    mla_area_pointer_header_t* header = reinterpret_cast<mla_area_pointer_header_t*>(payload.asPointer);
    if (header == nullptr) return;

    // Increment page ref count
    mla_area_page_header_t* page = header->page;
    if (page->creatorTaskId == mla_current_task_id) {
        page->refCount++;
    } else {
        mla_atomic_increment(page->OtherTaskRefCount);
    }
}

static void mla_internal_area_free_page(mla_area_pointer_memory_manager_t& area_manager, mla_area_page_header_t* page) {

    mla_size_t currentPos = mla_internal_area_align_up(sizeof(mla_area_page_header_t));
    mla_size_t headerSize = mla_internal_area_align_up(sizeof(mla_area_pointer_header_t));

    mla_int32_t finalPos = page->CurrentPosition.value;
    while (currentPos < (mla_size_t)finalPos) {
        mla_area_pointer_header_t* header = reinterpret_cast<mla_area_pointer_header_t*>(reinterpret_cast<mla_byte_t*>(page) + currentPos);
        if (header->cleanupHook != nullptr) {
            mla_platform_pointer_t data = reinterpret_cast<mla_byte_t*>(header) + headerSize;
            header->cleanupHook(data, header->cleanupHookUserData);
        }
        currentPos += header->itemSize;
    }

    // Remove from list
    mla_area_page_header_t* curr = area_manager.currentPage;
    mla_area_page_header_t* prev = nullptr;

    while(curr != nullptr) {
        if (curr == page) {
            if (prev == nullptr) {
                area_manager.currentPage = curr->NextPage;
            } else {
                prev->NextPage = curr->NextPage;
            }
            break;
        }
        prev = curr;
        curr = curr->NextPage;
    }

    mla_platform_free(page);
}

void mla_internal_pointer_memory_manager_decReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    mla_area_pointer_memory_manager_t& area_manager = reinterpret_cast<mla_area_pointer_memory_manager_t&>(memory_manager);
    mla_area_pointer_header_t* header = reinterpret_cast<mla_area_pointer_header_t*>(payload.asPointer);
    if (header == nullptr) return;

    mla_area_page_header_t* page = header->page;

    mla_internal_area_lock(area_manager.lock);

    mla_int32_t page_remaining_ref_count;
    if (page->creatorTaskId == mla_current_task_id) {
        page_remaining_ref_count = (--page->refCount) + page->OtherTaskRefCount.value;
    } else {
        page_remaining_ref_count = mla_atomic_decrement(page->OtherTaskRefCount) + page->refCount;
    }

    if (page_remaining_ref_count == 0) {
        mla_internal_area_free_page(area_manager, page);
    }

    mla_internal_area_unlock(area_manager.lock);
}

mla_int32_t mla_internal_area_pointer_memory_manager_get_ref_count(const mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    mla_area_pointer_header_t* header = reinterpret_cast<mla_area_pointer_header_t*>(payload.asPointer);
    if (header == nullptr)
        return -1;

    return header->page->refCount + header->page->OtherTaskRefCount.value;
}

mla_area_pointer_memory_manager_t mla_area_pointer_memory_manager_create(mla_size_t p_PageSize) {

    return  {
        {
            mla_internal_area_pointer_memory_manager_malloc,
            mla_internal_area_pointer_memory_manager_get_platform_pointer,
            mla_internal_area_pointer_memory_manager_incReferences,
            mla_internal_pointer_memory_manager_decReferences,
            mla_internal_area_pointer_memory_manager_get_ref_count
        },
         nullptr,
         p_PageSize,
         {0}
    };

}
