//
// Created by chris on 8/10/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_GENERIC_H
#define COREOS_MLA_GLOBAL_PLATFORM_GENERIC_H

// Including necessary headers
#include "cstring"
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

mla_int32_t __generic_printf(const mla_char_t* format, ...) {
    va_list args;
    va_start(args, format);
    mla_int32_t result = vprintf(format, args);
    va_end(args);
    return result;
}


#endif
