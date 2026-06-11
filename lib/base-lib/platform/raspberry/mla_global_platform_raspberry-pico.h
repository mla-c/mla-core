//
// Created by chris on 9/12/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_RASPBERRY_PICO_H
#define MLA_GLOBAL_PLATFORM_RASPBERRY_PICO_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"
#include <Arduino.h>

void __pico_sleep(mla_uint32_t milliseconds) {

    delayMicroseconds(milliseconds);

}

mla_int32_t __pico_printf(const mla_char_t* format, ...) {

    mla_char_t l_Result[255];

    va_list args;
    va_start(args, format);
    mla_int32_t result = vsnprintf(l_Result, sizeof(l_Result) -1, format, args);
    va_end(args);
    l_Result[result] = '\0';
    Serial.print(l_Result);
    return result;
}

void __pico_on_malloc_failure(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name) {

    Serial.print("Memory allocation failed: ");
    const char* bytes_str = " bytes in ";
    const char* parenthesis_open = " (";
    const char* parenthesis_close = ")\n";


    // Convert size to string using stack buffer
    char size_buffer[32] = {0}; // Large enough for any practical size_t value
    int size_len = 0;
    mla_size_t temp_size = size;

    // Handle zero case specially
    if (temp_size == 0) {
        size_buffer[0] = '0';
        size_len = 1;
    } else {
        // Convert number to string manually
        int pos = sizeof(size_buffer) - 2; // Leave room for null terminator
        while (temp_size > 0 && pos >= 0) {
            size_buffer[pos--] = '0' + (temp_size % 10);
            temp_size /= 10;
        }
        size_len = sizeof(size_buffer) - pos - 2;
        // Move to beginning of buffer
        memmove(size_buffer, &size_buffer[pos + 1], size_len);
    }

    Serial.print(size_buffer);
    Serial.print(bytes_str);
    Serial.print(filename);
    Serial.print(parenthesis_open);
    Serial.print(function_name);
    Serial.print(parenthesis_close);

    Serial.flush();
}

mla_size_t __prio_std_read(mla_char_t* buffer, mla_size_t size) {

    mla_size_t readedChars = 0;

    while (readedChars < size - 1) { // Leave space for null terminator

        if (Serial.available() > 0) {

            mla_char_t c = Serial.read();
            buffer[readedChars++] = c;
            if (c == '\n') {
                break; // Stop reading on newline or carriage return
            }
        } else {
            break;
        }
    }

    if (readedChars > 0) {
        // Print the received characters for the user to see what was typed
        for (mla_size_t i = 0; i < readedChars; ++i) {
            Serial.print(buffer[i]);
        }
    }

    Serial.flush();

    // convert \r\n from the end to \n or convert single \r to \n
    if (readedChars >= 2 && buffer[readedChars - 2] == '\r' && buffer[readedChars - 1] == '\n') {
        buffer[readedChars - 2] = '\n';
        readedChars -= 1; // Remove the extra character
    } else if (readedChars >= 1 && buffer[readedChars - 1] == '\r') {
        buffer[readedChars - 1] = '\n';
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
        __generic_strlen,
        __generic_snprintf,
        __generic_strstr,
        __generic_malloc,
        __generic_free,
        __pico_on_malloc_failure,
        __pico_printf,
        __prio_std_read,
        __generic_strtod,
        __generic_strtoll,
        __generic_strtoull,
        __pico_sleep,
    };

void mla_boot_os_application() {

    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif