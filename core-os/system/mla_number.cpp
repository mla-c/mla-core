//
// Created by chris on 9/22/2025.
//

#include "mla_number.h"

mla_bool_t mla_parse_double(const mla_string_t &str, mla_double_t& out_value) {
    if (str.length == 0 || str.data == nullptr) {
        return false;
    }

    mla_c_string_t c_str = mla_string_to_cString(str);

    mla_bool_t success = mla_strtod(c_str.c_str, str.length, &out_value);

    if (c_str.isOwner) {
        mla_free(const_cast<mla_char_t *>(c_str.c_str));
    }
    return success;
}

mla_bool_t mla_parse_float(const mla_string_t &str, mla_float_t& out_value) {

    mla_double_t double_value;
    if (!mla_parse_double(str, double_value)) {
        return false;
    }

    out_value = static_cast<mla_float_t>(double_value);
    return true;
}

mla_bool_t mla_parse_int64(const mla_string_t &str, mla_int64_t& out_value) {

    if (str.length == 0 || str.data == nullptr) {
        return false;
    }

    mla_c_string_t c_str = mla_string_to_cString(str);

    mla_bool_t success = mla_strtoll(c_str.c_str, str.length, &out_value);

    if (c_str.isOwner) {
        mla_free(const_cast<mla_char_t *>(c_str.c_str));
    }
    return success;

}

mla_bool_t mla_parse_uint64(const mla_string_t &str, mla_uint64_t& out_value) {
    if (str.length == 0 || str.data == nullptr) {
        return false;
    }

    if (str.data[0] == '-') {
        return false; // Negative sign not allowed for unsigned
    }

    mla_c_string_t c_str = mla_string_to_cString(str);

    mla_bool_t success = mla_strtoull(c_str.c_str, str.length, &out_value);

    if (c_str.isOwner) {
        mla_free(const_cast<mla_char_t *>(c_str.c_str));
    }
    return success;
}

mla_bool_t mla_parse_int32(const mla_string_t &str, mla_int32_t& out_value) {

    mla_int64_t value;

    if (!mla_parse_int64(str, value)) {
        return false;
    }

    if (value < mla_int32_min || value > mla_int32_max) {
        return false;
    }

    out_value = static_cast<mla_int32_t>(value);
    return true;
}

mla_bool_t mla_parse_uint32(const mla_string_t &str, mla_uint32_t& out_value) {

    mla_uint64_t value;
    if (!mla_parse_uint64(str, value)) {
        return false;
    }

    if (value > mla_uint32_max) {
        return false;
    }

    out_value = static_cast<mla_uint32_t>(value);
    return true;
}

mla_bool_t mla_parse_int16(const mla_string_t &str, mla_int16_t& out_value) {

    mla_int64_t value;
    if (!mla_parse_int64(str, value)) {
        return false;
    }

    if (value < mla_int16_min || value > mla_int16_max) {
        return false;
    }

    out_value = static_cast<mla_int16_t>(value);
    return true;
}

mla_bool_t mla_parse_uint16(const mla_string_t &str, mla_uint16_t& out_value) {

    mla_uint64_t value;
    if (!mla_parse_uint64(str, value)) {
        return false;
    }

    if (value > mla_uint16_max) {
        return false;
    }

    out_value = static_cast<mla_uint16_t>(value);
    return true;
}

mla_bool_t mla_parse_int8(const mla_string_t &str, mla_int8_t& out_value) {

    mla_int64_t value;
    if (!mla_parse_int64(str, value)) {
        return false;
    }

    if (value < mla_int8_min || value > mla_int8_max) {
        return false;
    }

    out_value = static_cast<mla_int8_t>(value);
    return true;
}

mla_bool_t mla_parse_uint8(const mla_string_t &str, mla_uint8_t& out_value) {

    mla_uint64_t value;
    if (!mla_parse_uint64(str, value)) {
        return false;
    }

    if (value > mla_uint8_max) {
        return false;
    }

    out_value = static_cast<mla_uint8_t>(value);
    return true;
}

mla_bool_t mla_parse_bool(const mla_string_t &str, mla_bool_t& out_value) {

    if (str.length == 0 || str.data == nullptr) {
        return false;
    }

    if (mla_string_equals_const(str, "true") ||
        mla_string_equals_const(str, "TRUE") ||
        mla_string_equals_const(str, "1")) {
        out_value = true;
        return true;
    }

    if (mla_string_equals_const(str, "false") ||
        mla_string_equals_const(str, "FALSE") ||
        mla_string_equals_const(str, "0")) {
        out_value = false;
        return true;
    }

    return false;
}
