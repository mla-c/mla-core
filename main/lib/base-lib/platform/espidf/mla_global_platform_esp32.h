//
// Created by chris on 9/11/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_ESP32_H
#define MLA_GLOBAL_PLATFORM_ESP32_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include "../../core/lifecycle/mla_lifecycle_events.h"

#if mla_use_fast_float == 1

// Using fast-float library for optimized float parsing
// https://github.com/fastfloat/fast_float
// Is found about 40% faster than standard library
// the code size is growing by about 15KB

#include "../generic/mla_global_platform_fast_float.h"

#define mla_platform_strtod mla_private_fast_float_strtod
#define mla_platform_strtoll mla_private_fast_float_strtoll
#define mla_platform_strtoull mla_private_fast_float_strtoull

#else

#define mla_platform_strtod mla_private_generic_strtod
#define mla_platform_strtoll mla_private_generic_strtoll
#define mla_platform_strtoull mla_private_generic_strtoull

#endif

void mla_private_esp32_sleep(mla_uint32_t milliseconds) {

    vTaskDelay(pdMS_TO_TICKS(milliseconds));

}

mla_uint64_t mla_private_esp32_system_time_ms() {

    return pdTICKS_TO_MS(xTaskGetTickCount());

}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access {
    mla_private_generic_memcpy,
        mla_private_generic_memset,
        mla_private_generic_memcmp,
        mla_private_generic_memmove,
        mla_private_generic_strlen,
        mla_private_generic_strstr,
        mla_private_generic_malloc,
        mla_private_generic_free,
        mla_private_generic_on_malloc_failure,
        mla_private_generic_print,
        mla_private_generic_std_read,
        mla_platform_strtod,
        mla_platform_strtoll,
        mla_platform_strtoull,
        mla_private_esp32_sleep,
        mla_private_esp32_system_time_ms,
        nullptr
    };

void mla_boot_os_application() {

    // Disable the watchdog for testing purposes
    esp_task_wdt_deinit();

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif