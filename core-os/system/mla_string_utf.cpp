//
// This file contains the implementation of UTF-8 string handling functions
//
// Created by chris on 9/14/2025.
//

#include "mla_string.h"
#include "../log/mla_logging.h"

mla_multi_byte_char_t mla_string_multi_byte_char_at(const mla_string_t &p_String, mla_size_t p_Index) {

    mla_multi_byte_char_t result = {{0}};

    if (!p_String.data || p_String.length == 0) {
        return result;
    }

    mla_size_t byteIndex = 0;
    mla_size_t charIndex = 0;

    // Advance to the start byte of the desired UTF-8 character
    while (byteIndex < p_String.length && charIndex < p_Index) {
        mla_uint8_t first = static_cast<mla_uint8_t>(p_String.data[byteIndex]);
        mla_size_t advance = 1;

        if      ((first & 0x80) == 0x00) advance = 1;      // 0xxxxxxx
        else if ((first & 0xE0) == 0xC0) advance = 2;      // 110xxxxx
        else if ((first & 0xF0) == 0xE0) advance = 3;      // 1110xxxx
        else if ((first & 0xF8) == 0xF0) advance = 4;      // 11110xxx
        else advance = 1; // Invalid leading byte, treat as single

        byteIndex += advance;
        charIndex++;
    }

    // Out of range or did not reach desired character
    if (byteIndex >= p_String.length || charIndex != p_Index) {
        return result;
    }

    // Determine length of current character
    mla_uint8_t first = static_cast<mla_uint8_t>(p_String.data[byteIndex]);
    mla_size_t byteCount = 1;
    if      ((first & 0x80) == 0x00) byteCount = 1;
    else if ((first & 0xE0) == 0xC0) byteCount = 2;
    else if ((first & 0xF0) == 0xE0) byteCount = 3;
    else if ((first & 0xF8) == 0xF0) byteCount = 4;
    else byteCount = 1;

    // Clamp if truncated
    if (byteIndex + byteCount > p_String.length) {
        byteCount = p_String.length - byteIndex;
    }

    for (mla_size_t i = 0; i < byteCount; ++i) {
        result.bytes[i] = p_String.data[byteIndex + i];
    }

    return result;

}

mla_size_t mla_string_multi_byte_char_count(const mla_string_t &p_String) {

    mla_size_t charCount = 0;

    for (mla_size_t i = 0; i < p_String.length; ++i) {

        // Count only bytes that are not continuation bytes (10xxxxxx)
        // This counts start of characters or ASCII characters
        if ((p_String.data[i] & 0xC0) != 0x80) {
            charCount++;
        }
    }

    return charCount;

}


