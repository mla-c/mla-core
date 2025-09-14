//
// Created by chris on 9/12/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_ESP8266_H
#define COREOS_MLA_GLOBAL_PLATFORM_ESP8266_H

#include "../../core-os/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/portmacro.h>
#include <freertos/FreeRTOSConfig.h>



void __esp8266_sleep(mla_uint32_t milliseconds) {

    vTaskDelay(milliseconds * configTICK_RATE_HZ / 1000);

}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access {
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
        __generic_std_read,
        __esp8266_sleep,
    };

void mla_boot_os_application() {

    // Disable the watchdog for testing purposes
    //system_soft_wdt_stop();

}

#endif