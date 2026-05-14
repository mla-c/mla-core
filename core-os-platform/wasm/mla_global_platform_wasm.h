//
// Created by christian on 8/12/25.
//

#ifndef MLA_MLA_GLOBAL_PLATFORM_WASM_H
#define MLA_MLA_GLOBAL_PLATFORM_WASM_H

#include "../../core-os/mla_data_types.h"
#include <unistd.h>
#include <time.h>
#include "../generic/mla_global_platform_generic.h"
#include "../../core-os/lifecycle/mla_lifecycle_events.h"


void __wasm_sleep(mla_uint32_t milliseconds) {

    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

mla_uint64_t __wasm_system_time_ms() {
    struct timespec ts;
    // Uses a faster, less precise clock source usually sufficient for millisecond-level timing
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (mla_uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access ={
    __generic_memcpy,
    __generic_memset,
    __generic_memcmp,
    __generic_memmove,
    __generic_strcpy,
    __generic_strlen,
    __generic_strstr,
    __generic_malloc,
    __generic_free,
    __generic_is_gcc_pointer,
    __generic_on_malloc_failure,
    __generic_print,
    __generic_std_read,
    __generic_strtod,
    __generic_strtoll,
    __generic_strtoull,
    __wasm_sleep,
    __wasm_system_time_ms
};


void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}


#endif
