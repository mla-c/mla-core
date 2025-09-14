//
// This file contains the implementations of functions to convert basic types to mla_string_t
// Created by chris on 9/14/2025.
//

#include "mla_string.h"
#include "../mla_data_types.h"
#include "mla_reference.h"



mla_string_t mla_string_from_int32(mla_int32_t p_Value) {
    // Allocate buffer for maximum int32 digits (-2147483648 = 11 chars + null terminator)
    const mla_size_t bufferSize = 12;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_int32_t length = mla_snprintf(buffer, bufferSize, "%ld", p_Value);

    if (length > 0) {
        return { buffer, static_cast<mla_size_t>(length), mla_buffer_reference(buffer), MLA_STRING_MEMORY_LAYOUT_C_STRING };
    } else {
        mla_free(buffer);
        return mla_string_empty();
    }
}

mla_string_t mla_string_from_uint32(mla_uint32_t p_Value) {
    // Allocate buffer for maximum uint32 digits (4294967295 = 10 chars + null terminator)
    const mla_size_t bufferSize = 11;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_int32_t length = mla_snprintf(buffer, bufferSize, "%lu", p_Value);

    if (length > 0) {
        return { buffer, static_cast<mla_size_t>(length), mla_buffer_reference(buffer), MLA_STRING_MEMORY_LAYOUT_C_STRING };
    } else {
        mla_free(buffer);
        return mla_string_empty();
    }
}

mla_string_t mla_string_from_int64(mla_int64_t p_Value) {
    // Allocate buffer for maximum int64 digits (-9223372036854775808 = 20 chars + null terminator)
    const mla_size_t bufferSize = 21;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_int32_t length = mla_snprintf(buffer, bufferSize, "%lld", p_Value);

    if (length > 0) {
        return { buffer, static_cast<mla_size_t>(length), mla_buffer_reference(buffer), MLA_STRING_MEMORY_LAYOUT_C_STRING };
    } else {
        mla_free(buffer);
        return mla_string_empty();
    }
}

mla_string_t mla_string_from_uint64(mla_uint64_t p_Value) {
    // Allocate buffer for maximum uint64 digits (18446744073709551615 = 20 chars + null terminator)
    const mla_size_t bufferSize = 21;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_int32_t length = mla_snprintf(buffer, bufferSize, "%llu", p_Value);

    if (length > 0) {
        return { buffer, static_cast<mla_size_t>(length), mla_buffer_reference(buffer), MLA_STRING_MEMORY_LAYOUT_C_STRING };
    } else {
        mla_free(buffer);
        return mla_string_empty();
    }
}

mla_string_t mla_string_from_float(mla_float_t p_Value, mla_size_t p_DecimalPlaces) {
    // Allocate buffer for float representation (considering sign, digits, decimal point, and precision)
    // Maximum: -3.402823e+38 with precision, so 64 chars should be safe
    const mla_size_t bufferSize = 64;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    // Create format string with specified decimal places
    mla_char_t formatBuffer[16];
    mla_snprintf(formatBuffer, sizeof(formatBuffer), "%%.%luf", p_DecimalPlaces);

    mla_int32_t length = mla_snprintf(buffer, bufferSize, formatBuffer, p_Value);

    if (length > 0) {
        return { buffer, static_cast<mla_size_t>(length), mla_buffer_reference(buffer), MLA_STRING_MEMORY_LAYOUT_C_STRING };
    } else {
        mla_free(buffer);
        return mla_string_empty();
    }
}

mla_string_t mla_string_from_double(mla_double_t p_Value, mla_size_t p_DecimalPlaces) {
    // Allocate buffer for double representation (considering sign, digits, decimal point, and precision)
    // Maximum: -1.7976931348623157e+308 with precision, so 128 chars should be safe
    const mla_size_t bufferSize = 128;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    // Create format string with specified decimal places
    mla_char_t formatBuffer[16];
    mla_snprintf(formatBuffer, sizeof(formatBuffer), "%%.%luf", p_DecimalPlaces);

    mla_int32_t length = mla_snprintf(buffer, bufferSize, formatBuffer, p_Value);

    if (length > 0) {
        return { buffer, static_cast<mla_size_t>(length), mla_buffer_reference(buffer), MLA_STRING_MEMORY_LAYOUT_C_STRING };
    } else {
        mla_free(buffer);
        return mla_string_empty();
    }
}

mla_string_t mla_string_from_bool(mla_bool_t p_Value) {
    if (p_Value) {
        // Return "true" (4 chars + null terminator)
        return mla_string_const("true");
    } else {
        // Return "false" (5 chars + null terminator)
        return mla_string_const("false");
    }
}