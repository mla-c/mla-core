//
// Created by christian on 8/12/25.
//

#ifndef MLA_GLOBAL_PLATFORM_WASM_H
#define MLA_GLOBAL_PLATFORM_WASM_H

#include "../../core/mla_data_types.h"
#include <unistd.h>
#include <time.h>
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"


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
    mla_internal_generic_memcpy,
    mla_internal_generic_memset,
    mla_internal_generic_memcmp,
    mla_internal_generic_memmove,
    mla_internal_generic_strlen,
    mla_internal_generic_strstr,
    mla_internal_generic_malloc,
    mla_internal_generic_free,
    mla_internal_generic_on_malloc_failure,
    mla_internal_generic_print,
    mla_internal_generic_std_read,
    mla_internal_generic_strtod,
    mla_internal_generic_strtoll,
    mla_internal_generic_strtoull,
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
