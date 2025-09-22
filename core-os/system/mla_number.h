//
// Created by chris on 9/22/2025.
//

#ifndef COREOS_MLA_MLA_NUMBER_H
#define COREOS_MLA_MLA_NUMBER_H
#include "../mla_data_types.h"
#include "mla_string.h"

mla_bool_t mla_parse_double(const mla_string_t& str, mla_double_t& out_value);
mla_bool_t mla_parse_float(const mla_string_t& str, mla_float_t& out_value);
mla_bool_t mla_parse_int64(const mla_string_t& str, mla_int64_t& out_value);
mla_bool_t mla_parse_uint64(const mla_string_t& str, mla_uint64_t& out_value);
mla_bool_t mla_parse_int32(const mla_string_t& str, mla_int32_t& out_value);
mla_bool_t mla_parse_uint32(const mla_string_t& str, mla_uint32_t& out_value);
mla_bool_t mla_parse_int16(const mla_string_t& str, mla_int16_t& out_value);
mla_bool_t mla_parse_uint16(const mla_string_t& str, mla_uint16_t& out_value);
mla_bool_t mla_parse_int8(const mla_string_t& str, mla_int8_t& out_value);
mla_bool_t mla_parse_uint8(const mla_string_t& str, mla_uint8_t& out_value);
mla_bool_t mla_parse_bool(const mla_string_t& str, mla_bool_t& out_value);



#endif
