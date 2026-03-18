//
// Created by chris on 9/17/2025.
//

#include "mla_bytes.h"

mla_bytes_t mla_bytes_empty() {
    return {
        nullptr,
        0,
        mla_buffer_reference_noOwner()
    };
}

mla_bytes_t mla_bytes(mla_size_t p_Length) {

    if (p_Length == 0) {
        return mla_bytes_empty();
    }

    mla_byte_t* buffer = reinterpret_cast<mla_byte_t*>(mla_malloc(p_Length));

    if (buffer == nullptr) {
        return mla_bytes_empty();
    }

    mla_memset(buffer, 0, p_Length);

    return {
        buffer,
        p_Length,
        mla_buffer_reference(buffer)
    };
}


mla_bytes_t mla_bytes_from_external_buffer(mla_byte_t* p_Data, const mla_size_t p_Size) {

    if (p_Data == nullptr || p_Size == 0) {
        return mla_bytes_empty();
    }

    return {
        p_Data,
        p_Size,
        mla_buffer_reference_noOwner()
    };

}

// This function creates bytes from a buffer and takes ownership of the buffer
// You must not free the buffer after calling this function
mla_bytes_t mla_bytes_from_buffer_with_ownership(mla_byte_t* p_Data, const mla_size_t p_Size) {

    if (p_Data == nullptr) {
        return mla_bytes_empty();
    }

    return {
        p_Data,
        p_Size,
        mla_buffer_reference(p_Data)
    };
}

mla_bytes_t mla_bytes_copy(const mla_bytes_t& p_Bytes) {

    if (p_Bytes.size == 0 || p_Bytes.data == nullptr) {
        return mla_bytes_empty();
    }

    mla_byte_t* buffer = reinterpret_cast<mla_byte_t*>(mla_malloc(p_Bytes.size));

    if (buffer == nullptr) {
        return mla_bytes_empty();
    }

    mla_memcpy(buffer, p_Bytes.data, p_Bytes.size);

    return {
        buffer,
        p_Bytes.size,
        mla_buffer_reference(buffer)
    };

}

const mla_byte_t* mla_bytes_get_data_readonly(const mla_bytes_t& p_Bytes) {
    return p_Bytes.data;
}

mla_byte_t* mla_bytes_get_data_for_writing(mla_bytes_t& p_Bytes) {

    return const_cast<mla_byte_t*>(p_Bytes.data);

}

mla_size_t mla_bytes_length(const mla_bytes_t& p_Bytes) {

    return p_Bytes.size;
}

mla_bool_t mla_bytes_is_empty(const mla_bytes_t& p_Bytes) {

    return p_Bytes.data == nullptr || p_Bytes.size == 0;

}

void mla_bytes_destroy(mla_bytes_t& p_Bytes) {

    if (p_Bytes.data == nullptr) {
        return; // Nothing to free
    }

    p_Bytes.data = nullptr; // Clear the pointer
    p_Bytes.dataOwner = mla_buffer_reference_noOwner();
    p_Bytes.size = 0; // Reset the length

}

