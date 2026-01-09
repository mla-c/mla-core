#ifndef COREOS_MLA_DATATYPES_H
#define COREOS_MLA_DATATYPES_H

/////////////////////////////////////////////////
// Main Datatypes for the MLA framework
////////////////////////////////////////////////

// Platform detection and 32-bit type definitions
#if defined(__LP64__) || defined(_LP64) || defined(__x86_64__) || defined(__aarch64__) || (defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 8)
    // 64-bit Unix/Linux (LP64 model): long is 8 bytes
    #define ___mla_internal_int32 int

#elif defined(_WIN32) || defined(_WIN64)
    // Windows (LLP64 model): long is 4 bytes even on 64-bit
    #define ___mla_internal_int32 long

#else
    // Fallback for other platforms: use int (safest cross-platform choice)
    #define ___mla_internal_int32 int
#endif

#define mla_bool_t bool

#define mla_int8_t signed char
#define mla_uint8_t unsigned char
#define mla_int16_t signed short
#define mla_uint16_t unsigned short
#define mla_int32_t signed ___mla_internal_int32
#define mla_uint32_t unsigned ___mla_internal_int32
#define mla_int64_t signed long long
#define mla_uint64_t unsigned long long

#define mla_float_t float
#define mla_double_t double

#define mla_char_t char
#define mla_utf_16_char_t unsigned short
#define mla_utf_32_char_t unsigned ___mla_internal_int32

#define mla_void_t void
#define mla_pointer_t void*


// Limits for the data types

#define mla_int8_min (-127 - 1)
#define mla_int8_max (127)

#define mla_uint8_min (0)
#define mla_uint8_max (255)

#define mla_int16_min (-32767 - 1)
#define mla_int16_max (32767)

#define mla_uint16_min (0)
#define mla_uint16_max (65535)

#define mla_int32_min (-2147483647 - 1)
#define mla_int32_max (2147483647)

#define mla_uint32_min (0)
#define mla_uint32_max (4294967295U)

#define mla_int64_min (-9223372036854775807L - 1)
#define mla_int64_max (9223372036854775807L)

#define mla_uint64_min (0UL)
#define mla_uint64_max (18446744073709551615ULL)

#define mla_float_min (-3.40282347e+38F)
#define mla_float_max (3.40282347e+38F)


// Shortcut for common types
#define mla_callback_userdata mla_uint64_t

#define mla_size_t mla_uint32_t
#define mla_size_min mla_uint32_min
#define mla_size_max mla_uint32_max

#define mla_hash_t mla_size_t

#define mla_byte_t mla_uint8_t
#define mla_byte_min mla_uint8_min
#define mla_byte_max mla_uint8_max

// Atomic types
struct mla_atomic_int32_t {
    volatile mla_int32_t value;
};

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

// Macro for INFINITY
#define mla_infinity_pos (__builtin_inff())
#define mla_infinity_neg (-(__builtin_inff()))


//////////////////////////////////////////////////////
// Loging and Debugging Macros

// Macro to get the filename only
inline const mla_char_t* ___native_strrchr(const mla_char_t* str, mla_int32_t ch) {
    const mla_char_t* last = nullptr;
    while (*str != '\0') {
        if (*str == ch) {
            last = str;
        }
        str++;
    }
    // Check if the null terminator matches the character
    if (ch == '\0') {
        return str;
    }
    return last;
}

// Extracts the filename from the full path
// The macro works for both Unix and Windows style paths
#define __FILENAME_ONLY__ \
(___native_strrchr(__FILE__, '/') ? ___native_strrchr(__FILE__, '/') + 1 : \
(___native_strrchr(__FILE__, '\\') ? ___native_strrchr(__FILE__, '\\') + 1 : __FILE__))

//////////////////////////////////////////////////
/// Low level memory operations and access to printf and other C functions


typedef struct mla_low_level_operations_t {
    // Function pointers for memory operations
    mla_pointer_t (*memcpy)(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size);
    mla_pointer_t (*memset)(mla_pointer_t dest, mla_byte_t value, mla_size_t size);
    mla_int32_t (*memcmp)(const mla_pointer_t dest, const mla_pointer_t src, mla_size_t size);
    mla_pointer_t (*memmove)(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size);

    // Function pointers for string operations
    mla_char_t* (*strcpy)(mla_char_t* dest, const mla_char_t* src);
    mla_size_t (*strlen)(const mla_char_t* str);
    const mla_char_t* (*strstr)(const mla_char_t* str, const mla_char_t* substr);

    // Memory allocation and deallocation functions
    mla_pointer_t (*malloc)(mla_size_t size);
    void (*free)(mla_pointer_t ptr);
    mla_bool_t (*is_gcc_pointer)(const mla_pointer_t ptr);
    void (*on_malloc_failure)(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name);

    // Function pointers for printf and other output functions
    mla_int32_t (*printf)(const mla_char_t* format, ...);
    mla_size_t (*std_read)(mla_char_t* buffer, mla_size_t size);

    // Parsing
    mla_bool_t (*strtod)(const mla_char_t* str, mla_size_t length, mla_double_t* out_value); // String to double
    mla_bool_t (*strtoll)(const mla_char_t* str, mla_size_t length, mla_int64_t* out_value); // String to int64
    mla_bool_t (*strtoull)(const mla_char_t* str, mla_size_t length, mla_uint64_t* out_value); // String to uint64

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
#define mla_strstr(str, substr) g_low_level_access.strstr((str), (substr))

// Memory allocation and deallocation
inline mla_pointer_t mla_malloc_with_check(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name) {

    mla_pointer_t ptr = g_low_level_access.malloc(size);
    if (ptr == nullptr) {

        const mla_char_t* foundChar = ___native_strrchr(filename, '/');

        if (foundChar == nullptr) {
            foundChar = ___native_strrchr(filename, '\\');
        }

        if (foundChar != nullptr) {
            foundChar = foundChar + 1;
        } else {
            foundChar = filename;
        }

        g_low_level_access.on_malloc_failure(size, foundChar, function_name);
    }
    return ptr;
}

#define mla_malloc(size) mla_malloc_with_check(size, __FILENAME_ONLY__, __func__)
#define mla_free(ptr) g_low_level_access.free((ptr))
#define mla_is_gcc_pointer(ptr) g_low_level_access.is_gcc_pointer((ptr))

// Default printf function
#define mla_printf(format, ...) g_low_level_access.printf((format), __VA_ARGS__)
#define mla_std_read(buffer, size) g_low_level_access.std_read((buffer), (size))

// Parsing functions
#define mla_strtod(str, length, out_value) g_low_level_access.strtod((str), (length), (out_value))
#define mla_strtoll(str, length, out_value) g_low_level_access.strtoll((str), (length), (out_value))
#define mla_strtoull(str, length, out_value) g_low_level_access.strtoull((str), (length), (out_value))

// Sleep function for timing
#define mla_sleep(milliseconds) g_low_level_access.sleep((milliseconds))

// Default initializer for structs and classes which is used to initialize items in data structures like arrays or hash maps.
// Is not really a macro but is part of multiple other macros
template < class T >
struct mla_default_initializer {

    static T init() {
        return T();
    }
};

#define mla_default_init(T) mla_default_initializer<T>

//#define mla_offsetof(type, member) ((mla_size_t)((mla_byte_t*)&(((type*)((mla_byte_t*)1))->member) - (mla_byte_t*)1))
#define mla_offsetof(type, member) ((mla_size_t)__builtin_offsetof(type, member))


#endif
