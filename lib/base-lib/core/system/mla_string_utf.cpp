//
// This file contains the implementation of UTF-8 string handling functions
//
// Created by chris on 9/14/2025.
//

#include "mla_string.h"
#include "../log/mla_logging.h"

mla_multi_byte_char_t mla_string_multi_byte_char_at(const mla_string_t &p_String, mla_size_t p_Index) {

    mla_multi_byte_char_t result = {{0}};

    mla_size_t length = mla_string_length(p_String);
    const mla_char_t* data = mla_string_data(p_String);

    if (!data || length == 0) {
        return result;
    }

    mla_size_t byteIndex = 0;
    mla_size_t charIndex = 0;

    // Advance to the start byte of the desired UTF-8 character
    while (byteIndex < length && charIndex < p_Index) {
        mla_uint8_t first = static_cast<mla_uint8_t>(data[byteIndex]);
        mla_size_t advance = 1;

        if      ((first & 0xE0) == 0xC0) advance = 2;      // 110xxxxx
        else if ((first & 0xF0) == 0xE0) advance = 3;      // 1110xxxx
        else if ((first & 0xF8) == 0xF0) advance = 4;      // 11110xxx
        // ASCII and invalid leading bytes both advance by 1

        byteIndex += advance;
        charIndex++;
    }

    // Out of range or did not reach desired character
    if (byteIndex >= length || charIndex != p_Index) {
        return result;
    }

    // Determine length of current character
    mla_uint8_t first = static_cast<mla_uint8_t>(data[byteIndex]);
    mla_size_t byteCount = 1;

    if      ((first & 0xE0) == 0xC0) byteCount = 2;
    else if ((first & 0xF0) == 0xE0) byteCount = 3;
    else if ((first & 0xF8) == 0xF0) byteCount = 4;
    // ASCII and invalid leading bytes both use 1 byte

    // Clamp if truncated
    if (byteIndex + byteCount > length) {
        byteCount = length - byteIndex;
    }

    for (mla_size_t i = 0; i < byteCount; ++i) {
        result.bytes[i] = data[byteIndex + i];
    }

    return result;

}

mla_size_t mla_string_multi_byte_char_count(const mla_string_t &p_String) {

    mla_size_t charCount = 0;

    mla_size_t length = mla_string_length(p_String);
    const mla_char_t* data = mla_string_data(p_String);

    for (mla_size_t i = 0; i < length; ++i) {

        // Count only bytes that are not continuation bytes (10xxxxxx)
        // This counts start of characters or ASCII characters
        if ((data[i] & 0xC0) != 0x80) {
            charCount++;
        }
    }

    return charCount;

}

const mla_utf_16_char_t* mla_string_utf16_data(const mla_string_utf16_buffer_t &p_Utf16Buffer) {
    return mla_pointer_get_data<mla_utf_16_char_t>(p_Utf16Buffer.data);
}

const mla_utf_32_char_t* mla_string_utf32_data(const mla_string_utf32_buffer_t &p_Utf32Buffer) {
    return mla_pointer_get_data<mla_utf_32_char_t>(p_Utf32Buffer.data);
}


