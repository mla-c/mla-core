//
// Created by chris on 11/13/2025.
//

#include "mla_endian_utils.h"


#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define MLA_LITTLE_ENDIAN 1
    #define MLA_BIG_ENDIAN 0
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define MLA_LITTLE_ENDIAN 0
    #define MLA_BIG_ENDIAN 1
#else
    // Fallback to runtime detection
    static const union { mla_uint16_t i; mla_uint8_t c[2]; } endian_test = {1};
#define MLA_LITTLE_ENDIAN (endian_test.c[0] == 1)
#define MLA_BIG_ENDIAN (!MLA_LITTLE_ENDIAN)
#endif


// Byte-swapping functions
mla_uint16_t mla_swap_uint16(mla_uint16_t val) {
    return (val << 8) | (val >> 8);
}

mla_uint32_t mla_swap_uint32(mla_uint32_t val) {
    return ((val << 24) | ((val << 8) & 0x00FF0000) |
           ((val >> 8) & 0x0000FF00) | (val >> 24));
}

mla_uint64_t mla_swap_uint64(mla_uint64_t val) {
    return ((val << 56) |
           ((val << 40) & 0x00FF000000000000ULL) |
           ((val << 24) & 0x0000FF0000000000ULL) |
           ((val << 8)  & 0x000000FF00000000ULL) |
           ((val >> 8)  & 0x00000000FF000000ULL) |
           ((val >> 24) & 0x0000000000FF0000ULL) |
           ((val >> 40) & 0x000000000000FF00ULL) |
           (val >> 56));
}

// Conversion functions for host to little endian
mla_int16_t mla_host_to_le_int16(mla_int16_t val) {
    return MLA_LITTLE_ENDIAN ? val : mla_swap_uint16(val);
}

mla_int32_t mla_host_to_le_int32(mla_int32_t val) {
    return MLA_LITTLE_ENDIAN ? val : mla_swap_uint32(val);
}

mla_int64_t mla_host_to_le_int64(mla_int64_t val) {
    return MLA_LITTLE_ENDIAN ? val : mla_swap_uint64(val);
}

mla_uint16_t mla_host_to_le_uint16(mla_uint16_t val) {
    return MLA_LITTLE_ENDIAN ? val : mla_swap_uint16(val);
}

mla_uint32_t mla_host_to_le_uint32(mla_uint32_t val) {
    return MLA_LITTLE_ENDIAN ? val : mla_swap_uint32(val);
}

mla_uint64_t mla_host_to_le_uint64(mla_uint64_t val) {
    return MLA_LITTLE_ENDIAN ? val : mla_swap_uint64(val);
}

// For floating-point types
mla_float_t mla_host_to_le_float(mla_float_t val) {
    union { mla_float_t f; mla_uint32_t i; } u;
    u.f = val;
    u.i = mla_host_to_le_uint32(u.i);
    return u.f;
}

mla_double_t mla_host_to_le_double(mla_double_t val) {
    union { mla_double_t d; mla_uint64_t i; } u;
    u.d = val;
    u.i = mla_host_to_le_uint64(u.i);
    return u.d;
}


// Conversion functions for host to big endian (network byte order)
mla_uint16_t mla_host_to_be_uint16(mla_uint16_t val) {
    return MLA_BIG_ENDIAN ? val : mla_swap_uint16(val);
}

mla_uint64_t mla_host_to_be_uint64(mla_uint64_t val) {
    return MLA_BIG_ENDIAN ? val : mla_swap_uint64(val);
}

mla_uint32_t mla_host_to_be_uint32(mla_uint32_t val) {
    return MLA_BIG_ENDIAN ? val : mla_swap_uint32(val);
}

mla_uint16_t mla_be_to_host_uint16(mla_uint16_t val) {
    return MLA_BIG_ENDIAN ? val : mla_swap_uint16(val);
}

mla_uint64_t mla_be_to_host_uint64(mla_uint64_t val) {
    return MLA_BIG_ENDIAN ? val : mla_swap_uint64(val);
}

mla_uint32_t mla_be_to_host_uint32(mla_uint32_t val) {
    return MLA_BIG_ENDIAN ? val : mla_swap_uint32(val);
}