mla_string_utf16_buffer_t mla_string_to_utf16_buffer(mla_string_t &p_String) {

    if (p_String.length == 0) {
        return {nullptr, 0}; // Empty string, return empty buffer
    }

    mla_size_t realCharCount = mla_string_multi_byte_char_count(p_String);
    // Allocate potentially more space for surrogate pairs
    mla_size_t maxSize = realCharCount * 2 + 1; // Max possible size (if all are surrogate pairs) + null terminator
    mla_utf_16_char_t* buffer = static_cast<mla_utf_16_char_t*>(mla_malloc(sizeof(mla_utf_16_char_t) * maxSize));

    if (buffer == nullptr) {
        mla_error(mla_string_const("Failed to allocate memory for UTF-16 buffer."));
        return {nullptr, 0};
    }

    mla_size_t byteIndex = 0;
    mla_size_t utf16Index = 0;

    // Process each UTF-8 character
    while (byteIndex < p_String.length) {
        mla_uint32_t codePoint = 0;
        mla_uint8_t firstByte = static_cast<mla_uint8_t>(p_String.data[byteIndex]);

        if ((firstByte & 0x80) == 0) {
            // 1-byte ASCII character
            codePoint = firstByte;
            byteIndex += 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (byteIndex + 1 < p_String.length) {
                codePoint = ((firstByte & 0x1F) << 6) | (static_cast<mla_uint8_t>(p_String.data[byteIndex + 1]) & 0x3F);
                byteIndex += 2;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (byteIndex + 2 < p_String.length) {
                codePoint = ((firstByte & 0x0F) << 12) |
                           ((static_cast<mla_uint8_t>(p_String.data[byteIndex + 1]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(p_String.data[byteIndex + 2]) & 0x3F);
                byteIndex += 3;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            if (byteIndex + 3 < p_String.length) {
                codePoint = ((firstByte & 0x07) << 18) |
                           ((static_cast<mla_uint8_t>(p_String.data[byteIndex + 1]) & 0x3F) << 12) |
                           ((static_cast<mla_uint8_t>(p_String.data[byteIndex + 2]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(p_String.data[byteIndex + 3]) & 0x3F);
                byteIndex += 4;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else {
            // Invalid UTF-8 leading byte
            codePoint = 0xFFFD; // Replacement character
            byteIndex += 1;
        }

        // Convert codePoint to UTF-16
        if (codePoint <= 0xFFFF) {
            // BMP character - single UTF-16 code unit
            buffer[utf16Index++] = static_cast<mla_utf_16_char_t>(codePoint);
        } else {
            // Supplementary plane - surrogate pair
            codePoint -= 0x10000;
            buffer[utf16Index++] = static_cast<mla_utf_16_char_t>(0xD800 + (codePoint >> 10));
            buffer[utf16Index++] = static_cast<mla_utf_16_char_t>(0xDC00 + (codePoint & 0x3FF));
        }
    }

    // Add null terminator
    buffer[utf16Index] = 0;

    return {
        buffer,
        realCharCount
    };
}

mla_string_t mla_string_from_utf16_buffer(const mla_string_utf16_buffer_t &p_Utf16Buffer) {

if (p_Utf16Buffer.data == nullptr || p_Utf16Buffer.charCount == 0) {
        return mla_string_empty();
    }

    // Calculate maximum possible UTF-8 size (worst case: 4 bytes per character)
    mla_size_t maxUtf8Size = p_Utf16Buffer.charCount * 4;
    mla_char_t* utf8Buffer = mla_create_char_array(maxUtf8Size);

    if (utf8Buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t utf8Index = 0;
    mla_size_t utf16Index = 0;

    while (utf16Index < p_Utf16Buffer.charCount && p_Utf16Buffer.data[utf16Index] != 0) {
        mla_uint32_t codePoint;
        mla_utf_16_char_t c = p_Utf16Buffer.data[utf16Index++];

        // Check if this is a surrogate pair
        if (c >= 0xD800 && c <= 0xDBFF && utf16Index < p_Utf16Buffer.charCount) {
            // High surrogate
            mla_utf_16_char_t low = p_Utf16Buffer.data[utf16Index];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                // Valid surrogate pair
                codePoint = 0x10000 + (((c - 0xD800) << 10) | (low - 0xDC00));
                utf16Index++; // Consume the low surrogate
            } else {
                // Invalid surrogate pair
                codePoint = 0xFFFD; // Replacement character
            }
        } else if (c >= 0xDC00 && c <= 0xDFFF) {
            // Unpaired low surrogate
            codePoint = 0xFFFD; // Replacement character
        } else {
            // Regular BMP character
            codePoint = c;
        }

        // Encode code point as UTF-8
        if (codePoint < 0x80) {
            // 1-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(codePoint);
        } else if (codePoint < 0x800) {
            // 2-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0xC0 | (codePoint >> 6));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else if (codePoint < 0x10000) {
            // 3-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0xE0 | (codePoint >> 12));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else {
            // 4-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0xF0 | (codePoint >> 18));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 12) & 0x3F));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        }
    }

    return {
        utf8Buffer,
        utf8Index,
        MLA_STRING_MEMORY_LAYOUT_BUFFER,
        mla_buffer_reference(utf8Buffer)

    };

}

void mla_string_utf16_buffer_destroy(mla_string_utf16_buffer_t &p_Buffer) {

    mla_free(p_Buffer.data); // Clean up allocated memory
    p_Buffer.data = nullptr;
    p_Buffer.charCount = 0;
}

mla_string_utf32_buffer_t mla_string_to_utf32_buffer(mla_string_t &p_String) {

    if (p_String.length == 0) {
        return {nullptr, 0}; // Empty string, return empty buffer
    }

    mla_size_t realCharCount = mla_string_multi_byte_char_count(p_String);
    mla_utf_32_char_t* buffer = static_cast<mla_utf_32_char_t*>(mla_malloc(sizeof(mla_utf_32_char_t) * (realCharCount + 1))); // +1 for null terminator

    if (buffer == nullptr) {
        return {nullptr, 0};
    }

    mla_size_t byteIndex = 0;
    mla_size_t utf32Index = 0;

    // Process each UTF-8 character
    while (byteIndex < p_String.length) {
        mla_uint32_t codePoint = 0;
        mla_uint8_t firstByte = static_cast<mla_uint8_t>(p_String.data[byteIndex]);

        if ((firstByte & 0x80) == 0) {
            // 1-byte ASCII character
            codePoint = firstByte;
            byteIndex += 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (byteIndex + 1 < p_String.length) {
                codePoint = ((firstByte & 0x1F) << 6) | (static_cast<mla_uint8_t>(p_String.data[byteIndex + 1]) & 0x3F);
                byteIndex += 2;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (byteIndex + 2 < p_String.length) {
                codePoint = ((firstByte & 0x0F) << 12) |
                           ((static_cast<mla_uint8_t>(p_String.data[byteIndex + 1]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(p_String.data[byteIndex + 2]) & 0x3F);
                byteIndex += 3;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            if (byteIndex + 3 < p_String.length) {
                codePoint = ((firstByte & 0x07) << 18) |
                           ((static_cast<mla_uint8_t>(p_String.data[byteIndex + 1]) & 0x3F) << 12) |
                           ((static_cast<mla_uint8_t>(p_String.data[byteIndex + 2]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(p_String.data[byteIndex + 3]) & 0x3F);
                byteIndex += 4;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else {
            // Invalid UTF-8 leading byte
            codePoint = 0xFFFD; // Replacement character
            byteIndex += 1;
        }

        // For UTF-32, we just store the code point directly
        buffer[utf32Index++] = static_cast<mla_utf_32_char_t>(codePoint);
    }

    // Add null terminator
    buffer[utf32Index] = 0;

    return {
        buffer,
        realCharCount
    };

}

mla_string_t mla_string_from_utf32_buffer(const mla_string_utf32_buffer_t &p_Utf32Buffer) {

if (p_Utf32Buffer.data == nullptr || p_Utf32Buffer.charCount == 0) {
        return mla_string_empty();
    }

    // Calculate maximum possible UTF-8 size (worst case: 4 bytes per character)
    mla_size_t maxUtf8Size = p_Utf32Buffer.charCount * 4;
    mla_char_t* utf8Buffer = mla_create_char_array(maxUtf8Size);

    if (utf8Buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t utf8Index = 0;
    mla_size_t utf32Index = 0;

    while (utf32Index < p_Utf32Buffer.charCount && p_Utf32Buffer.data[utf32Index] != 0) {
        mla_uint32_t codePoint = p_Utf32Buffer.data[utf32Index++];

        // Validate code point
        if (codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF)) {
            codePoint = 0xFFFD; // Replacement character for invalid code points
        }

        // Encode code point as UTF-8
        if (codePoint < 0x80) {
            // 1-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(codePoint);
        } else if (codePoint < 0x800) {
            // 2-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0xC0 | (codePoint >> 6));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else if (codePoint < 0x10000) {
            // 3-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0xE0 | (codePoint >> 12));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else {
            // 4-byte sequence
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0xF0 | (codePoint >> 18));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 12) & 0x3F));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            utf8Buffer[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        }
    }

    return {
        utf8Buffer,
        utf8Index,
        MLA_STRING_MEMORY_LAYOUT_BUFFER,
        mla_buffer_reference(utf8Buffer)
    };

}

void mla_string_utf32_buffer_destroy(mla_string_utf32_buffer_t &p_Buffer) {
    mla_free(p_Buffer.data); // Clean up allocated memory
    p_Buffer.data = nullptr;
    p_Buffer.charCount = 0;
}