mla_string_utf16_buffer_t mla_string_to_utf16_buffer(const mla_string_t &p_String) {

    mla_size_t length = mla_string_length(p_String);
    const mla_char_t* data = mla_string_data(p_String);

    if (length == 0) {
        return {mla_pointer_null(), 0}; // Empty string, return empty buffer
    }

    mla_size_t realCharCount = mla_string_multi_byte_char_count(p_String);
    // Allocate potentially more space for surrogate pairs
    mla_size_t maxSize = realCharCount * 2 + 1; // Max possible size (if all are surrogate pairs) + null terminator
    mla_pointer_t buffer= mla_malloc(sizeof(mla_utf_16_char_t) * maxSize, nullptr, mla_dynamic_data_empty());


    mla_utf_16_char_t* utf16_data = mla_pointer_get_data<mla_utf_16_char_t>(buffer);

    if (utf16_data == nullptr) {
        mla_error(mla_string_const("Failed to get data pointer for UTF-16 buffer."));
        return {mla_pointer_null(), 0};
    }

    mla_size_t byteIndex = 0;
    mla_size_t utf16Index = 0;

    // Process each UTF-8 character
    while (byteIndex < length) {
        mla_uint32_t codePoint = 0;
        mla_uint8_t firstByte = static_cast<mla_uint8_t>(data[byteIndex]);

        if ((firstByte & 0x80) == 0) {
            // 1-byte ASCII character
            codePoint = firstByte;
            byteIndex += 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (byteIndex + 1 < length) {
                codePoint = ((firstByte & 0x1F) << 6) | (static_cast<mla_uint8_t>(data[byteIndex + 1]) & 0x3F);
                byteIndex += 2;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (byteIndex + 2 < length) {
                codePoint = ((firstByte & 0x0F) << 12) |
                           ((static_cast<mla_uint8_t>(data[byteIndex + 1]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(data[byteIndex + 2]) & 0x3F);
                byteIndex += 3;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            if (byteIndex + 3 < length) {
                codePoint = ((firstByte & 0x07) << 18) |
                           ((static_cast<mla_uint8_t>(data[byteIndex + 1]) & 0x3F) << 12) |
                           ((static_cast<mla_uint8_t>(data[byteIndex + 2]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(data[byteIndex + 3]) & 0x3F);
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
            utf16_data[utf16Index++] = static_cast<mla_utf_16_char_t>(codePoint);
        } else {
            // Supplementary plane - surrogate pair
            codePoint -= 0x10000;
            utf16_data[utf16Index++] = static_cast<mla_utf_16_char_t>(0xD800 + (codePoint >> 10));
            utf16_data[utf16Index++] = static_cast<mla_utf_16_char_t>(0xDC00 + (codePoint & 0x3FF));
        }
    }

    // Add null terminator
    utf16_data[utf16Index] = 0;

    return {
        buffer,
        realCharCount
    };
}

mla_string_t mla_string_from_utf16_buffer(const mla_string_utf16_buffer_t &p_Utf16Buffer) {

    if (p_Utf16Buffer.charCount == 0) {
        return mla_string_empty();
    }

    // Calculate maximum possible UTF-8 size (worst case: 4 bytes per character)
    mla_size_t maxUtf8Size = p_Utf16Buffer.charCount * 4;

    mla_utf_16_char_t* uft_16_data = mla_pointer_get_data<mla_utf_16_char_t>(p_Utf16Buffer.data);

    if (!uft_16_data) {
        return mla_string_empty();
    }

    // Try SSO first - use embedded buffer directly
    if (maxUtf8Size <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t utf8Index = 0;
        mla_size_t utf16Index = 0;

        while (utf16Index < p_Utf16Buffer.charCount && uft_16_data[utf16Index] != 0) {
            mla_uint32_t codePoint;
            mla_utf_16_char_t c = uft_16_data[utf16Index++];

            // Handle surrogate pairs
            if (c >= 0xD800 && c <= 0xDBFF && utf16Index < p_Utf16Buffer.charCount) {
                mla_utf_16_char_t low = uft_16_data[utf16Index];
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    codePoint = 0x10000 + (((c - 0xD800) << 10) | (low - 0xDC00));
                    utf16Index++;
                } else {
                    codePoint = 0xFFFD;
                }
            } else if (c >= 0xDC00 && c <= 0xDFFF) {
                codePoint = 0xFFFD;
            } else {
                codePoint = c;
            }

            // Encode to UTF-8
            if (codePoint < 0x80) {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(codePoint);
            } else if (codePoint < 0x800) {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0xC0 | (codePoint >> 6));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
            } else if (codePoint < 0x10000) {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0xE0 | (codePoint >> 12));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
            } else {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0xF0 | (codePoint >> 18));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 12) & 0x3F));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
            }
        }

        result.embedded.length = static_cast<mla_uint8_t>(utf8Index);
        return result;
    }

    // Allocate heap buffer for larger strings
    mla_pointer_t utf8Buffer = mla_create_char_array(maxUtf8Size);

    mla_char_t* utf8_data = mla_pointer_get_data<mla_char_t>(utf8Buffer);

    if (!utf8_data) {
        return mla_string_empty();
    }

    mla_size_t utf8Index = 0;
    mla_size_t utf16Index = 0;

    while (utf16Index < p_Utf16Buffer.charCount && uft_16_data[utf16Index] != 0) {
        mla_uint32_t codePoint;
        mla_utf_16_char_t c = uft_16_data[utf16Index++];

        if (c >= 0xD800 && c <= 0xDBFF && utf16Index < p_Utf16Buffer.charCount) {
            mla_utf_16_char_t low = uft_16_data[utf16Index];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                codePoint = 0x10000 + (((c - 0xD800) << 10) | (low - 0xDC00));
                utf16Index++;
            } else {
                codePoint = 0xFFFD;
            }
        } else if (c >= 0xDC00 && c <= 0xDFFF) {
            codePoint = 0xFFFD;
        } else {
            codePoint = c;
        }

        if (codePoint < 0x80) {
            utf8_data[utf8Index++] = static_cast<mla_char_t>(codePoint);
        } else if (codePoint < 0x800) {
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0xC0 | (codePoint >> 6));
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else if (codePoint < 0x10000) {
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0xE0 | (codePoint >> 12));
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else {
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0xF0 | (codePoint >> 18));
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 12) & 0x3F));
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            utf8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        }
    }

    mla_string_t result = {utf8Buffer, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = utf8Index;
    return result;
}

