//
// Created by chris on 4/21/2026.
//

#include "mla_data_types.h"
#include "log/mla_logging.h"
#include "system/mla_string_concat.h"

mla_pointer_t __noop_pointer_memory_manager_malloc(mla_pointer_memory_manager_t& memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {
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

mla_platform_pointer_t __noop_pointer_memory_manager_get_platform_pointer(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    return payload.asPointer;
}

mla_int32_t __noop_pointer_memory_manager_get_ref_count(const mla_pointer_memory_manager_t & memory_manager, mla_dynamic_data_t payload) {
    (void)memory_manager;
    (void)payload;
    return -1; // Not supported
}

// This Memory Manager only exists for external heap data. We dont do any management here
// is just a bridge between the worlds
mla_pointer_memory_manager_t g_noop_pointer_memory_manager = {
    __noop_pointer_memory_manager_malloc,
    __noop_pointer_memory_manager_get_platform_pointer,
    nullptr,
    nullptr,
    __noop_pointer_memory_manager_get_ref_count
};


mla_pointer_t mla_platform_pointer_to_managed_pointer(const mla_platform_pointer_t resource) {

    if (resource == nullptr) {
        return mla_pointer_null();
    }

    return {
        mla_dynamic_data_from_pointer(const_cast<mla_platform_pointer_t>(resource)),
        &g_noop_pointer_memory_manager
    };
}