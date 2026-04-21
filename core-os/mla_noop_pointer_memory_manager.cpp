//
// Created by chris on 4/21/2026.
//

#include "mla_data_types.h"

mla_pointer_t __noop_pointer_memory_manager_malloc(mla_pointer_memory_manager_t& memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {

    // not supported
    return mla_pointer_null();
}

mla_bool_t __noop_pointer_memory_manager_is_null(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    return payload.asPointer == nullptr;
}

mla_platform_pointer_t __noop_pointer_memory_manager_get_platform_pointer(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    return payload.asPointer;
}

void __noop_pointer_memory_manager_incReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    (void)payload;
}

void __noop_pointer_memory_manager_decReferences(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {

    (void)memory_manager;
    (void)payload;
}

mla_int32_t __noop_pointer_memory_manager_get_ref_count(const mla_pointer_memory_manager_t & memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    (void)payload;
    return -1; // Not supported
}

// This Memory Manager only exists for external resource. We dont do any management here
// is just a bridge between the worlds
mla_pointer_memory_manager_t g_noop_pointer_memory_manager = {
    __noop_pointer_memory_manager_malloc,
    __noop_pointer_memory_manager_is_null,
    __noop_pointer_memory_manager_get_platform_pointer,
    __noop_pointer_memory_manager_incReferences,
    __noop_pointer_memory_manager_decReferences,
    __noop_pointer_memory_manager_get_ref_count
};


mla_pointer_t mla_platform_pointer_to_managed_pointer(mla_platform_pointer_t resource) {

    if (resource == nullptr) {
        return mla_pointer_null();
    }

    return {
        mla_dynamic_data_from_pointer(resource),
        &g_noop_pointer_memory_manager
    };
}