//
// Created by chris on 9/22/2025.
//

#include "mla_number.h"



mla_bool_t mla_parse_double(const mla_string_t &str, mla_double_t& out_value) {

    mla_size_t length = mla_string_length(str);

    if (length == 0) {
        return false;
    }

    if (length < (mla_global_config_number_parse_max_stack_buffer_size -1)) {

        const mla_char_t* c_str = mla_string_data(str);

        if (c_str == nullptr) {
            return false;
        }

        mla_char_t buffer[mla_global_config_number_parse_max_stack_buffer_size];
        mla_memcpy(buffer, c_str, length);
        buffer[length] = '\0'; // Null-terminate the buffer

        return mla_strtod(buffer, length, &out_value);

    } else {

        // For longer strings, we can convert to a C-style string to ensure null-termination
        mla_c_string_t c_string = mla_string_to_cString(str);
        const mla_char_t* c_str = mla_c_string_data(c_string);
        if (c_str == nullptr) {
            return false;
        }

        return mla_strtod(c_str, length, &out_value);
    }
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

    mla_size_t length = mla_string_length(str);

    if (length == 0) {
        return false;
    }

    if (length < (mla_global_config_number_parse_max_stack_buffer_size - 1)) {

        const mla_char_t* c_str = mla_string_data(str);

        if (c_str == nullptr) {
            return false;
        }

        mla_char_t buffer[mla_global_config_number_parse_max_stack_buffer_size];
        mla_memcpy(buffer, c_str, length);
        buffer[length] = '\0';

        return mla_strtoll(buffer, length, &out_value);

    } else {

        mla_c_string_t c_str = mla_string_to_cString(str);
        const mla_char_t* c_str_data = mla_c_string_data(c_str);

        if (c_str_data == nullptr) {
            return false;
        }

        return mla_strtoll(c_str_data, length, &out_value);
    }

}

mla_bool_t mla_parse_uint64(const mla_string_t &str, mla_uint64_t& out_value) {

    mla_size_t length = mla_string_length(str);

    if (length == 0) {
        return false;
    }

    if (length < (mla_global_config_number_parse_max_stack_buffer_size - 1)) {

        const mla_char_t* c_str = mla_string_data(str);

        if (c_str == nullptr) {
            return false;
        }

        if (c_str[0] == '-') {
            return false; // Negative sign not allowed for unsigned
        }

        mla_char_t buffer[mla_global_config_number_parse_max_stack_buffer_size];
        mla_memcpy(buffer, c_str, length);
        buffer[length] = '\0';

        return mla_strtoull(buffer, length, &out_value);

    } else {

        mla_c_string_t c_str = mla_string_to_cString(str);
        const mla_char_t* c_str_data = mla_c_string_data(c_str);

        if (c_str_data == nullptr) {
            return false;
        }

        if (c_str_data[0] == '-') {
            return false; // Negative sign not allowed for unsigned
        }

        return mla_strtoull(c_str_data, length, &out_value);
    }
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

    mla_size_t length = mla_string_length(str);

    if (length == 0) {
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

mla_bool_t mla_parse_uint8_hex(const mla_string_t& str, mla_uint8_t& out_value) {
    mla_uint64_t value;
    if (!mla_parse_uint64_hex(str, value)) {
        return false;
    }

    if (value > mla_uint8_max) {
        return false;
    }

    out_value = static_cast<mla_uint8_t>(value);
    return true;
}

mla_bool_t mla_parse_uint16_hex(const mla_string_t& str, mla_uint16_t& out_value) {
    mla_uint64_t value;
    if (!mla_parse_uint64_hex(str, value)) {
        return false;
    }

    if (value > mla_uint16_max) {
        return false;
    }

    out_value = static_cast<mla_uint16_t>(value);
    return true;
}

mla_bool_t mla_parse_uint32_hex(const mla_string_t& str, mla_uint32_t& out_value) {
    mla_uint64_t value;
    if (!mla_parse_uint64_hex(str, value)) {
        return false;
    }

    if (value > mla_uint32_max) {
        return false;
    }

    out_value = static_cast<mla_uint32_t>(value);
    return true;
}

mla_bool_t mla_parse_uint64_hex(const mla_string_t& str, mla_uint64_t& out_value) {

    mla_size_t length = mla_string_length(str);

    if (length == 0) {
        return false;
    }

    const mla_char_t* data = mla_string_data(str);

    mla_size_t start = 0;

    // Skip optional "0x" or "0X" prefix
    if (length >= 2 && data[0] == '0' && (data[1] == 'x' || data[1] == 'X')) {
        start = 2;
    }

    if (start >= length) {
        return false; // No digits after prefix
    }

    mla_uint64_t result = 0;

    for (mla_size_t i = start; i < length; ++i) {
        mla_char_t c = data[i];
        mla_uint8_t digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        } else {
            return false; // Invalid hex character
        }

        // Check for overflow before shifting
        if (result > (mla_uint64_max >> 4)) {
            return false;
        }

        result = (result << 4) | digit;
    }

    out_value = result;
    return true;
}

