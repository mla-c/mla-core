//
// Created by chris on 9/17/2025.
//

#include "mla_bytes.h"

mla_bytes_t mla_bytes_empty() {
    return {
        mla_pointer_null(),
        0
    };
}

mla_bytes_t mla_bytes(mla_size_t p_Length) {

    if (p_Length == 0) {
        return mla_bytes_empty();
    }

    mla_pointer_t buffer = mla_malloc_buffer(sizeof(mla_byte_t) * p_Length);
    mla_byte_t* data = mla_pointer_get_data<mla_byte_t>(buffer);

    if (data == nullptr) {
        return mla_bytes_empty();
    }

    return {
        buffer,
        p_Length
    };
}


mla_bytes_t mla_bytes_from_external_buffer(mla_pointer_t& p_Data, const mla_size_t p_Size) {

    if (mla_pointer_is_null(p_Data) || p_Size == 0) {
        return mla_bytes_empty();
    }

    return {
        p_Data,
        p_Size
    };

}


mla_bytes_t mla_bytes_copy(const mla_bytes_t& p_Bytes) {

    if (mla_bytes_is_empty(p_Bytes)) {
        return mla_bytes_empty();
    }

    mla_pointer_t newBuffer = mla_malloc_buffer(p_Bytes.size);
    mla_byte_t* old_buffer_data = mla_pointer_get_data<mla_byte_t>(p_Bytes.heap_data);
    mla_byte_t* new_buffer_data = mla_pointer_get_data<mla_byte_t>(newBuffer);

    if (old_buffer_data == nullptr || new_buffer_data == nullptr) {
        return mla_bytes_empty();
    }

    mla_memcpy(new_buffer_data, old_buffer_data, p_Bytes.size);

    return {
        newBuffer,
        p_Bytes.size
    };

}

const mla_byte_t* mla_bytes_get_data_readonly(const mla_bytes_t& p_Bytes) {
    return mla_pointer_get_data<mla_byte_t>(p_Bytes.heap_data);
}

mla_byte_t* mla_bytes_get_data_for_writing(mla_bytes_t& p_Bytes) {

    return mla_pointer_get_data<mla_byte_t>(p_Bytes.heap_data);
}

mla_size_t mla_bytes_length(const mla_bytes_t& p_Bytes) {

    return p_Bytes.size;
}

mla_bool_t mla_bytes_is_empty(const mla_bytes_t& p_Bytes) {

    return p_Bytes.size == 0 || mla_pointer_is_null(p_Bytes.heap_data);

}

void mla_bytes_destroy(mla_bytes_t& p_Bytes) {

    if (!mla_pointer_is_null(p_Bytes.heap_data)) {
        p_Bytes.heap_data = mla_pointer_null();
    }

    p_Bytes.size = 0; // Reset the length

}

mla_string_t mla_bytes_to_string(const mla_bytes_t& p_Bytes) {

    if (mla_bytes_is_empty(p_Bytes)) {
        return mla_string_empty();
    }

    mla_string_t result = {p_Bytes.heap_data, {{MLA_STRING_MEMORY_LAYOUT_BUFFER, 0, {0}}}};
    result.heap.char_offset = 0;
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

        mla_pointer_t buffer = mla_malloc_buffer(sizeof(mla_byte_t) * length);

        mla_byte_t* buffer_data = mla_pointer_get_data<mla_byte_t>(buffer);

        if (buffer_data == nullptr) {
            return mla_bytes_empty();
        }

        mla_memcpy(buffer_data, data, length);
        return {
            buffer,
            length
        };

    } else {

        return {
            p_String.data_storage,
            length
        };
    }
}

mla_string_t mla_bytes_to_base64(const mla_bytes_t& p_Bytes) {

    if (mla_bytes_is_empty(p_Bytes)) {
        return mla_string_empty();
    }

    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Calculate output length: (input_length * 4 + 2) / 3 rounded up
    mla_size_t output_length = ((p_Bytes.size + 2) / 3) * 4;

    mla_pointer_t buffer = mla_create_char_array(output_length + 1); // +1 for null terminator
    mla_char_t* buffer_data = mla_pointer_get_data<mla_char_t>(buffer);

    if (buffer_data == nullptr) {
        return mla_string_empty();
    }

    const mla_byte_t* bytes_data = mla_bytes_get_data_readonly(p_Bytes);

    if (bytes_data == nullptr) {
        return mla_string_empty();
    }

    mla_size_t output_pos = 0;

    for (mla_size_t i = 0; i < p_Bytes.size; i += 3) {
        mla_uint32_t triple = 0;
        mla_size_t bytes_in_group = 0;

        // Read up to 3 bytes
        for (mla_size_t j = 0; j < 3 && (i + j) < p_Bytes.size; ++j) {
            triple = (triple << 8) | bytes_data[i + j];
            ++bytes_in_group;
        }

        // Pad to 3 bytes
        triple <<= (3 - bytes_in_group) * 8;

        // Extract 4 6-bit values
        buffer_data[output_pos++] = base64_chars[(triple >> 18) & 0x3F];
        buffer_data[output_pos++] = base64_chars[(triple >> 12) & 0x3F];
        buffer_data[output_pos++] = (bytes_in_group > 1) ? base64_chars[(triple >> 6) & 0x3F] : '=';
        buffer_data[output_pos++] = (bytes_in_group > 2) ? base64_chars[triple & 0x3F] : '=';
    }

    buffer_data[output_length] = '\0';

    return mla_string(buffer, output_length);

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
            decode_table['A' + i] = static_cast<mla_int32_t>(i);
            decode_table['a' + i] = static_cast<mla_int32_t>(i) + 26;
        }
        for (mla_size_t i = 0; i < 10; ++i) {
            decode_table['0' + i] = static_cast<mla_int32_t>(i) + 52;
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

    mla_pointer_t buffer = mla_malloc_buffer(sizeof(mla_byte_t) * output_length);
    mla_byte_t* buffer_data = mla_pointer_get_data<mla_byte_t>(buffer);

    if (buffer_data == nullptr) {
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
                return mla_bytes_empty();
            }
        }

        if (valid_chars >= 2 && output_pos < output_length) {
            buffer_data[output_pos++] = (quad >> 16) & 0xFF;
        }
        if (valid_chars >= 3 && output_pos < output_length) {
            buffer_data[output_pos++] = (quad >> 8) & 0xFF;
        }
        if (valid_chars >= 4 && output_pos < output_length) {
            buffer_data[output_pos++] = quad & 0xFF;
        }
    }

    return mla_bytes_from_external_buffer(buffer, output_length);

}