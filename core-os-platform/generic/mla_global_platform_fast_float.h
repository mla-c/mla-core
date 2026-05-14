//
// Created by chris on 9/23/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_FAST_FLOAT_H
#define MLA_GLOBAL_PLATFORM_FAST_FLOAT_H

#include "../../core-os/mla_data_types.h"
#include "fast_float.h"

mla_bool_t __fast_float_strtod(const mla_char_t* str, mla_size_t length, mla_double_t* out_value) {
    auto result = fast_float::from_chars(str, str + length, *out_value);
    return result.ec == std::errc() && result.ptr == str + length;
}

mla_bool_t __fast_float_strtoll(const mla_char_t* str, mla_size_t length, mla_int64_t* out_value) {

    // using fast-float to parse integers
    auto result = fast_float::from_chars(str, str + length, *out_value);
    return result.ec == std::errc() && result.ptr == str + length;
}

mla_bool_t __fast_float_strtoull(const mla_char_t* str, mla_size_t length, mla_uint64_t* out_value) {

    // using fast-float to parse integers
    auto result = fast_float::from_chars(str, str + length, *out_value);
    return result.ec == std::errc() && result.ptr == str + length;

}


#endif