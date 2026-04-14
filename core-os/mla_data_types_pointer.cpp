//
// Created by chris on 4/14/2026.
//

#include "mla_data_types.h"
#include "task/mla_atomic.h"

struct mla_pointer_header_t {
    mla_atomic_int32_t refCount;
    mla_pointer_cleanup_hook_t cleanupHook;
    mla_dynamic_data_t cleanupHookUserData;
};

mla_pointer_t mla_pointer_null() {
    return {
        nullptr
    };
}

mla_bool_t mla_pointer_is_null(const mla_pointer_t& ptr) {
    return ptr.headData == nullptr;
}

mla_platform_pointer_t mla_pointer_get_platform_pointer(const mla_pointer_t& ptr) {

    if (ptr.headData == nullptr) {
        return nullptr;
    }

    return ptr.headData + sizeof(mla_platform_pointer_t);

}

mla_pointer_header_t* __mla_pointer_get_header(const mla_pointer_t& ptr) {
    if (ptr.headData == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<mla_pointer_header_t*>(ptr.headData);
}

mla_pointer_header_t* __mla_pointer_get_header(mla_pointer_t* ptr) {
    if (ptr->headData == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<mla_pointer_header_t*>(ptr->headData);
}

mla_pointer_header_t* __mla_pointer_get_header(mla_uint8_t* headData) {
    if (headData == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<mla_pointer_header_t*>(headData);
}

mla_int32_t mla_pointer_ref_count(const mla_pointer_t& ptr) {

    mla_pointer_header_t* header = __mla_pointer_get_header(ptr);

    if (header) {
        return header->refCount.value;
    }

    return 0;
}


mla_pointer_t::mla_pointer_t(const mla_pointer_t& p_Other) : headData(p_Other.headData) {

    mla_pointer_header_t* header = __mla_pointer_get_header(this);

    if (header) {
        mla_atomic_increment(header->refCount);
    }
}

mla_pointer_t::mla_pointer_t(mla_uint8_t* headData) : headData(headData) {

    mla_pointer_header_t* header = __mla_pointer_get_header(headData);

    if (header) {
        mla_atomic_increment(header->refCount);
    }
}

void __mla_pointer_destroy(mla_uint8_t* headData) {

    mla_pointer_header_t* header = __mla_pointer_get_header(headData);

    if (header) {
        if (mla_atomic_decrement(header->refCount) == 0) {

            mla_platform_pointer_t l_Data = headData + sizeof(mla_platform_pointer_t);

            // Call the cleanup hook if it is set
            if (header->cleanupHook != nullptr) {
                header->cleanupHook(l_Data, header->cleanupHookUserData);
            }

            mla_platform_free(headData);
        }
    }
}


mla_pointer_t::~mla_pointer_t() {
    __mla_pointer_destroy(headData);
    headData = nullptr;

}

mla_pointer_t& mla_pointer_t::operator=(const mla_pointer_t& p_Other) {

    if (this != &p_Other) {
        // Destroy current buffer

        // Because by destroying the current buffer is can happend
        // that the p_Other is Zeroed out.
        // This happends if there are cyclic references. link in a linked list.
        // So we need to backup the buffer pointer before destroying it.
        // It will not destroy the buffer of p_Other, because it is a reference.
        // Only the pointer for holding the reference is destroyed.
        mla_uint8_t* backupBuffer = p_Other.headData;

        __mla_pointer_destroy(headData); // Destroy current buffer

        // Assign new buffer
        headData = backupBuffer; // Assign new buffer

        mla_pointer_header_t* header = __mla_pointer_get_header(headData);

        // Increment reference count of the new buffer
        if (header) {
            mla_atomic_increment(header->refCount);
        }
    }
    return *this;
}

mla_pointer_t mla_malloc_with_check(mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {

    if (size == 0) {
        return mla_pointer_null();
    }

    mla_size_t totalSize = sizeof(mla_pointer_header_t) + size;
    mla_platform_pointer_t rawPtr = mla_platform_malloc_with_check(totalSize, filename, function_name);
    if (rawPtr == nullptr) {
        return mla_pointer_null();
    }

    mla_memset(rawPtr, 0, totalSize);

    // Initialize header
    mla_pointer_header_t* header = reinterpret_cast<mla_pointer_header_t*>(rawPtr);
    header->cleanupHook = cleanup_hook;
    header->cleanupHookUserData = cleanup_data;

    return {
        static_cast<mla_uint8_t*>(rawPtr)
    };}