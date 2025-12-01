//
// This file contains the implementations of functions to convert basic types to mla_string_t
// Created by chris on 9/14/2025.
//

#include "mla_string.h"
#include "../mla_data_types.h"
#include "mla_reference.h"

mla_string_t mla_string_from_int8(mla_int8_t p_Value) {
    const mla_size_t bufferSize = 5;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_bool_t isNegative = p_Value < 0;
    mla_uint8_t value = isNegative ? static_cast<mla_uint8_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint8_t>(p_Value);

    if (isNegative) {
        buffer[index++] = '-';
    }

    mla_size_t startDigits = index;
    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    // Reverse digits
    mla_size_t end = index - 1;
    while (startDigits < end) {
        mla_char_t temp = buffer[startDigits];
        buffer[startDigits] = buffer[end];
        buffer[end] = temp;
        startDigits++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint8(mla_uint8_t p_Value) {
    const mla_size_t bufferSize = 4;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_uint8_t value = p_Value;

    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    // Reverse digits
    mla_size_t start = 0;
    mla_size_t end = index - 1;
    while (start < end) {
        mla_char_t temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_int16(mla_int16_t p_Value) {
    const mla_size_t bufferSize = 7;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_bool_t isNegative = p_Value < 0;
    mla_uint16_t value = isNegative ? static_cast<mla_uint16_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint16_t>(p_Value);

    if (isNegative) {
        buffer[index++] = '-';
    }

    mla_size_t startDigits = index;
    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    mla_size_t end = index - 1;
    while (startDigits < end) {
        mla_char_t temp = buffer[startDigits];
        buffer[startDigits] = buffer[end];
        buffer[end] = temp;
        startDigits++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint16(mla_uint16_t p_Value) {
    const mla_size_t bufferSize = 6;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_uint16_t value = p_Value;

    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    mla_size_t start = 0;
    mla_size_t end = index - 1;
    while (start < end) {
        mla_char_t temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_int32(mla_int32_t p_Value) {
    const mla_size_t bufferSize = 12;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_bool_t isNegative = p_Value < 0;
    mla_uint32_t value = isNegative ? static_cast<mla_uint32_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint32_t>(p_Value);

    if (isNegative) {
        buffer[index++] = '-';
    }

    mla_size_t startDigits = index;
    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    mla_size_t end = index - 1;
    while (startDigits < end) {
        mla_char_t temp = buffer[startDigits];
        buffer[startDigits] = buffer[end];
        buffer[end] = temp;
        startDigits++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint32(mla_uint32_t p_Value) {
    const mla_size_t bufferSize = 11;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_uint32_t value = p_Value;

    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    mla_size_t start = 0;
    mla_size_t end = index - 1;
    while (start < end) {
        mla_char_t temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_int64(mla_int64_t p_Value) {
    const mla_size_t bufferSize = 21;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_bool_t isNegative = p_Value < 0;
    mla_uint64_t value = isNegative ? static_cast<mla_uint64_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint64_t>(p_Value);

    if (isNegative) {
        buffer[index++] = '-';
    }

    mla_size_t startDigits = index;
    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    mla_size_t end = index - 1;
    while (startDigits < end) {
        mla_char_t temp = buffer[startDigits];
        buffer[startDigits] = buffer[end];
        buffer[end] = temp;
        startDigits++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint64(mla_uint64_t p_Value) {
    const mla_size_t bufferSize = 21;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;
    mla_uint64_t value = p_Value;

    do {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    mla_size_t start = 0;
    mla_size_t end = index - 1;
    while (start < end) {
        mla_char_t temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_float(mla_float_t p_Value, mla_size_t p_DecimalPlaces) {
    const mla_size_t bufferSize = 64;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;

    // Handle special cases
    if (p_Value != p_Value) { // NaN
        buffer[0] = 'N'; buffer[1] = 'a'; buffer[2] = 'N'; buffer[3] = '\0';
        return { buffer, 3, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
    }
    if (p_Value == mla_infinity_pos) { // +Infinity
        buffer[0] = 'i'; buffer[1] = 'n'; buffer[2] = 'f'; buffer[3] = '\0';
        return { buffer, 3, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
    }
    if (p_Value == mla_infinity_neg) { // -Infinity
        buffer[0] = '-'; buffer[1] = 'i'; buffer[2] = 'n'; buffer[3] = 'f'; buffer[4] = '\0';
        return { buffer, 4, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
    }

    // Handle negative values
    if (p_Value < 0) {
        buffer[index++] = '-';
        p_Value = -p_Value;
    }

    // Apply rounding
    mla_float_t roundingFactor = 0.5f;
    for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
        roundingFactor /= 10.0f;
    }
    p_Value += roundingFactor;

    // Extract integer part
    mla_uint64_t integerPart = static_cast<mla_uint64_t>(p_Value);
    mla_float_t fractionalPart = p_Value - static_cast<mla_float_t>(integerPart);

    // Convert integer part
    if (integerPart == 0) {
        buffer[index++] = '0';
    } else {
        mla_size_t intStartIndex = index;
        do {
            buffer[index++] = '0' + (integerPart % 10);
            integerPart /= 10;
        } while (integerPart > 0);

        // Reverse integer digits
        mla_size_t intEndIndex = index - 1;
        while (intStartIndex < intEndIndex) {
            mla_char_t temp = buffer[intStartIndex];
            buffer[intStartIndex] = buffer[intEndIndex];
            buffer[intEndIndex] = temp;
            intStartIndex++;
            intEndIndex--;
        }
    }

    // Add decimal point and fractional part if needed
    if (p_DecimalPlaces > 0) {
        buffer[index++] = '.';

        // Convert fractional part
        for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
            fractionalPart *= 10.0f;
            mla_uint8_t digit = static_cast<mla_uint8_t>(fractionalPart);
            buffer[index++] = '0' + digit;
            fractionalPart -= static_cast<mla_float_t>(digit);
        }
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_double(mla_double_t p_Value, mla_size_t p_DecimalPlaces) {
    const mla_size_t bufferSize = 128;
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t index = 0;

    // Handle special cases
    if (p_Value != p_Value) { // NaN
        buffer[0] = 'N'; buffer[1] = 'a'; buffer[2] = 'N'; buffer[3] = '\0';
        return { buffer, 3, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
    }
    if (p_Value == mla_infinity_pos) { // +Infinity
        buffer[0] = 'i'; buffer[1] = 'n'; buffer[2] = 'f'; buffer[3] = '\0';
        return { buffer, 3, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
    }
    if (p_Value == mla_infinity_neg) { // -Infinity
        buffer[0] = '-'; buffer[1] = 'i'; buffer[2] = 'n'; buffer[3] = 'f'; buffer[4] = '\0';
        return { buffer, 4, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
    }

    // Handle negative values
    if (p_Value < 0) {
        buffer[index++] = '-';
        p_Value = -p_Value;
    }

    // Apply rounding
    mla_double_t roundingFactor = 0.5;
    for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
        roundingFactor /= 10.0;
    }
    p_Value += roundingFactor;

    // Extract integer part
    mla_uint64_t integerPart = static_cast<mla_uint64_t>(p_Value);
    mla_double_t fractionalPart = p_Value - static_cast<mla_double_t>(integerPart);

    // Convert integer part
    if (integerPart == 0) {
        buffer[index++] = '0';
    } else {
        mla_size_t intStartIndex = index;
        do {
            buffer[index++] = '0' + (integerPart % 10);
            integerPart /= 10;
        } while (integerPart > 0);

        // Reverse integer digits
        mla_size_t intEndIndex = index - 1;
        while (intStartIndex < intEndIndex) {
            mla_char_t temp = buffer[intStartIndex];
            buffer[intStartIndex] = buffer[intEndIndex];
            buffer[intEndIndex] = temp;
            intStartIndex++;
            intEndIndex--;
        }
    }

    // Add decimal point and fractional part if needed
    if (p_DecimalPlaces > 0) {
        buffer[index++] = '.';

        // Convert fractional part
        for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
            fractionalPart *= 10.0;
            mla_uint8_t digit = static_cast<mla_uint8_t>(fractionalPart);
            buffer[index++] = '0' + digit;
            fractionalPart -= static_cast<mla_double_t>(digit);
        }
    }

    buffer[index] = '\0';
    return { buffer, index, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
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

const mla_char_t hexDigits[] = "0123456789ABCDEF";

mla_string_t mla_string_from_uint8_hex(mla_uint8_t p_Value) {
    const mla_size_t bufferSize = 5; // "0x" + 2 hex digits + null
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    buffer[0] = '0';
    buffer[1] = 'x';


    buffer[2] = hexDigits[(p_Value >> 4) & 0x0F];
    buffer[3] = hexDigits[p_Value & 0x0F];
    buffer[4] = '\0';

    return { buffer, 4, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint16_hex(mla_uint16_t p_Value) {
    const mla_size_t bufferSize = 7; // "0x" + 4 hex digits + null
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    buffer[0] = '0';
    buffer[1] = 'x';

    buffer[2] = hexDigits[(p_Value >> 12) & 0x0F];
    buffer[3] = hexDigits[(p_Value >> 8) & 0x0F];
    buffer[4] = hexDigits[(p_Value >> 4) & 0x0F];
    buffer[5] = hexDigits[p_Value & 0x0F];
    buffer[6] = '\0';

    return { buffer, 6, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint32_hex(mla_uint32_t p_Value) {
    const mla_size_t bufferSize = 11; // "0x" + 8 hex digits + null
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    buffer[0] = '0';
    buffer[1] = 'x';

    buffer[2] = hexDigits[(p_Value >> 28) & 0x0F];
    buffer[3] = hexDigits[(p_Value >> 24) & 0x0F];
    buffer[4] = hexDigits[(p_Value >> 20) & 0x0F];
    buffer[5] = hexDigits[(p_Value >> 16) & 0x0F];
    buffer[6] = hexDigits[(p_Value >> 12) & 0x0F];
    buffer[7] = hexDigits[(p_Value >> 8) & 0x0F];
    buffer[8] = hexDigits[(p_Value >> 4) & 0x0F];
    buffer[9] = hexDigits[p_Value & 0x0F];
    buffer[10] = '\0';

    return { buffer, 10, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}

mla_string_t mla_string_from_uint64_hex(mla_uint64_t p_Value) {
    const mla_size_t bufferSize = 19; // "0x" + 16 hex digits + null
    mla_char_t* buffer = mla_create_char_array(bufferSize);

    if (buffer == nullptr) {
        return mla_string_empty();
    }

    buffer[0] = '0';
    buffer[1] = 'x';

    buffer[2] = hexDigits[(p_Value >> 60) & 0x0F];
    buffer[3] = hexDigits[(p_Value >> 56) & 0x0F];
    buffer[4] = hexDigits[(p_Value >> 52) & 0x0F];
    buffer[5] = hexDigits[(p_Value >> 48) & 0x0F];
    buffer[6] = hexDigits[(p_Value >> 44) & 0x0F];
    buffer[7] = hexDigits[(p_Value >> 40) & 0x0F];
    buffer[8] = hexDigits[(p_Value >> 36) & 0x0F];
    buffer[9] = hexDigits[(p_Value >> 32) & 0x0F];
    buffer[10] = hexDigits[(p_Value >> 28) & 0x0F];
    buffer[11] = hexDigits[(p_Value >> 24) & 0x0F];
    buffer[12] = hexDigits[(p_Value >> 20) & 0x0F];
    buffer[13] = hexDigits[(p_Value >> 16) & 0x0F];
    buffer[14] = hexDigits[(p_Value >> 12) & 0x0F];
    buffer[15] = hexDigits[(p_Value >> 8) & 0x0F];
    buffer[16] = hexDigits[(p_Value >> 4) & 0x0F];
    buffer[17] = hexDigits[p_Value & 0x0F];
    buffer[18] = '\0';

    return { buffer, 18, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(buffer) };
}