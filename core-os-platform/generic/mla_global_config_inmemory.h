//
// Created by christian on 10/2/25.
//

#ifndef MLA_MLA_GLOBAL_CONFIG_INMEMORY_H
#define MLA_MLA_GLOBAL_CONFIG_INMEMORY_H

// In-memory configuration storage
#include "../../core-os/config/mla_config.h"

#if !defined(mla_max_config_size)
#define mla_max_config_size (4 * 1024) // 4KB is default
#endif

static mla_byte_t g_config_storage[mla_max_config_size];
static mla_size_t g_used_config_size = 0;


mla_bytes_t __generic_inmemory_read_config_input() {

    if (g_used_config_size == 0) {
        return mla_bytes_empty();
    }

    return  mla_bytes_from_external_buffer(mla_platform_pointer_to_managed_pointer(g_config_storage), g_used_config_size);
}

mla_bytes_t __generic_inmemory_create_config_output_buffer() {

    return mla_bytes(mla_max_config_size);
}

mla_bool_t __generic_inmemory_commit_config_output(mla_bytes_t& output, mla_size_t unused_bytes) {

    if (unused_bytes > mla_max_config_size) {
        return false;
    }

    mla_memcpy(g_config_storage, mla_bytes_get_data_for_writing(output), unused_bytes);
    g_used_config_size = unused_bytes;
    return true;
}

mla_bool_t __generic_inmemory_config_reset() {

    mla_memset(g_config_storage, 0, mla_max_config_size);
    g_used_config_size = 0;
    return true;
}

mla_config_low_level_operations_t g_config_low_level_operations = {
    __generic_inmemory_read_config_input,
    __generic_inmemory_create_config_output_buffer,
    __generic_inmemory_commit_config_output,
    __generic_inmemory_config_reset
};

#endif
