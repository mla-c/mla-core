//
// Created by chris on 8/10/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_GENERIC_H
#define MLA_GLOBAL_PLATFORM_GENERIC_H

// Including necessary headers
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include "../../core/mla_data_types.h"

mla_platform_pointer_t mla_internal_generic_memcpy(mla_platform_pointer_t dest, mla_platform_const_pointer_t src, mla_size_t size) {
    return memcpy(dest, src, size);
}

mla_platform_pointer_t mla_internal_generic_memset(mla_platform_pointer_t dest, mla_byte_t value, mla_size_t size) {
    return memset(dest, value, size);
}

mla_int32_t mla_internal_generic_memcmp(mla_platform_const_pointer_t dest, mla_platform_const_pointer_t src, mla_size_t size) {
    return memcmp(dest, src, size);
}

mla_platform_pointer_t mla_internal_generic_memmove(mla_platform_pointer_t dest, mla_platform_const_pointer_t src, mla_size_t size) {
    return memmove(dest, src, size);
}

mla_size_t mla_internal_generic_strlen(const mla_char_t* str) {
    return static_cast<mla_size_t>(strlen(str));
}

const mla_char_t* mla_internal_generic_strstr(const mla_char_t* str, const mla_char_t* substr) {
    return strstr(str, substr);
}

mla_platform_pointer_t mla_internal_generic_malloc(mla_size_t size) {
    return malloc(size);
}

void mla_internal_generic_free(mla_platform_pointer_t ptr) {
    free(ptr);
}

mla_size_t mla_internal_generic_print(const mla_char_t* format, mla_size_t length) {
    return static_cast<mla_size_t>(fwrite(format, 1, length, stdout));
}

void mla_internal_generic_on_malloc_failure(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name) {

    // Use direct writes to stderr without formatting that might allocate
    const char* prefix = "Memory allocation failed: ";
    const char* bytes_str = " bytes in ";
    const char* parenthesis_open = " (";
    const char* parenthesis_close = ")\n";

    // Write each part separately to avoid memory allocation
    fwrite(prefix, 1, strlen(prefix), stdout);

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
            size_buffer[pos--] = static_cast<char>('0' + (temp_size % 10));
            temp_size /= 10;
        }
        size_len = static_cast<int>(sizeof(size_buffer) - pos - 2);
        // Move to beginning of buffer
        memmove(size_buffer, &size_buffer[pos + 1], size_len);
    }

    fwrite(size_buffer, 1, size_len, stdout);
    fwrite(bytes_str, 1, strlen(bytes_str), stdout);
    fwrite(filename, 1, strlen(filename), stdout);
    fwrite(parenthesis_open, 1, strlen(parenthesis_open), stdout);
    fwrite(function_name, 1, strlen(function_name), stdout);
    fwrite(parenthesis_close, 1, strlen(parenthesis_close), stdout);

    // Ensure output is flushed
    fflush(stdout);
}

mla_size_t mla_internal_generic_std_read(mla_char_t* buffer, mla_size_t size) {
    mla_char_t* lastChar = fgets(buffer, static_cast<int>(size), stdin);
    if (lastChar == nullptr) {
        return 0; // No data read
    }
    char *nul = static_cast<mla_char_t *>(memchr(buffer, '\0', size));
    return nul != nullptr ? static_cast<mla_size_t>(nul - buffer) : size;
}

mla_bool_t mla_internal_generic_strtod(const mla_char_t* str, mla_size_t length, mla_double_t* out_value) {

    mla_char_t* endptr;
    *out_value = strtod(str, &endptr);

    // Check if conversion was successful
    if (endptr == str) {
        return false; // No conversion performed
    }

    // Check if we consumed all the expected characters
    if (static_cast<mla_size_t>(endptr - str) != length) {
        return false; // Didn't parse the entire string
    }

    return true;
}

mla_bool_t mla_internal_generic_strtoll(const mla_char_t* str, mla_size_t length, mla_int64_t* out_value) {

    mla_char_t* endptr;
    *out_value = strtoll(str, &endptr, 10);

    // Check if conversion was successful
    if (endptr == str) {
        return false; // No conversion performed
    }

    // Check if we consumed all the expected characters
    if (static_cast<mla_size_t>(endptr - str) != length) {
        return false; // Didn't parse the entire string
    }

    return true;
}

mla_bool_t mla_internal_generic_strtoull(const mla_char_t* str, mla_size_t length, mla_uint64_t* out_value) {

    mla_char_t* endptr;
    *out_value = strtoull(str, &endptr, 10);

    // Check if conversion was successful
    if (endptr == str) {
        return false; // No conversion performed
    }

    // Check if we consumed all the expected characters
    if (static_cast<mla_size_t>(endptr - str) != length) {
        return false; // Didn't parse the entire string
    }

    return true;
}

#endif
