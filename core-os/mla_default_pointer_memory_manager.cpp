//
// Created by chris on 4/20/2026.
//

#include "mla_data_types.h"
#include "task/mla_atomic.h"


struct mla_pointer_header_t {
    mla_atomic_int32_t refCount;
    mla_pointer_cleanup_hook_t cleanupHook;
    mla_dynamic_data_t cleanupHookUserData;
};

mla_pointer_header_t* __mla_pointer_get_header(mla_dynamic_data_t& payload) {

    if (payload.asPointer == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<mla_pointer_header_t*>(payload.asPointer);
}

mla_pointer_t __default_pointer_memory_manager_malloc(mla_pointer_memory_manager_t& memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {

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
        mla_dynamic_data_from_pointer(rawPtr),
        &memory_manager
    };
}

mla_bool_t __default_pointer_memory_manager_is_null(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    return payload.asPointer == nullptr;
}

mla_platform_pointer_t __default_pointer_memory_manager_get_platform_pointer(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    return static_cast<mla_byte_t*>(payload.asPointer) + sizeof(mla_platform_pointer_t);
}

void __default_pointer_memory_manager_incReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;

    mla_pointer_header_t* header = __mla_pointer_get_header(payload);
    if (header == nullptr) {
        return;
    }

    mla_atomic_increment(header->refCount);
}

void __default_pointer_memory_manager_decReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {

    (void)memory_manager;

    mla_pointer_header_t* header = __mla_pointer_get_header(payload);

    if (header) {
        if (mla_atomic_decrement(header->refCount) == 0) {

            mla_platform_pointer_t l_Data = static_cast<mla_byte_t*>(payload.asPointer) + sizeof(mla_platform_pointer_t);

            // Call the cleanup hook if it is set
            if (header->cleanupHook != nullptr) {
                header->cleanupHook(l_Data, header->cleanupHookUserData);
            }

            mla_platform_free(payload.asPointer);
        }
    }
}

mla_int32_t __default_pointer_memory_manager_get_ref_count(const mla_pointer_memory_manager_t & memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;

    mla_pointer_header_t* header = __mla_pointer_get_header(payload);
    if (header == nullptr) {
        return -1; // Not supported
    }

    return header->refCount.value;
}

mla_pointer_memory_manager_t g_default_pointer_memory_manager = {
    __default_pointer_memory_manager_malloc,
    __default_pointer_memory_manager_is_null,
    __default_pointer_memory_manager_get_platform_pointer,
    __default_pointer_memory_manager_incReferences,
    __default_pointer_memory_manager_decReferences,
    __default_pointer_memory_manager_get_ref_count
};