mla_string_utf32_buffer_t mla_string_to_utf32_buffer(const mla_string_t &p_String) {

    mla_size_t length = mla_string_length(p_String);

    if (length == 0) {
        return {mla_pointer_null(), 0}; // Empty string, return empty buffer
    }

    mla_size_t realCharCount = mla_string_multi_byte_char_count(p_String);

    mla_pointer_t buffer = mla_malloc(sizeof(mla_utf_32_char_t) * (realCharCount + 1), nullptr, mla_dynamic_data_empty()); // +1 for null terminator

    mla_utf_32_char_t* utf32_data = mla_pointer_get_data<mla_utf_32_char_t>(buffer);

    if (utf32_data == nullptr) {
        return {mla_pointer_null(), 0};
    }

    mla_size_t byteIndex = 0;
    mla_size_t utf32Index = 0;
    const mla_char_t* data = mla_string_data(p_String);

    // Process each UTF-8 character
    while (byteIndex < length) {
        mla_uint32_t codePoint = 0;
        mla_uint8_t firstByte = static_cast<mla_uint8_t>(data[byteIndex]);

        if ((firstByte & 0x80) == 0) {
            // 1-byte ASCII character
            codePoint = firstByte;
            byteIndex += 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (byteIndex + 1 < length) {
                codePoint = ((firstByte & 0x1F) << 6) | (static_cast<mla_uint8_t>(data[byteIndex + 1]) & 0x3F);
                byteIndex += 2;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (byteIndex + 2 < length) {
                codePoint = ((firstByte & 0x0F) << 12) |
                           ((static_cast<mla_uint8_t>(data[byteIndex + 1]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(data[byteIndex + 2]) & 0x3F);
                byteIndex += 3;
            } else {
                // Incomplete sequence
                codePoint = 0xFFFD; // Replacement character
                byteIndex += 1;
            }
        } else if ((firstByte & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            if (byteIndex + 3 < length) {
                codePoint = ((firstByte & 0x07) << 18) |
                           ((static_cast<mla_uint8_t>(data[byteIndex + 1]) & 0x3F) << 12) |
                           ((static_cast<mla_uint8_t>(data[byteIndex + 2]) & 0x3F) << 6) |
                           (static_cast<mla_uint8_t>(data[byteIndex + 3]) & 0x3F);
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
        utf32_data[utf32Index++] = static_cast<mla_utf_32_char_t>(codePoint);
    }

    // Add null terminator
    utf32_data[utf32Index] = 0;

    return {
        buffer,
        realCharCount
    };

}

mla_string_t mla_string_from_utf32_buffer(const mla_string_utf32_buffer_t &p_Utf32Buffer) {

    if (p_Utf32Buffer.charCount == 0) {
        return mla_string_empty();
    }

    // Calculate maximum possible UTF-8 size (worst case: 4 bytes per character)
    mla_size_t maxUtf8Size = p_Utf32Buffer.charCount * 4;

    mla_utf_32_char_t* utf32_data = mla_pointer_get_data<mla_utf_32_char_t>(p_Utf32Buffer.data);

    if (utf32_data == nullptr) {
        return mla_string_empty();
    }

    // Try SSO first - use embedded buffer directly
    if (maxUtf8Size <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};

        mla_size_t utf8Index = 0;
        mla_size_t utf32Index = 0;

        while (utf32Index < p_Utf32Buffer.charCount && utf32_data[utf32Index] != 0) {
            mla_uint32_t codePoint = utf32_data[utf32Index++];

            // Validate code point
            if (codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF)) {
                codePoint = 0xFFFD; // Replacement character
            }

            // Encode to UTF-8
            if (codePoint < 0x80) {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(codePoint);
            } else if (codePoint < 0x800) {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0xC0 | (codePoint >> 6));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
            } else if (codePoint < 0x10000) {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0xE0 | (codePoint >> 12));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
            } else {
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0xF0 | (codePoint >> 18));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 12) & 0x3F));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
                result.embedded.data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
            }
        }

        result.embedded.length = static_cast<mla_uint8_t>(utf8Index);
        return result;
    }

    // Allocate heap buffer for larger strings
    mla_pointer_t utf8Buffer = mla_create_char_array(maxUtf8Size);
    mla_char_t* uft8_data = mla_pointer_get_data<mla_char_t>(utf8Buffer);

    if (uft8_data == nullptr) {
        return mla_string_empty();
    }

    mla_size_t utf8Index = 0;
    mla_size_t utf32Index = 0;

    while (utf32Index < p_Utf32Buffer.charCount && utf32_data[utf32Index] != 0) {
        mla_uint32_t codePoint = utf32_data[utf32Index++];

        if (codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF)) {
            codePoint = 0xFFFD;
        }

        if (codePoint < 0x80) {
            uft8_data[utf8Index++] = static_cast<mla_char_t>(codePoint);
        } else if (codePoint < 0x800) {
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0xC0 | (codePoint >> 6));
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else if (codePoint < 0x10000) {
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0xE0 | (codePoint >> 12));
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        } else {
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0xF0 | (codePoint >> 18));
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 12) & 0x3F));
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | ((codePoint >> 6) & 0x3F));
            uft8_data[utf8Index++] = static_cast<mla_char_t>(0x80 | (codePoint & 0x3F));
        }
    }

    mla_string_t result = {utf8Buffer, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = utf8Index;
    return result;
}