mla_string_t mla_bytes_to_string(const mla_bytes_t& p_Bytes) {

    if (p_Bytes.data == nullptr || p_Bytes.size == 0) {
        return mla_string_empty();
    }

    mla_string_t result = {p_Bytes.dataOwner, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.data = reinterpret_cast<const mla_char_t*>(p_Bytes.data);
    result.heap.length = p_Bytes.size;
    return result;

}

mla_bytes_t mla_bytes_from_string(const mla_string_t& p_String) {

    mla_size_t length = mla_string_length(p_String);
    const mla_char_t* data = mla_string_data(p_String);

    if (data == nullptr || length == 0) {
        return mla_bytes_empty();
    }

    if (p_String.embedded.memoryLayout == MLA_STRING_MEMORY_LAYOUT_EMBEDDED) {
        // We need to copy for case because the string is using SSO and we can't guarantee its lifetime

        mla_byte_t* buffer = reinterpret_cast<mla_byte_t*>(mla_malloc(length));

        if (buffer == nullptr) {
            return mla_bytes_empty();
        }

        mla_memcpy(buffer, data, length);
        return {
            buffer,
            length,
            mla_buffer_reference(buffer)
        };

    } else {
        const mla_byte_t* buffer = reinterpret_cast<const mla_byte_t*>(data);

        return {
            buffer,
            length,
            p_String.dataOwner
        };
    }
}

mla_string_t mla_bytes_to_base64(const mla_bytes_t& p_Bytes) {

    if (p_Bytes.data == nullptr || p_Bytes.size == 0) {
        return mla_string_empty();
    }

    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Calculate output length: (input_length * 4 + 2) / 3 rounded up
    mla_size_t output_length = ((p_Bytes.size + 2) / 3) * 4;

    mla_char_t* buffer = mla_create_char_array(output_length + 1); // +1 for null terminator
    if (buffer == nullptr) {
        return mla_string_empty();
    }

    mla_size_t output_pos = 0;

    for (mla_size_t i = 0; i < p_Bytes.size; i += 3) {
        mla_uint32_t triple = 0;
        mla_size_t bytes_in_group = 0;

        // Read up to 3 bytes
        for (mla_size_t j = 0; j < 3 && (i + j) < p_Bytes.size; ++j) {
            triple = (triple << 8) | p_Bytes.data[i + j];
            ++bytes_in_group;
        }

        // Pad to 3 bytes
        triple <<= (3 - bytes_in_group) * 8;

        // Extract 4 6-bit values
        buffer[output_pos++] = base64_chars[(triple >> 18) & 0x3F];
        buffer[output_pos++] = base64_chars[(triple >> 12) & 0x3F];
        buffer[output_pos++] = (bytes_in_group > 1) ? base64_chars[(triple >> 6) & 0x3F] : '=';
        buffer[output_pos++] = (bytes_in_group > 2) ? base64_chars[triple & 0x3F] : '=';
    }

    buffer[output_length] = '\0';

    return mla_string_from_buffer_with_ownership(buffer, output_length);

}

mla_bytes_t mla_bytes_from_base64(const mla_string_t& p_Base64String) {

    mla_size_t base64StringLength = mla_string_length(p_Base64String);
    const mla_char_t* base64StringData = mla_string_data(p_Base64String);

    if (base64StringData == nullptr || base64StringLength == 0) {
        return mla_bytes_empty();
    }

    // Static decode table - initialized only once
    static mla_int32_t decode_table[256] = {0};
    static mla_bool_t table_initialized = false;

    if (!table_initialized) {
        // Initialize all entries to -1
        for (mla_size_t i = 0; i < 256; ++i) {
            decode_table[i] = -1;
        }

        // Fill decode table
        for (mla_size_t i = 0; i < 26; ++i) {
            decode_table['A' + i] = i;
            decode_table['a' + i] = i + 26;
        }
        for (mla_size_t i = 0; i < 10; ++i) {
            decode_table['0' + i] = i + 52;
        }
        decode_table[static_cast<mla_uint8_t>('+')] = 62;
        decode_table[static_cast<mla_uint8_t>('/')] = 63;
        decode_table[static_cast<mla_uint8_t>('=')] = 0; // Padding

        table_initialized = true;
    }

    // Rest of the function remains the same...
    mla_size_t input_length = base64StringLength;
    mla_size_t padding = 0;

    if (input_length >= 2) {
        if (base64StringData[input_length - 1] == '=') padding++;
        if (base64StringData[input_length - 2] == '=') padding++;
    }

    mla_size_t output_length = (input_length * 3) / 4 - padding;

    if (output_length == 0) {
        return mla_bytes_empty();
    }

    mla_byte_t* buffer = reinterpret_cast<mla_byte_t*>(mla_malloc(output_length));
    if (buffer == nullptr) {
        return mla_bytes_empty();
    }

    mla_size_t output_pos = 0;

    for (mla_size_t i = 0; i < input_length; i += 4) {
        mla_uint32_t quad = 0;
        mla_size_t valid_chars = 0;

        for (mla_size_t j = 0; j < 4 && (i + j) < input_length; ++j) {
            mla_char_t c = base64StringData[i + j];
            mla_int32_t val = decode_table[static_cast<mla_uint8_t>(c)];

            if (val >= 0) {
                quad = (quad << 6) | val;
                if (c != '=') valid_chars++;
            } else {
                mla_free(buffer);
                return mla_bytes_empty();
            }
        }

        if (valid_chars >= 2 && output_pos < output_length) {
            buffer[output_pos++] = (quad >> 16) & 0xFF;
        }
        if (valid_chars >= 3 && output_pos < output_length) {
            buffer[output_pos++] = (quad >> 8) & 0xFF;
        }
        if (valid_chars >= 4 && output_pos < output_length) {
            buffer[output_pos++] = quad & 0xFF;
        }
    }

    return mla_bytes_from_buffer_with_ownership(buffer, output_length);

}