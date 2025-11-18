#ifndef COREOS_MLA_GLOBAL_PLATFORM_WINDOWS_H
#define COREOS_MLA_GLOBAL_PLATFORM_WINDOWS_H

#include "../../core-os/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core-os/lifecycle/mla_lifecycle_events.h"

#include <windows.h>

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

#define mla_platform_strtod __generic_strtod
#define mla_platform_strtoll __generic_strtoll
#define mla_platform_strtoull __generic_strtoull

#endif



void __windows_sleep(mla_uint32_t milliseconds) {

    Sleep(milliseconds);

}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access {
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
        __generic_printf,
        __generic_std_read,
        mla_platform_strtod,
        mla_platform_strtoll,
        mla_platform_strtoull,
        __windows_sleep,
    };

void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif
