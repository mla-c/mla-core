//
// Created by chris on 8/10/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_LINUX_H
#define COREOS_MLA_GLOBAL_PLATFORM_LINUX_H

#include "../../core-os/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include <unistd.h>


void __linux_sleep(mla_uint32_t milliseconds) {

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
    __generic_snprintf,
    __generic_strstr,
    __generic_malloc,
    __generic_free,
    __generic_printf,
    __linux_sleep,
};


void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.
    // Currently, it does nothing but can be extended in the future.
}

#endif
