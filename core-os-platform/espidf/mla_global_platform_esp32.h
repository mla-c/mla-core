//
// Created by chris on 9/11/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_ESP32_H
#define COREOS_MLA_GLOBAL_PLATFORM_ESP32_H

#include "../../core-os/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

void __esp32_sleep(mla_uint32_t milliseconds) {

    vTaskDelay(pdMS_TO_TICKS(milliseconds));

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
        __esp32_sleep,
    };

void mla_boot_os_application() {

    // Disable the watchdog for testing purposes
    esp_task_wdt_deinit();
}

#endif