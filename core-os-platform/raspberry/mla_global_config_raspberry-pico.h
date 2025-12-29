//
// Created by chris on 12/29/2025.
//

#ifndef COREOS_MLA_GLOBAL_CONFIG_RASPBERRY_PICO_H
#define COREOS_MLA_GLOBAL_CONFIG_RASPBERRY_PICO_H

// In-memory configuration storage for Raspberry Pi Pico
// Similar to other embedded platforms (ESP, WASM)
#include "../../core-os/config/mla_config.h"

#if !defined(mla_max_config_size)
#define mla_max_config_size (4 * 1024) // 4KB is default for embedded devices
#endif

static mla_byte_t g_config_storage[mla_max_config_size];
static mla_size_t g_used_config_size = 0;

mla_bytes_t __pico_inmemory_read_config_input() {

    if (g_used_config_size == 0) {
        return mla_bytes_empty();
    }

    return mla_bytes_from_external_buffer(g_config_storage, g_used_config_size);
}

mla_bytes_t __pico_inmemory_create_config_output_buffer() {

    return mla_bytes(mla_max_config_size);
}

mla_bool_t __pico_inmemory_commit_config_output(mla_bytes_t& output, mla_size_t unused_bytes) {

    if (unused_bytes > mla_max_config_size) {
        return false;
    }

    mla_memcpy(g_config_storage, mla_bytes_get_data_for_writing(output), unused_bytes);
    g_used_config_size = unused_bytes;
    return true;
}

mla_bool_t __pico_inmemory_config_reset() {

    mla_memset(g_config_storage, 0, mla_max_config_size);
    g_used_config_size = 0;
    return true;
}

mla_config_low_level_operations_t g_config_low_level_operations = {
    __pico_inmemory_read_config_input,
    __pico_inmemory_create_config_output_buffer,
    __pico_inmemory_commit_config_output,
    __pico_inmemory_config_reset
};

#endif
