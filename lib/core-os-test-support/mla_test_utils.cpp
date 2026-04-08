//
// Created by chris on 1/9/2026.
//

#include "mla_test_utils.h"

mla_test_pointer_t mla_test_memcpy(mla_test_pointer_t dest, const mla_test_pointer_t src, mla_test_uint32_t n) {
    mla_test_uint8_t* d = (mla_test_uint8_t*)dest;
    const mla_test_uint8_t* s = (const mla_test_uint8_t*)src;
    for (mla_test_uint32_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Helper to copy memory (handles overlapping)
mla_test_pointer_t mla_test_memmove(mla_test_pointer_t dest, const mla_test_pointer_t src, mla_test_uint32_t n) {
    mla_test_uint8_t* d = (mla_test_uint8_t*)dest;
    const mla_test_uint8_t* s = (const mla_test_uint8_t*)src;

    if (d < s) {
        // Copy forward
        for (mla_test_uint32_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copy backward to handle overlap
        for (mla_test_uint32_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

// Helper to set memory to a value
mla_test_pointer_t mla_test_memset(mla_test_pointer_t ptr, mla_test_int32_t value, mla_test_uint32_t n) {
    mla_test_uint8_t* p = (mla_test_uint8_t*)ptr;
    for (mla_test_uint32_t i = 0; i < n; i++) {
        p[i] = (mla_test_uint8_t)value;
    }
    return ptr;
}

// Helper to compare strings
mla_test_int32_t mla_test_strcmp(const mla_test_char_t* str1, const mla_test_char_t* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (mla_test_int32_t)((mla_test_uint8_t)*str1 - (mla_test_uint8_t)*str2);
}

mla_test_uint32_t mla_test_strlen(const mla_test_char_t* str) {
    const mla_test_char_t* s = str;
    while (*s) {
        s++;
    }
    return (mla_test_uint32_t)(s - str);
}

// Helper function to convert uint32 to string and return length
mla_test_uint32_t mla_uint32_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint32_t value) {
    if (buffer_size == 0) return 0;

    // Handle zero case
    if (value == 0) {
        if (buffer_size > 0) buffer[0] = '0';
        return 1;
    }

    // Count digits
    mla_test_uint32_t temp = value;
    mla_test_uint32_t digit_count = 0;
    while (temp > 0) {
        digit_count++;
        temp /= 10;
    }

    if (digit_count >= buffer_size) return 0;

    // Fill buffer from right to left
    mla_test_uint32_t pos = digit_count;
    temp = value;
    while (temp > 0) {
        buffer[--pos] = '0' + (temp % 10);
        temp /= 10;
    }

    return digit_count;
}

// Helper function to convert int16 to string and return length
mla_test_uint32_t mla_int16_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_int16_t value) {
    if (buffer_size == 0) return 0;

    mla_test_uint32_t offset = 0;

    // Handle negative numbers
    if (value < 0) {
        buffer[offset++] = '-';
        if (value == -32768) {
            // Special case for minimum int16_t
            const mla_test_char_t* min_str = "32768";
            mla_test_uint32_t len = 0;
            while (min_str[len] && offset < buffer_size) {
                buffer[offset++] = min_str[len++];
            }
            return offset;
        }
        value = -value;
    }

    // Convert absolute value using uint32 function
    mla_test_uint32_t len = mla_uint32_to_string(buffer + offset, buffer_size - offset, (mla_test_uint32_t)value);
    return offset + len;
}

// Helper function to convert uint16 to string and return length
mla_test_uint32_t mla_uint16_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint16_t value) {
    return mla_uint32_to_string(buffer, buffer_size, (mla_test_uint32_t)value);
}

// Helper function to convert int32 to string and return length
mla_test_uint32_t mla_int32_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_int32_t value) {
    if (buffer_size == 0) return 0;

    mla_test_uint32_t offset = 0;

    // Handle negative numbers
    if (value < 0) {
        buffer[offset++] = '-';
        if (value == (mla_test_int32_t)(-2147483647 - 1)) {
            // Special case for minimum int32_t
            const mla_test_char_t* min_str = "2147483648";
            mla_test_uint32_t len = 0;
            while (min_str[len] && offset < buffer_size) {
                buffer[offset++] = min_str[len++];
            }
            return offset;
        }
        value = -value;
    }

    // Convert absolute value using uint32 function
    mla_test_uint32_t len = mla_uint32_to_string(buffer + offset, buffer_size - offset, (mla_test_uint32_t)value);
    return offset + len;
}

// Helper function to convert uint64 to string and return length
mla_test_uint32_t mla_uint64_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint64_t value) {
    if (buffer_size == 0) return 0;

    // Handle zero case
    if (value == 0) {
        if (buffer_size > 0) buffer[0] = '0';
        return 1;
    }

    // Count digits
    mla_test_uint64_t temp = value;
    mla_test_uint32_t digit_count = 0;
    while (temp > 0) {
        digit_count++;
        temp /= 10;
    }

    if (digit_count >= buffer_size) return 0;

    // Fill buffer from right to left
    mla_test_uint32_t pos = digit_count;
    temp = value;
    while (temp > 0) {
        buffer[--pos] = '0' + (temp % 10);
        temp /= 10;
    }

    return digit_count;
}

// Helper function to convert int64 to string and return length
mla_test_uint32_t mla_int64_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_int64_t value) {
    if (buffer_size == 0) return 0;

    mla_test_uint32_t offset = 0;

    // Handle negative numbers
    if (value < 0) {
        buffer[offset++] = '-';
        if (value == -9223372036854775807LL - 1) {
            // Special case for minimum int64_t
            const mla_test_char_t* min_str = "9223372036854775808";
            mla_test_uint32_t len = 0;
            while (min_str[len] && offset < buffer_size) {
                buffer[offset++] = min_str[len++];
            }
            return offset;
        }
        value = -value;
    }

    // Convert absolute value using uint64 function
    mla_test_uint32_t len = mla_uint64_to_string(buffer + offset, buffer_size - offset, (mla_test_uint64_t)value);
    return offset + len;
}

