#ifndef MLA_DATATYPES_H
#define MLA_DATATYPES_H

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


#define mla_pointer_t void*


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


// Min and max macros for various types
#define mla_min(a, b) ((a) < (b) ? (a) : (b))
#define mla_max(a, b) ((a) > (b) ? (a) : (b))

// Default memory operations
#define mla_memcpy(dest, src, size) memcpy((dest), (src), (size))
#define mla_memset(dest, value, size) memset((dest), (value), (size))

// Unsafe C String functions
#define mla_strcpy(dest, src) strcpy((dest), (src))
#define mla_strlen(str) strlen((str))



// Define a type for strings
// This is a simple string structure that holds the length and a pointer to the data
// It is not null-terminated, so it should be used with care
struct mla_string_t {

    mla_string_t() {
        length = 0;
        data = nullptr;
    }

    mla_string_t(mla_char_t* p_Data) {
        length = mla_strlen(p_Data);
        data = p_Data;
    }

    mla_uint32_t length;
    mla_char_t *data;
};

#endif
