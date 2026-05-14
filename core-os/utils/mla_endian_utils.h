//
// Created by chris on 11/13/2025.
//

#ifndef MLA_ENDIAN_UTILS_H
#define MLA_ENDIAN_UTILS_H

#include "../mla_data_types.h"

// Byte-swapping functions
mla_uint16_t mla_swap_uint16(mla_uint16_t val);

mla_uint32_t mla_swap_uint32(mla_uint32_t val);

mla_uint64_t mla_swap_uint64(mla_uint64_t val);

// Conversion functions for host to little endian
mla_int16_t mla_host_to_le_int16(mla_int16_t val);

mla_int32_t mla_host_to_le_int32(mla_int32_t val);

mla_int64_t mla_host_to_le_int64(mla_int64_t val);

mla_uint16_t mla_host_to_le_uint16(mla_uint16_t val);

mla_uint32_t mla_host_to_le_uint32(mla_uint32_t val);

mla_uint64_t mla_host_to_le_uint64(mla_uint64_t val);

// For floating-point types
mla_float_t mla_host_to_le_float(mla_float_t val);
mla_double_t mla_host_to_le_double(mla_double_t val);


mla_uint16_t mla_host_to_be_uint16(mla_uint16_t val);
mla_uint64_t mla_host_to_be_uint64(mla_uint64_t val);
mla_uint32_t mla_host_to_be_uint32(mla_uint32_t val);

mla_uint16_t mla_be_to_host_uint16(mla_uint16_t val);
mla_uint64_t mla_be_to_host_uint64(mla_uint64_t val);
mla_uint32_t mla_be_to_host_uint32(mla_uint32_t val);

// Little endian to host (same functions, just with more descriptive names)
#define mla_le_to_host_int16 mla_host_to_le_int16
#define mla_le_to_host_int32 mla_host_to_le_int32
#define mla_le_to_host_int64 mla_host_to_le_int64
#define mla_le_to_host_uint16 mla_host_to_le_uint16
#define mla_le_to_host_uint32 mla_host_to_le_uint32
#define mla_le_to_host_uint64 mla_host_to_le_uint64
#define mla_le_to_host_float mla_host_to_le_float
#define mla_le_to_host_double mla_host_to_le_double

#endif