// Helper function to convert float to string with precision
mla_test_uint32_t mla_float_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_float_t value, mla_test_uint32_t precision) {
    if (buffer_size == 0) return 0;

    mla_test_uint32_t offset = 0;

    // Handle negative numbers
    if (value < 0) {
        buffer[offset++] = '-';
        value = -value;
    }

    // Get integer part
    mla_test_int32_t int_part = (mla_test_int32_t)value;
    offset += mla_int32_to_string(buffer + offset, buffer_size - offset, int_part);

    // Add decimal point
    if (offset < buffer_size - 1) {
        buffer[offset++] = '.';
    }

    // Get fractional part
    mla_test_float_t frac_part = value - int_part;
    for (mla_test_uint32_t i = 0; i < precision && offset < buffer_size - 1; i++) {
        frac_part *= 10;
        mla_test_int32_t digit = (mla_test_int32_t)frac_part;
        buffer[offset++] = (mla_test_char_t)('0' + digit);
        frac_part -= digit;
    }

    return offset;
}

// Helper function to convert double to string with precision
mla_test_uint32_t mla_double_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_double_t value, mla_test_uint32_t precision) {
    if (buffer_size == 0) return 0;

    mla_test_uint32_t offset = 0;

    // Handle negative numbers
    if (value < 0) {
        buffer[offset++] = '-';
        value = -value;
    }

    // Get integer part
    mla_test_int64_t int_part = (mla_test_int64_t)value;
    offset += mla_int64_to_string(buffer + offset, buffer_size - offset, int_part);

    // Add decimal point
    if (offset < buffer_size - 1) {
        buffer[offset++] = '.';
    }

    // Get fractional part
    mla_test_double_t frac_part = value - int_part;
    for (mla_test_uint32_t i = 0; i < precision && offset < buffer_size - 1; i++) {
        frac_part *= 10;
        mla_test_int32_t digit = (mla_test_int32_t)frac_part;
        buffer[offset++] = (mla_test_char_t)('0' + digit);
        frac_part -= digit;
    }

    return offset;
}

// Helper function to convert pointer to hex string
mla_test_uint32_t mla_pointer_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, void* ptr) {
    if (buffer_size < 3) return 0;

    mla_test_uint32_t offset = 0;

    // Add "0x" prefix
    buffer[offset++] = '0';
    buffer[offset++] = 'x';

    // Convert pointer to unsigned integer
    mla_test_uint64_t addr = (mla_test_uint64_t)ptr;

    if (addr == 0) {
        buffer[offset++] = '0';
        return offset;
    }

    // Convert to hex
    const mla_test_char_t* hex_digits = "0123456789abcdef";
    mla_test_char_t hex_buffer[16];
    mla_test_uint32_t hex_len = 0;

    while (addr > 0 && hex_len < 16) {
        hex_buffer[hex_len++] = hex_digits[addr % 16];
        addr /= 16;
    }

    // Reverse and copy to output buffer
    for (mla_test_uint32_t i = 0; i < hex_len && offset < buffer_size - 1; i++) {
        buffer[offset++] = hex_buffer[hex_len - 1 - i];
    }

    return offset;
}

// Helper to append a string to a buffer
mla_test_uint32_t mla_test_strcat(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint32_t offset, const mla_test_char_t* str) {
    mla_test_uint32_t count = 0;
    while (str[count] && (offset + count) < (buffer_size - 1)) {
        buffer[offset + count] = str[count];
        count++;
    }
    return count;
}

