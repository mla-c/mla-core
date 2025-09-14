//
// Created by chris on 9/12/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_RASPBERRY_PICO_H
#define COREOS_MLA_GLOBAL_PLATFORM_RASPBERRY_PICO_H

#include "../../core-os/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include <Arduino.h>

void __pico_sleep(mla_uint32_t milliseconds) {

    delayMicroseconds(milliseconds);

}

mla_int32_t __pico_printf(const mla_char_t* format, ...) {

    mla_char_t* l_Result = new mla_char_t[2048];

    va_list args;
    va_start(args, format);
    mla_int32_t result = snprintf(l_Result, 2048, format, args);
    va_end(args);
    Serial.print(l_Result);
    return result;
}

mla_size_t __prio_std_read(mla_char_t* buffer, mla_size_t size) {

    mla_size_t readedChars = 0;

    while (readedChars < size - 1) { // Leave space for null terminator

        if (Serial.available() > 0) {

            mla_char_t c = Serial.read();
            buffer[readedChars++] = c;
            if (c == '\n' || c == '\r') {
                break; // Stop reading on newline or carriage return
            }
        } else {
            break;
        }
    }
    buffer[readedChars] = '\0'; // Null-terminate the string
    return readedChars;
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
        __pico_printf,
        __prio_std_read,
        __pico_sleep,
    };

void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.
    // Currently, it does nothing but can be extended in the future.
}

#endif