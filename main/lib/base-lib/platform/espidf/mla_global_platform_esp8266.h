//
// Created by chris on 9/12/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_ESP8266_H
#define MLA_GLOBAL_PLATFORM_ESP8266_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/portmacro.h>
#include <freertos/FreeRTOSConfig.h>



void mla_private_esp8266_sleep(mla_uint32_t milliseconds) {

    vTaskDelay(milliseconds * configTICK_RATE_HZ / 1000);

}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access {
    mla_private_generic_memcpy,
        mla_private_generic_memset,
        mla_private_generic_memcmp,
        mla_private_generic_memmove,
        mla_private_generic_strlen,
        __generic_snprintf,
        mla_private_generic_strstr,
        mla_private_generic_malloc,
        mla_private_generic_free,
        mla_private_generic_printf,
        mla_private_generic_std_read,
        mla_private_generic_strtod,
        mla_private_generic_strtoll,
        mla_private_generic_strtoull,
        mla_private_esp8266_sleep,
    };

void mla_boot_os_application() {

    // Disable the watchdog for testing purposes
    //system_soft_wdt_stop();

    // Finish boot
    mla_lifecycle_fire_boot_events();

}

#endif