//
// Created by christian on 8/12/25.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_WASM_H
#define COREOS_MLA_GLOBAL_PLATFORM_WASM_H

#include "../../core-os/mla_data_types.h"
#include <unistd.h>
#include "../generic/mla_global_platform_generic.h"


void __wasm_sleep(mla_uint32_t milliseconds) {

    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
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
    __generic_printf,
    __generic_std_read,
    __generic_strtod,
    __generic_strtoll,
    __generic_strtoull,
    __wasm_sleep,
};


void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.
    // Currently, it does nothing but can be extended in the future.
}


#endif
