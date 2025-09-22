//
// Created by chris on 8/10/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_GENERIC_H
#define COREOS_MLA_GLOBAL_PLATFORM_GENERIC_H

// Including necessary headers
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include "../../core-os/mla_data_types.h"

mla_pointer_t __generic_memcpy(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size) {
    return memcpy(dest, src, size);
}

mla_pointer_t __generic_memset(mla_pointer_t dest, mla_byte_t value, mla_size_t size) {
    return memset(dest, value, size);
}

mla_int32_t __generic_memcmp(const mla_pointer_t dest, const mla_pointer_t src, mla_size_t size) {
    return memcmp(dest, src, size);
}

mla_pointer_t __generic_memmove(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size) {
    return memmove(dest, src, size);
}

mla_char_t* __generic_strcpy(mla_char_t* dest, const mla_char_t* src) {
    return strcpy(dest, src);
}

mla_size_t __generic_strlen(const mla_char_t* str) {
    return (mla_size_t)strlen(str);
}

mla_int32_t __generic_snprintf(mla_char_t* dest, mla_size_t size, const mla_char_t* format, ...) {
    va_list args;
    va_start(args, format);
    mla_int32_t result = vsnprintf(dest, size, format, args);
    va_end(args);
    return result;
}

const mla_char_t* __generic_strstr(const mla_char_t* str, const mla_char_t* substr) {
    return strstr(str, substr);
}

mla_pointer_t __generic_malloc(mla_size_t size) {
    return malloc(size);
}

void __generic_free(mla_pointer_t ptr) {
    free(ptr);
}

mla_bool_t __generic_is_gcc_pointer(const mla_pointer_t ptr) {
    (void)ptr;
    return false;
}

mla_int32_t __generic_printf(const mla_char_t* format, ...) {
    va_list args;
    va_start(args, format);
    mla_int32_t result = vprintf(format, args);
    va_end(args);
    return result;
}

mla_size_t __generic_std_read(mla_char_t* buffer, mla_size_t size) {
    mla_char_t* lastChar = fgets(buffer, size, stdin);
    if (lastChar == nullptr) {
        return 0; // No data read
    }
    char *nul = (mla_char_t*)memchr(buffer, '\0', size);
    return nul ? (mla_size_t)(nul - buffer) : size;
}

mla_bool_t __generic_strtod(const mla_char_t* str, mla_size_t length, mla_double_t* out_value) {

    mla_char_t* endptr;
    *out_value = strtod(str, &endptr);

    // Check if conversion was successful
    if (endptr == str) {
        return false; // No conversion performed
    }

    // Check if we consumed all the expected characters
    if ((size_t)(endptr - str) != length) {
        return false; // Didn't parse the entire string
    }

    return true;
}

mla_bool_t __generic_strtoll(const mla_char_t* str, mla_size_t length, mla_int64_t* out_value) {

    mla_char_t* endptr;
    *out_value = strtoll(str, &endptr, 10);

    // Check if conversion was successful
    if (endptr == str) {
        return false; // No conversion performed
    }

    // Check if we consumed all the expected characters
    if ((mla_size_t)(endptr - str) != length) {
        return false; // Didn't parse the entire string
    }

    return true;
}

mla_bool_t __generic_strtoull(const mla_char_t* str, mla_size_t length, mla_uint64_t* out_value) {

    mla_char_t* endptr;
    *out_value = strtoull(str, &endptr, 10);

    // Check if conversion was successful
    if (endptr == str) {
        return false; // No conversion performed
    }

    // Check if we consumed all the expected characters
    if ((mla_size_t)(endptr - str) != length) {
        return false; // Didn't parse the entire string
    }

    return true;
}

#endif
