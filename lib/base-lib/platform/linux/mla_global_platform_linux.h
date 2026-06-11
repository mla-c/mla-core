//
// Created by chris on 8/10/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_LINUX_H
#define MLA_GLOBAL_PLATFORM_LINUX_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <unistd.h>
#include <time.h>

#if mla_use_fast_float == 1

// Using fast-float library for optimized float parsing
// https://github.com/fastfloat/fast_float
// Is found about 40% faster than standard library
// the code size is growing by about 15KB

#include "../generic/mla_global_platform_fast_float.h"

#define mla_platform_strtod __fast_float_strtod
#define mla_platform_strtoll __fast_float_strtoll
#define mla_platform_strtoull __fast_float_strtoull

#else

#define mla_platform_strtod mla_internal_generic_strtod
#define mla_platform_strtoll mla_internal_generic_strtoll
#define mla_platform_strtoull mla_internal_generic_strtoull

#endif


void __linux_sleep(mla_uint32_t milliseconds) {

    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

mla_uint64_t __linux_system_time_ms() {

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
        mla_platform_strtod,
        mla_platform_strtoll,
        mla_platform_strtoull,
    __linux_sleep,
    __linux_system_time_ms
};


void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif
