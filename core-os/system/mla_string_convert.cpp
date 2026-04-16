//
// This file contains the implementations of functions to convert basic types to mla_string_t
// Created by chris on 9/14/2025.
//

#include "mla_string.h"
#include "../mla_data_types.h"

mla_string_t mla_string_from_int8(mla_int8_t p_Value) {

    const mla_size_t maxLength = 4; // Max: "-128"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result =  {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_bool_t isNegative = p_Value < 0;
        mla_uint8_t value = isNegative ? static_cast<mla_uint8_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint8_t>(p_Value);

        if (isNegative) {
            result.embedded.data[index++] = '-';
        }

        mla_size_t startDigits = index;
        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t end = index - 1;
        while (startDigits < end) {
            mla_char_t temp = result.embedded.data[startDigits];
            result.embedded.data[startDigits] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            startDigits++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        // Fall back to heap allocation for longer strings
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result =  {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint8(mla_uint8_t p_Value) {

    const mla_size_t maxLength = 3; // Max: "255"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_uint8_t value = p_Value;

        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t start = 0;
        mla_size_t end = index - 1;
        while (start < end) {
            mla_char_t temp = result.embedded.data[start];
            result.embedded.data[start] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            start++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_int16(mla_int16_t p_Value) {

    const mla_size_t maxLength = 6; // Max: "-32768"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_bool_t isNegative = p_Value < 0;
        mla_uint16_t value = isNegative ? static_cast<mla_uint16_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint16_t>(p_Value);

        if (isNegative) {
            result.embedded.data[index++] = '-';
        }

        mla_size_t startDigits = index;
        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t end = index - 1;
        while (startDigits < end) {
            mla_char_t temp = result.embedded.data[startDigits];
            result.embedded.data[startDigits] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            startDigits++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint16(mla_uint16_t p_Value) {

    const mla_size_t maxLength = 5; // Max: "65535"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_uint16_t value = p_Value;

        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t start = 0;
        mla_size_t end = index - 1;
        while (start < end) {
            mla_char_t temp = result.embedded.data[start];
            result.embedded.data[start] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            start++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_int32(mla_int32_t p_Value) {

    const mla_size_t maxLength = 11; // Max: "-2147483648"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_bool_t isNegative = p_Value < 0;
        mla_uint32_t value = isNegative ? static_cast<mla_uint32_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint32_t>(p_Value);

        if (isNegative) {
            result.embedded.data[index++] = '-';
        }

        mla_size_t startDigits = index;
        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t end = index - 1;
        while (startDigits < end) {
            mla_char_t temp = result.embedded.data[startDigits];
            result.embedded.data[startDigits] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            startDigits++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint32(mla_uint32_t p_Value) {

    const mla_size_t maxLength = 10; // Max: "4294967295"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_uint32_t value = p_Value;

        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t start = 0;
        mla_size_t end = index - 1;
        while (start < end) {
            mla_char_t temp = result.embedded.data[start];
            result.embedded.data[start] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            start++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_int64(mla_int64_t p_Value) {

    const mla_size_t maxLength = 20; // Max: "-9223372036854775808"

    if (maxLength <= mla_global_config_string_sso_max_length) {

        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_bool_t isNegative = p_Value < 0;
        mla_uint64_t value = isNegative ? static_cast<mla_uint64_t>(-(p_Value + 1)) + 1 : static_cast<mla_uint64_t>(p_Value);

        if (isNegative) {
            result.embedded.data[index++] = '-';
        }

        mla_size_t startDigits = index;
        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t end = index - 1;
        while (startDigits < end) {
            mla_char_t temp = result.embedded.data[startDigits];
            result.embedded.data[startDigits] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            startDigits++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint64(mla_uint64_t p_Value) {
    const mla_size_t maxLength = 20; // Max: "18446744073709551615"

    if (maxLength <= mla_global_config_string_sso_max_length) {

        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t index = 0;
        mla_uint64_t value = p_Value;

        do {
            result.embedded.data[index++] = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse digits
        mla_size_t start = 0;
        mla_size_t end = index - 1;
        while (start < end) {
            mla_char_t temp = result.embedded.data[start];
            result.embedded.data[start] = result.embedded.data[end];
            result.embedded.data[end] = temp;
            start++;
            end--;
        }

        result.embedded.length = static_cast<mla_uint8_t>(index);
        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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
        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_size(mla_size_t p_Value) {
    return mla_string_from_uint32(p_Value);
}

mla_string_t mla_string_from_float(mla_float_t p_Value, mla_size_t p_DecimalPlaces) {

    // Handle special cases
    if (p_Value != p_Value) { // NaN
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.data[0] = 'N';
        result.embedded.data[1] = 'a';
        result.embedded.data[2] = 'N';
        result.embedded.length = 3;
        return result;
    }
    if (p_Value == mla_infinity_pos) { // +Infinity
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.data[0] = 'i';
        result.embedded.data[1] = 'n';
        result.embedded.data[2] = 'f';
        result.embedded.length = 3;
        return result;
    }
    if (p_Value == mla_infinity_neg) { // -Infinity
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.data[0] = '-';
        result.embedded.data[1] = 'i';
        result.embedded.data[2] = 'n';
        result.embedded.data[3] = 'f';
        result.embedded.length = 4;
        return result;
    }

    // 1. Determine size requirements
    mla_bool_t isNegative = false;
    if (p_Value < 0) {
        isNegative = true;
        p_Value = -p_Value;
    }

    // Apply rounding
    mla_float_t roundingFactor = 0.5f;
    for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
        roundingFactor /= 10.0f;
    }
    p_Value += roundingFactor;

    // Split parts
    mla_uint64_t integerPart = static_cast<mla_uint64_t>(p_Value);
    mla_float_t fractionalPart = p_Value - static_cast<mla_float_t>(integerPart);

    // Count integer digits
    mla_size_t intDigits = 0;
    mla_uint64_t tempInt = integerPart;
    if (tempInt == 0) {
        intDigits = 1;
    } else {
        while (tempInt > 0) {
            tempInt /= 10;
            intDigits++;
        }
    }

    // Calculate total length
    mla_size_t totalLength = intDigits;
    if (isNegative) totalLength++;
    if (p_DecimalPlaces > 0) totalLength += (1 + p_DecimalPlaces); // +1 for dot

    // 2. Prepare destination
    mla_string_t result = mla_string_empty();
    mla_char_t* dest;

    if (totalLength <= mla_global_config_string_sso_max_length) {
        // Initialize for SSO
        result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = static_cast<mla_uint8_t>(totalLength);
        dest = result.embedded.data;
    } else {
        // Initialize for Heap
        const mla_size_t bufferSize = totalLength + 1;
        dest = mla_create_char_array(bufferSize);
        if (dest == nullptr) {
            return mla_string_empty();
        }
        dest[totalLength] = '\0'; // Null terminator

        result = {mla_buffer_reference(dest), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = dest;
        result.heap.length = totalLength;
    }

    // 3. Write Data
    mla_size_t index = 0;

    // Sign
    if (isNegative) {
        dest[index++] = '-';
    }

    // Integer Part (Write backwards from end of int segment)
    mla_size_t intEndIndex = index + intDigits;
    mla_size_t writerPos = intEndIndex - 1;

    if (integerPart == 0) {
        dest[writerPos] = '0';
    } else {
        while (integerPart > 0) {
            dest[writerPos--] = '0' + (integerPart % 10);
            integerPart /= 10;
        }
    }
    index = intEndIndex;

    // Decimal Point and Fraction
    if (p_DecimalPlaces > 0) {
        dest[index++] = '.';

        for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
            fractionalPart *= 10.0f;
            mla_uint8_t digit = static_cast<mla_uint8_t>(fractionalPart);
            dest[index++] = '0' + digit;
            fractionalPart -= static_cast<mla_float_t>(digit);
        }
    }

    return result;
}

mla_string_t mla_string_from_double(mla_double_t p_Value, mla_size_t p_DecimalPlaces) {

    // Handle special cases
    if (p_Value != p_Value) { // NaN
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.data[0] = 'N';
        result.embedded.data[1] = 'a';
        result.embedded.data[2] = 'N';
        result.embedded.length = 3;
        return result;
    }
    if (p_Value == mla_infinity_pos) { // +Infinity
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.data[0] = 'i';
        result.embedded.data[1] = 'n';
        result.embedded.data[2] = 'f';
        result.embedded.length = 3;
        return result;
    }
    if (p_Value == mla_infinity_neg) { // -Infinity
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.data[0] = '-';
        result.embedded.data[1] = 'i';
        result.embedded.data[2] = 'n';
        result.embedded.data[3] = 'f';
        result.embedded.length = 4;
        return result;
    }

    // 1. Determine size requirements
    mla_bool_t isNegative = false;
    if (p_Value < 0) {
        isNegative = true;
        p_Value = -p_Value;
    }

    // Apply rounding
    mla_double_t roundingFactor = 0.5;
    for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
        roundingFactor /= 10.0;
    }
    p_Value += roundingFactor;

    // Split parts
    mla_uint64_t integerPart = static_cast<mla_uint64_t>(p_Value);
    mla_double_t fractionalPart = p_Value - static_cast<mla_double_t>(integerPart);

    // Count integer digits
    mla_size_t intDigits = 0;
    mla_uint64_t tempInt = integerPart;
    if (tempInt == 0) {
        intDigits = 1;
    } else {
        while (tempInt > 0) {
            tempInt /= 10;
            intDigits++;
        }
    }

    // Calculate total length
    mla_size_t totalLength = intDigits;
    if (isNegative) totalLength++;
    if (p_DecimalPlaces > 0) totalLength += (1 + p_DecimalPlaces); // +1 for dot

    // 2. Prepare destination
    mla_string_t result = mla_string_empty();
    mla_char_t* dest;

    if (totalLength <= mla_global_config_string_sso_max_length) {
        // Initialize for SSO
        result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = static_cast<mla_uint8_t>(totalLength);
        dest = result.embedded.data;
    } else {
        // Initialize for Heap
        const mla_size_t bufferSize = totalLength + 1;
        dest = mla_create_char_array(bufferSize);
        if (dest == nullptr) {
            return mla_string_empty();
        }
        dest[totalLength] = '\0'; // Null terminator

        result = {mla_buffer_reference(dest), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = dest;
        result.heap.length = totalLength;
    }

    // 3. Write Data
    mla_size_t index = 0;

    // Sign
    if (isNegative) {
        dest[index++] = '-';
    }

    // Integer Part (Write backwards from end of int segment)
    mla_size_t intEndIndex = index + intDigits;
    mla_size_t writerPos = intEndIndex - 1;

    if (integerPart == 0) {
        dest[writerPos] = '0';
    } else {
        while (integerPart > 0) {
            dest[writerPos--] = '0' + (integerPart % 10);
            integerPart /= 10;
        }
    }
    index = intEndIndex;

    // Decimal Point and Fraction
    if (p_DecimalPlaces > 0) {
        dest[index++] = '.';

        for (mla_size_t i = 0; i < p_DecimalPlaces; i++) {
            fractionalPart *= 10.0;
            mla_uint8_t digit = static_cast<mla_uint8_t>(fractionalPart);
            dest[index++] = '0' + digit;
            fractionalPart -= static_cast<mla_double_t>(digit);
        }
    }

    return result;
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
    const mla_size_t maxLength = 4; // "0x" + 2 hex digits

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        result.embedded.data[0] = '0';
        result.embedded.data[1] = 'x';
        result.embedded.data[2] = hexDigits[(p_Value >> 4) & 0x0F];
        result.embedded.data[3] = hexDigits[p_Value & 0x0F];
        result.embedded.length = 4;

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
        mla_char_t* buffer = mla_create_char_array(bufferSize);

        if (buffer == nullptr) {
            return mla_string_empty();
        }

        buffer[0] = '0';
        buffer[1] = 'x';
        buffer[2] = hexDigits[(p_Value >> 4) & 0x0F];
        buffer[3] = hexDigits[p_Value & 0x0F];
        buffer[4] = '\0';

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = 4;
        return result;
    }
}

mla_string_t mla_string_from_uint16_hex(mla_uint16_t p_Value) {
    const mla_size_t maxLength = 6; // "0x" + 4 hex digits

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        result.embedded.data[0] = '0';
        result.embedded.data[1] = 'x';
        result.embedded.data[2] = hexDigits[(p_Value >> 12) & 0x0F];
        result.embedded.data[3] = hexDigits[(p_Value >> 8) & 0x0F];
        result.embedded.data[4] = hexDigits[(p_Value >> 4) & 0x0F];
        result.embedded.data[5] = hexDigits[p_Value & 0x0F];
        result.embedded.length = 6;

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = 6;
        return result;
    }
}

mla_string_t mla_string_from_uint32_hex(mla_uint32_t p_Value) {
    const mla_size_t maxLength = 10; // "0x" + 8 hex digits

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        result.embedded.data[0] = '0';
        result.embedded.data[1] = 'x';
        result.embedded.data[2] = hexDigits[(p_Value >> 28) & 0x0F];
        result.embedded.data[3] = hexDigits[(p_Value >> 24) & 0x0F];
        result.embedded.data[4] = hexDigits[(p_Value >> 20) & 0x0F];
        result.embedded.data[5] = hexDigits[(p_Value >> 16) & 0x0F];
        result.embedded.data[6] = hexDigits[(p_Value >> 12) & 0x0F];
        result.embedded.data[7] = hexDigits[(p_Value >> 8) & 0x0F];
        result.embedded.data[8] = hexDigits[(p_Value >> 4) & 0x0F];
        result.embedded.data[9] = hexDigits[p_Value & 0x0F];
        result.embedded.length = 10;

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = 10;
        return result;
    }
}

mla_string_t mla_string_from_uint64_hex(mla_uint64_t p_Value) {
    const mla_size_t maxLength = 18; // "0x" + 16 hex digits

    if (maxLength <= mla_global_config_string_sso_max_length) {
        // This branch won't be taken with default SSO size, but included for consistency
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        result.embedded.data[0] = '0';
        result.embedded.data[1] = 'x';
        result.embedded.data[2] = hexDigits[(p_Value >> 60) & 0x0F];
        result.embedded.data[3] = hexDigits[(p_Value >> 56) & 0x0F];
        result.embedded.data[4] = hexDigits[(p_Value >> 52) & 0x0F];
        result.embedded.data[5] = hexDigits[(p_Value >> 48) & 0x0F];
        result.embedded.data[6] = hexDigits[(p_Value >> 44) & 0x0F];
        result.embedded.data[7] = hexDigits[(p_Value >> 40) & 0x0F];
        result.embedded.data[8] = hexDigits[(p_Value >> 36) & 0x0F];
        result.embedded.data[9] = hexDigits[(p_Value >> 32) & 0x0F];
        result.embedded.data[10] = hexDigits[(p_Value >> 28) & 0x0F];
        result.embedded.data[11] = hexDigits[(p_Value >> 24) & 0x0F];
        result.embedded.data[12] = hexDigits[(p_Value >> 20) & 0x0F];
        result.embedded.data[13] = hexDigits[(p_Value >> 16) & 0x0F];
        result.embedded.data[14] = hexDigits[(p_Value >> 12) & 0x0F];
        result.embedded.data[15] = hexDigits[(p_Value >> 8) & 0x0F];
        result.embedded.data[16] = hexDigits[(p_Value >> 4) & 0x0F];
        result.embedded.data[17] = hexDigits[p_Value & 0x0F];
        result.embedded.length = 18;

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
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

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = 18;
        return result;
    }
}

mla_string_t mla_string_from_uint8_hex_short(mla_uint8_t p_Value) {
    const mla_size_t maxLength = 2; // Max: "FF"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        if (p_Value == 0) {
            result.embedded.data[0] = '0';
            result.embedded.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint8_t value = p_Value;

        while (value > 0) {
            result.embedded.data[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        result.embedded.length = static_cast<mla_uint8_t>(index);

        // Reverse in place
        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = result.embedded.data[i];
            result.embedded.data[i] = result.embedded.data[index - 1 - i];
            result.embedded.data[index - 1 - i] = temp;
        }

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
        mla_char_t* buffer = mla_create_char_array(bufferSize);

        if (buffer == nullptr) {
            return mla_string_empty();
        }

        if (p_Value == 0) {
            buffer[0] = '0';
            buffer[1] = '\0';
            mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
            result.heap.data = buffer;
            result.heap.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint8_t value = p_Value;

        while (value > 0) {
            buffer[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        buffer[index] = '\0';

        // Reverse in place
        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = buffer[i];
            buffer[i] = buffer[index - 1 - i];
            buffer[index - 1 - i] = temp;
        }

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint16_hex_short(mla_uint16_t p_Value) {
    const mla_size_t maxLength = 4; // Max: "FFFF"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        if (p_Value == 0) {
            result.embedded.data[0] = '0';
            result.embedded.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint16_t value = p_Value;

        while (value > 0) {
            result.embedded.data[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        result.embedded.length = static_cast<mla_uint8_t>(index);

        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = result.embedded.data[i];
            result.embedded.data[i] = result.embedded.data[index - 1 - i];
            result.embedded.data[index - 1 - i] = temp;
        }

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
        mla_char_t* buffer = mla_create_char_array(bufferSize);

        if (buffer == nullptr) {
            return mla_string_empty();
        }

        if (p_Value == 0) {
            buffer[0] = '0';
            buffer[1] = '\0';
            mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
            result.heap.data = buffer;
            result.heap.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint16_t value = p_Value;

        while (value > 0) {
            buffer[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        buffer[index] = '\0';

        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = buffer[i];
            buffer[i] = buffer[index - 1 - i];
            buffer[index - 1 - i] = temp;
        }

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint32_hex_short(mla_uint32_t p_Value) {
    const mla_size_t maxLength = 8; // Max: "FFFFFFFF"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        if (p_Value == 0) {
            result.embedded.data[0] = '0';
            result.embedded.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint32_t value = p_Value;

        while (value > 0) {
            result.embedded.data[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        result.embedded.length = static_cast<mla_uint8_t>(index);

        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = result.embedded.data[i];
            result.embedded.data[i] = result.embedded.data[index - 1 - i];
            result.embedded.data[index - 1 - i] = temp;
        }

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
        mla_char_t* buffer = mla_create_char_array(bufferSize);

        if (buffer == nullptr) {
            return mla_string_empty();
        }

        if (p_Value == 0) {
            buffer[0] = '0';
            buffer[1] = '\0';
            mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
            result.heap.data = buffer;
            result.heap.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint32_t value = p_Value;

        while (value > 0) {
            buffer[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        buffer[index] = '\0';

        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = buffer[i];
            buffer[i] = buffer[index - 1 - i];
            buffer[index - 1 - i] = temp;
        }

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}

mla_string_t mla_string_from_uint64_hex_short(mla_uint64_t p_Value) {
    const mla_size_t maxLength = 16; // Max: "FFFFFFFFFFFFFFFF"

    if (maxLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_buffer_reference_noOwner(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        if (p_Value == 0) {
            result.embedded.data[0] = '0';
            result.embedded.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint64_t value = p_Value;

        while (value > 0) {
            result.embedded.data[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        result.embedded.length = static_cast<mla_uint8_t>(index);

        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = result.embedded.data[i];
            result.embedded.data[i] = result.embedded.data[index - 1 - i];
            result.embedded.data[index - 1 - i] = temp;
        }

        return result;
    } else {
        const mla_size_t bufferSize = maxLength + 1;
        mla_char_t* buffer = mla_create_char_array(bufferSize);

        if (buffer == nullptr) {
            return mla_string_empty();
        }

        if (p_Value == 0) {
            buffer[0] = '0';
            buffer[1] = '\0';
            mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
            result.heap.data = buffer;
            result.heap.length = 1;
            return result;
        }

        mla_size_t index = 0;
        mla_uint64_t value = p_Value;

        while (value > 0) {
            buffer[index++] = hexDigits[value & 0xF];
            value >>= 4;
        }
        buffer[index] = '\0';

        for (mla_size_t i = 0; i < index / 2; ++i) {
            mla_char_t temp = buffer[i];
            buffer[i] = buffer[index - 1 - i];
            buffer[index - 1 - i] = temp;
        }

        mla_string_t result = {mla_buffer_reference(buffer), {{MLA_STRING_MEMORY_LAYOUT_HEAP_C_STRING, 0, {0}}}};
        result.heap.data = buffer;
        result.heap.length = index;
        return result;
    }
}
