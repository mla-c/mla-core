//
// Created by chris on 4/22/2026.
//

#include "mla_data_types.h"
#include "task/mla_atomic.h"

struct mla_native_resource_item_t {
    mla_atomic_int32_t refCount;
    mla_native_resource_clean_up_hook_t cleanupHook;
    mla_native_resource_t native_resource;
};

mla_pointer_t __mla_native_resource_pointer_memory_manager_malloc(mla_pointer_memory_manager_t& memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {
    (void)memory_manager;
    (void)size;
    (void)cleanup_hook;
    (void)cleanup_data;
    (void)filename;
    (void)function_name;
    //mla_error(mla_string_concat("Noop Pointer Memory Manager does not support malloc operation. File ", mla_string(filename), mla_string(function_name));
    // not supported
    return mla_pointer_null();
}

mla_platform_pointer_t __mla_native_resource_pointer_memory_manager_get_platform_pointer(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    return payload.asPointer;
}

void __mla_native_resource_pointer_memory_manager_incReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;

    mla_native_resource_item_t* item = reinterpret_cast<mla_native_resource_item_t*>(payload.asPointer);

    if (item == nullptr) {
        return;
    }

    mla_atomic_increment(item->refCount);

}

void __mla_native_resource_pointer_memory_manager_decReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {

    (void)memory_manager;

    mla_native_resource_item_t* item = reinterpret_cast<mla_native_resource_item_t*>(payload.asPointer);

    if (item == nullptr) {
        return;
    }


    if (mla_atomic_decrement(item->refCount) == 0) {

        // Call the cleanup hook if it is set
        if (item->cleanupHook != nullptr) {
            item->cleanupHook(item->native_resource);
        }

        mla_platform_free(payload.asPointer);
    }

}

mla_int32_t __mla_native_resource_pointer_memory_manager_get_ref_count(const mla_pointer_memory_manager_t & memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;

    mla_native_resource_item_t* item = reinterpret_cast<mla_native_resource_item_t*>(payload.asPointer);

    if (item == nullptr) {
        return 0;
    }

    return item->refCount.value;

}

// This Memory Manager only exists for external resource. We manage this resource if the pointer goes out of scope
// we execute the cleanup
mla_pointer_memory_manager_t g_mla_native_resource_pointer_memory_manager = {
    __mla_native_resource_pointer_memory_manager_malloc,
    __mla_native_resource_pointer_memory_manager_get_platform_pointer,
    __mla_native_resource_pointer_memory_manager_incReferences,
    __mla_native_resource_pointer_memory_manager_decReferences,
    __mla_native_resource_pointer_memory_manager_get_ref_count
};


mla_pointer_t mla_native_resource_to_managed_pointer(mla_native_resource_t& resource, mla_native_resource_clean_up_hook_t cleanup_hook) {

    mla_native_resource_item_t* item = reinterpret_cast<mla_native_resource_item_t*>(mla_platform_malloc(sizeof(mla_native_resource_item_t)));

    if (item == nullptr) {
        return mla_pointer_null();
    }

    mla_memset(item, 0, sizeof(mla_native_resource_item_t));
    item->cleanupHook = cleanup_hook;
    item->native_resource = resource;
    return {
        mla_dynamic_data_from_pointer(item),
        &g_mla_native_resource_pointer_memory_manager
    };
}

mla_native_resource_t* mla_native_resource_from_managed_pointer(mla_pointer_t pointer) {

    if (pointer.memoryManager != &g_mla_native_resource_pointer_memory_manager) {
        return nullptr;
    }

    mla_native_resource_item_t* item = reinterpret_cast<mla_native_resource_item_t*>(pointer.payload.asPointer);

    if (item == nullptr) {
        return nullptr;
    }

    return &item->native_resource;

}

mla_native_resource_t mla_native_resource_empty() {
    mla_native_resource_t data = {};
    data.asInt64 = 0;
    return data;
}