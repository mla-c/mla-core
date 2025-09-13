#ifndef COREOS_MLA_DATATYPES_H
#define COREOS_MLA_DATATYPES_H

/////////////////////////////////////////////////
// Main Datatypes for the MLA framework
////////////////////////////////////////////////

#define mla_bool_t bool

#define mla_int8_t signed char
#define mla_int8_min (-128)
#define mla_int8_max (127)

#define mla_uint8_t unsigned char
#define mla_uint8_min (0)
#define mla_uint8_max (255)

#define mla_int16_t signed int
#define mla_int16_min (-32768)
#define mla_int16_max (32767)

#define mla_uint16_t unsigned int
#define mla_uint16_min (0)
#define mla_uint16_max (65535)

#define mla_int32_t signed long
#define mla_int32_min (-2147483648)
#define mla_int32_max (2147483647)

#define mla_uint32_t unsigned long
#define mla_uint32_min (0)
#define mla_uint32_max (4294967295U)

#define mla_int64_t signed long long
#define mla_int64_min (-9223372036854775808L)
#define mla_int64_max (9223372036854775807L)

#define mla_uint64_t unsigned long long
#define mla_uint64_min (0UL)
#define mla_uint64_max (18446744073709551615ULL)

#define mla_float_t float
#define mla_double_t double
#define mla_char_t char

#define mla_hash_t mla_size_t

#define mla_void_t void
#define mla_pointer_t void*

#define mla_callback_userdata mla_uint64_t

// Shortcut for common types
#define mla_size_t mla_uint32_t
#define mla_size_min mla_uint32_min
#define mla_size_max mla_uint32_max

#define mla_int_t mla_int32_t
#define mla_int_min mla_int32_min
#define mla_int_max mla_int32_max

#define mla_uint_t mla_uint32_t
#define mla_uint_min mla_uint32_min
#define mla_uint_max mla_uint32_max

#define mla_byte_t mla_uint8_t
#define mla_byte_min mla_uint8_min
#define mla_byte_max mla_uint8_max

// Methode Visibility
#define mla_global extern

// Template Style macros
#include "stdarg.h"

#define mla_va_list va_list
#define mla_va_start(v, l) va_start(v, l)
#define mla_va_end(v) va_end(v)

/////////////////////////////////////////////////
// Commonly used macros for memory operations and type limits

// Min and max macros for various types
#define mla_min(a, b) ((a) < (b) ? (a) : (b))
#define mla_max(a, b) ((a) > (b) ? (a) : (b))

typedef struct mla_low_level_operations_t {
    // Function pointers for memory operations
    mla_pointer_t (*memcpy)(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size);
    mla_pointer_t (*memset)(mla_pointer_t dest, mla_byte_t value, mla_size_t size);
    mla_int32_t (*memcmp)(const mla_pointer_t dest, const mla_pointer_t src, mla_size_t size);
    mla_pointer_t (*memmove)(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size);

    // Function pointers for string operations
    mla_char_t* (*strcpy)(mla_char_t* dest, const mla_char_t* src);
    mla_size_t (*strlen)(const mla_char_t* str);
    mla_int32_t (*snprintf)(mla_char_t* dest, mla_size_t size, const mla_char_t* format, ...);
    const mla_char_t* (*strstr)(const mla_char_t* str, const mla_char_t* substr);

    // Memory allocation and deallocation functions
    mla_pointer_t (*malloc)(mla_size_t size);
    void (*free)(mla_pointer_t ptr);

    // Function pointers for printf and other output functions
    mla_int32_t (*printf)(const mla_char_t* format, ...);

    // Timing
    void (*sleep)(mla_uint32_t milliseconds);

} mla_low_level_operations_t;

// Global variable to hold the low-level memory operations
// This will be initialized in the OS application startup code
mla_global mla_low_level_operations_t g_low_level_access;

// Default memory operations
#define mla_memcpy(dest, src, size) g_low_level_access.memcpy((dest), (src), (size))
#define mla_memset(dest, value, size) g_low_level_access.memset((dest), (value), (size))
#define mla_memcmp(dest, src, size) g_low_level_access.memcmp((dest), (src), (size))
#define mla_memmove(dest, src, size) g_low_level_access.memmove((dest), (src), (size))

// Unsafe C String functions
#define mla_strcpy(dest, src) g_low_level_access.strcpy((dest), (src))
#define mla_strlen(str) g_low_level_access.strlen((str))
#define mla_snprintf(dest, size, format, ...) g_low_level_access.snprintf((dest), (size), (format), __VA_ARGS__)
#define mla_strstr(str, substr) g_low_level_access.strstr((str), (substr))

// Memory allocation and deallocation
#define mla_malloc(size) g_low_level_access.malloc((size))
#define mla_free(ptr) g_low_level_access.free((ptr))

// Default printf function
#define mla_printf(format, ...) g_low_level_access.printf((format), __VA_ARGS__)

// Sleep function for timing
#define mla_sleep(milliseconds) g_low_level_access.sleep((milliseconds))


//////////////////////////////////////////////////////
// Loging and Debugging Macros

// Macro to get the filename only
#include <cstring>

#define __FILENAME_ONLY__ (strrchr("/" __FILE__, '/') + 1)

// Default initializer for structs and classes which is used to initialize items in data structures like arrays or hash maps.
// Is not really a macro but is part of multiple other macros
template < class T >
struct mla_default_initializer {

    static T init() {
        return T();
    }
};

#define mla_default_init(T) mla_default_initializer<T>

// String Helper
#define mla_format(bufferFieldName, bufferSize, format, ...) \
    mla_char_t bufferFieldName[(bufferSize)]; \
    g_low_level_access.snprintf(bufferFieldName, (bufferSize), (format), __VA_ARGS__)

#endif
