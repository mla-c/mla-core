//
// Created by christian on 9/18/25.
//

#include "mla_json_serializer.h"

#include "../log/mla_logging.h"
#include "../system/mla_number.h"
#include "../system/mla_string.h"
#include "../system/mla_string_concat.h"

#define mla_bytes_prefix "mla_bytes:"

enum mla_json_serializer_element_type_t: mla_uint8_t {
    MLA_JSON_SERIALIZER_ELEMENT_NONE = 0,
    MLA_JSON_SERIALIZER_ELEMENT_START_LIST = 1,
    MLA_JSON_SERIALIZER_ELEMENT_END_LIST = 2,

    MLA_JSON_SERIALIZER_ELEMENT_START_OBJECT = 3,
    MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT = 4,

    MLA_JSON_SERIALIZER_ELEMENT_PROPERTY_NAME = 5,
    MLA_JSON_SERIALIZER_ELEMENT_VALUE = 6
};

mla_user_data_id_init(mla_json_serializer_element_type_user_data_name)

mla_bool_t __mla_json_serializer_write_comma_if_needed(mla_serializer_t &instance,
                                                 mla_json_serializer_element_type_t new_type) {

    mla_json_serializer_element_type_t last_type = (mla_json_serializer_element_type_t)mla_user_data_get_and_replace_int8(instance.user_data, mla_json_serializer_element_type_user_data_name, new_type, MLA_JSON_SERIALIZER_ELEMENT_NONE);


    switch (new_type) {
        case MLA_JSON_SERIALIZER_ELEMENT_NONE:
            break;
        case MLA_JSON_SERIALIZER_ELEMENT_START_LIST:
            if (last_type == MLA_JSON_SERIALIZER_ELEMENT_VALUE || last_type == MLA_JSON_SERIALIZER_ELEMENT_END_LIST ||
                last_type == MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT) {
                if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>(",")) != 1) {
                    return false;
                }
            }
            break;
        case MLA_JSON_SERIALIZER_ELEMENT_END_LIST:
            // No comma needed
            break;
        case MLA_JSON_SERIALIZER_ELEMENT_START_OBJECT:
            if (last_type == MLA_JSON_SERIALIZER_ELEMENT_VALUE || last_type == MLA_JSON_SERIALIZER_ELEMENT_END_LIST ||
                last_type == MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT) {
                if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>(",")) != 1) {
                    return false;
                }
            }
            break;
        case MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT:
            // No comma needed
            break;
        case MLA_JSON_SERIALIZER_ELEMENT_PROPERTY_NAME:
            if (last_type == MLA_JSON_SERIALIZER_ELEMENT_VALUE || last_type == MLA_JSON_SERIALIZER_ELEMENT_END_LIST ||
                last_type == MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT) {
                if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>(",")) != 1) {
                    return false;
                }
            }
            break;
        case MLA_JSON_SERIALIZER_ELEMENT_VALUE:
            if (last_type == MLA_JSON_SERIALIZER_ELEMENT_VALUE || last_type == MLA_JSON_SERIALIZER_ELEMENT_END_LIST ||
                last_type == MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT) {
                if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>(",")) != 1) {
                    return false;
                }
            }
            break;
    }

    return true;
}


mla_bool_t mla_json_serializer_write_start_struct(mla_serializer_t &instance) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_START_OBJECT)) {
        return false;
    }

    return instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("{")) == 1;
}

mla_bool_t mla_json_serializer_write_end_struct(mla_serializer_t &instance) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_END_OBJECT)) {
        return false;
    }

    return instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("}")) == 1;
}

mla_bool_t mla_json_serializer_write_start_list(mla_serializer_t &instance) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_START_LIST)) {
        return false;
    }

    return instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("[")) == 1;
}

mla_bool_t mla_json_serializer_write_end_list(mla_serializer_t &instance) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_END_LIST)) {
        return false;
    }
    return instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("]")) == 1;
}

mla_bool_t mla_json_serializer_write_property_name(mla_serializer_t &instance, const mla_string_t &name) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_PROPERTY_NAME)) {
        return false;
    }

    if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("\"")) != 1) {
        return false;
    }

    mla_size_t length = mla_string_length(name);

    if (length > 0) {
        const mla_char_t* data = mla_string_data(name);

        if (instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) != length) {
            return false;
        }
    }

    if (instance.output.write(instance.output, 0, 2, reinterpret_cast<const mla_byte_t *>("\":")) != 2) {
        return false;
    }

    return true;
}

mla_bool_t mla_json_serializer_write_bool(mla_serializer_t &instance, const mla_bool_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }

    if (value) {
        return instance.output.write(instance.output, 0, 4, reinterpret_cast<const mla_byte_t *>("true")) == 4;
    } else {
        return instance.output.write(instance.output, 0, 5, reinterpret_cast<const mla_byte_t *>("false")) == 5;
    }

}

mla_bool_t mla_json_serializer_write_int8(mla_serializer_t &instance, const mla_int8_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }

    mla_string_t str = mla_string_from_int8(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_int16(mla_serializer_t &instance, const mla_int16_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_int16(value);

    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_int32(mla_serializer_t &instance, const mla_int32_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_int32(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_int64(mla_serializer_t &instance, const mla_int64_t value) {

    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_int64(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_uint8(mla_serializer_t &instance, const mla_uint8_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_uint8(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_uint16(mla_serializer_t &instance, const mla_uint16_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_uint16(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;

}

mla_bool_t mla_json_serializer_write_uint32(mla_serializer_t &instance, const mla_uint32_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE) ) {
        return false;
    }
    mla_string_t str = mla_string_from_uint32(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;

}

mla_bool_t mla_json_serializer_write_uint64(mla_serializer_t &instance, const mla_uint64_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_uint64(value);
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_float(mla_serializer_t &instance, const mla_float_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_float(value, 6); // Default to 6 decimal places
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;

}

mla_bool_t mla_json_serializer_write_double(mla_serializer_t &instance, const mla_double_t value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    mla_string_t str = mla_string_from_double(value, 6); // Default to 6 decimal places
    mla_size_t length = mla_string_length(str);
    const mla_char_t* data = mla_string_data(str);
    return instance.output.write(instance.output, 0, length, reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t __mla_json_serializer_write_string_plain(mla_serializer_t &instance, const mla_string_t &value) {

    mla_size_t length = mla_string_length(value);

    if (length == 0)
        return true;

    const mla_char_t* data = mla_string_data(value);

    return instance.output.write(instance.output, 0, length,
                              reinterpret_cast<const mla_byte_t *>(data)) == length;
}

mla_bool_t mla_json_serializer_write_string(mla_serializer_t &instance, const mla_string_t &value) {

    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }

    if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("\"")) != 1) {
        return false;
    }

    if (!__mla_json_serializer_write_string_plain(instance, value) ) {
        return false;
    }

    return instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("\"")) == 1;

}

mla_bool_t mla_json_serializer_write_bytes(mla_serializer_t &instance, const mla_bytes_t &value) {
    if (!__mla_json_serializer_write_comma_if_needed(instance, MLA_JSON_SERIALIZER_ELEMENT_VALUE)) {
        return false;
    }
    // Serialize bytes as a base64 string
    mla_string_t base64Str = mla_bytes_to_base64(value);

    if (instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("\"")) != 1) {
        return false;
    }

    if (!__mla_json_serializer_write_string_plain(instance, mla_string_const(mla_bytes_prefix))) {
        return false;
    }

    if (!__mla_json_serializer_write_string_plain(instance, base64Str)) {
        return false;
    }

    return instance.output.write(instance.output, 0, 1, reinterpret_cast<const mla_byte_t *>("\"")) == 1;
}

mla_serializer_t mla_json_serializer(const mla_stream_output_t &output) {
    return {
        output,
        mla_user_data_empty(),
        mla_json_serializer_write_start_struct,
        mla_json_serializer_write_end_struct,
        mla_json_serializer_write_start_list,
        mla_json_serializer_write_end_list,
        mla_json_serializer_write_property_name,
        mla_json_serializer_write_bool,
        mla_json_serializer_write_int8,
        mla_json_serializer_write_int16,
        mla_json_serializer_write_int32,
        mla_json_serializer_write_int64,
        mla_json_serializer_write_uint8,
        mla_json_serializer_write_uint16,
        mla_json_serializer_write_uint32,
        mla_json_serializer_write_uint64,
        mla_json_serializer_write_float,
        mla_json_serializer_write_double,
        mla_json_serializer_write_string,
        mla_json_serializer_write_bytes
    };
}


mla_user_data_id_init(mla_json_deserializer_char_buffer_user_data_name)

mla_bool_t __mla_json_deserializer_read_next_non_whitespace_char(mla_deserializer_t &instance, mla_char_t &out_char) {
    // Check if there is something in buffer
    mla_char_t buffered_char = mla_user_data_get_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);

    if (buffered_char != 0) {

        mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);

        if (buffered_char != ' ' && buffered_char != '\n' && buffered_char != '\r' && buffered_char != '\t') {
            // Skip whitespace
            out_char = buffered_char;
            return true;
        }

    }


    mla_char_t new_char = 0;

    while (true) {
        if (instance.input.read(instance.input, 0, 1, reinterpret_cast<mla_byte_t *>(&new_char)) != 1) {
            return false;
        }

        if (new_char == ' ' || new_char == '\n' || new_char == '\r' || new_char == '\t') {
            // Skip whitespace
            continue;
        }

        break;
    }

    out_char = new_char;
    return true;
}

mla_size_t __mla_json_deserializer_read_next_data(mla_deserializer_t &instance, mla_char_t *charBuffer,
                                                  const mla_size_t length) {
    mla_size_t offset;
    mla_size_t lengthRemaining;

    if (length == 0)
        return 0;

    mla_char_t buffered_char = mla_user_data_get_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);

    // Read buffer if exists
    if (buffered_char != 0) {
        charBuffer[0] = buffered_char;
        mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);
        offset = 1;
        lengthRemaining = length - offset;
    } else {
        offset = 0;
        lengthRemaining = length;
    }

    const mla_size_t readed = instance.input.read(instance.input, offset * sizeof(mla_char_t), lengthRemaining * sizeof(mla_char_t),
                                                  reinterpret_cast<mla_byte_t *>(charBuffer));
    return (readed /  sizeof(mla_char_t)) + offset;
}

mla_bool_t __mla_json_deserializer_string_match(mla_deserializer_t &instance, const mla_char_t *data,
                                                const mla_size_t length) {
    if (length > 4)
        return false;

    // The current max is 5/ never more
    mla_char_t charBuffer[4];

    if (__mla_json_deserializer_read_next_data(instance, charBuffer, length) != length) {
        return false;
    }

    return mla_memcmp(&charBuffer, data, length) == 0;
}


mla_bool_t __mla_json_deserializer_read_string_data(mla_deserializer_t &instance, mla_string_t &out_str) {
    mla_char_t charBuffer[mla_stream_fast_read_buffer_size];
    mla_size_t position = 0;
    out_str = mla_string_empty();

    mla_char_t buffered_char = mla_user_data_get_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);

    // Read buffer if exists
    if (buffered_char != 0) {
        charBuffer[0] = buffered_char;
        mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);
        position = 1;
    }

    while (true) {
        // Add more space in case of unicode parsing
        if (position + 4 >= mla_stream_fast_read_buffer_size) {
            // Buffer full, append to string
            out_str = mla_string_concat(out_str, mla_string(charBuffer, position));
            position = 0;
        }

        // Read next char
        if (instance.input.read(instance.input, position * sizeof(mla_char_t), 1 * sizeof(mla_char_t),
                                reinterpret_cast<mla_byte_t *>(&charBuffer)) != 1 * sizeof(mla_char_t)) {
            return false;
        }

        if (charBuffer[position] == '"') {
            // End of string
            break;
        }

        if (charBuffer[position] < 0x20) {
            mla_error("Control character in string")
            return false;
        }


        if (charBuffer[position] == '\\') {
            // Read next char
            if (instance.input.read(instance.input, position * sizeof(mla_char_t), 1 * sizeof(mla_char_t),
                                    reinterpret_cast<mla_byte_t *>(&charBuffer)) != 1 * sizeof(mla_char_t)) {
                return false;
            }

            switch (charBuffer[position]) {
                case '"':
                case '\\':
                case '/':
                    break;
                case 'b':
                    charBuffer[position] = '\b';
                    break;
                case 'f':
                    charBuffer[position] = '\f';
                    break;
                case 'n':
                    charBuffer[position] = '\n';
                    break;
                case 'r':
                    charBuffer[position] = '\r';
                    break;
                case 't':
                    charBuffer[position] = '\t';
                    break;
                case 'u': {
                    if (instance.input.read(instance.input, position * sizeof(mla_char_t), 4 * sizeof(mla_char_t),
                                            reinterpret_cast<mla_byte_t *>(&charBuffer)) != 4 * sizeof(mla_char_t)) {
                        return false;
                    }

                    // Parse 4-digit hex value into a Unicode code point
                    mla_uint16_t unicode_val = 0;
                    for (mla_size_t i = 0; i < 4; i++) {
                        mla_char_t hex_digit = charBuffer[position + i];
                        unicode_val <<= 4;

                        if (hex_digit >= '0' && hex_digit <= '9') {
                            unicode_val |= (hex_digit - '0');
                        } else if (hex_digit >= 'a' && hex_digit <= 'f') {
                            unicode_val |= (hex_digit - 'a' + 10);
                        } else if (hex_digit >= 'A' && hex_digit <= 'F') {
                            unicode_val |= (hex_digit - 'A' + 10);
                        } else {
                            mla_error("Invalid Unicode escape sequence")
                            return false;
                        }
                    }

                    // Check for surrogate pairs
                    if (unicode_val >= 0xD800 && unicode_val <= 0xDBFF) {
                        // This is a high surrogate, we need to read the low surrogate
                        mla_char_t next_chars[6];
                        if (instance.input.read(instance.input, 0, 6 * sizeof(mla_char_t),
                                                reinterpret_cast<mla_byte_t *>(next_chars)) != 6 * sizeof(mla_char_t)) {
                            return false;
                        }

                        // Check if the next sequence is \uXXXX
                        if (next_chars[0] != '\\' || next_chars[1] != 'u') {
                            mla_error("Expected low surrogate pair after high surrogate")
                            return false;
                        }

                        // Parse the low surrogate
                        mla_uint16_t low_surrogate = 0;
                        for (mla_size_t i = 0; i < 4; i++) {
                            mla_char_t hex_digit = next_chars[2 + i];
                            low_surrogate <<= 4;

                            if (hex_digit >= '0' && hex_digit <= '9') {
                                low_surrogate |= (hex_digit - '0');
                            } else if (hex_digit >= 'a' && hex_digit <= 'f') {
                                low_surrogate |= (hex_digit - 'a' + 10);
                            } else if (hex_digit >= 'A' && hex_digit <= 'F') {
                                low_surrogate |= (hex_digit - 'A' + 10);
                            } else {
                                mla_error("Invalid Unicode escape sequence in low surrogate")
                                return false;
                            }
                        }

                        // Validate low surrogate
                        if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                            mla_error("Invalid low surrogate value")
                            return false;
                        }

                        // Calculate the actual code point from the surrogate pair
                        mla_uint32_t code_point = 0x10000 + ((unicode_val - 0xD800) << 10) + (low_surrogate - 0xDC00);

                        // Convert to UTF-8
                        if (code_point < 0x80) {
                            // 1-byte character
                            charBuffer[position] = (mla_char_t) code_point;
                            position++;
                        } else if (code_point < 0x800) {
                            // 2-byte character
                            charBuffer[position] = (mla_char_t) (0xC0 | (code_point >> 6));
                            charBuffer[position + 1] = (mla_char_t) (0x80 | (code_point & 0x3F));
                            position += 2;
                        } else if (code_point < 0x10000) {
                            // 3-byte character
                            charBuffer[position] = (mla_char_t) (0xE0 | (code_point >> 12));
                            charBuffer[position + 1] = (mla_char_t) (0x80 | ((code_point >> 6) & 0x3F));
                            charBuffer[position + 2] = (mla_char_t) (0x80 | (code_point & 0x3F));
                            position += 3;
                        } else {
                            // 4-byte character
                            charBuffer[position] = (mla_char_t) (0xF0 | (code_point >> 18));
                            charBuffer[position + 1] = (mla_char_t) (0x80 | ((code_point >> 12) & 0x3F));
                            charBuffer[position + 2] = (mla_char_t) (0x80 | ((code_point >> 6) & 0x3F));
                            charBuffer[position + 3] = (mla_char_t) (0x80 | (code_point & 0x3F));
                            position += 4;
                        }
                    } else {
                        // Regular Unicode character (not a surrogate pair)
                        if (unicode_val < 0x80) {
                            // 1-byte character
                            charBuffer[position] = (mla_char_t) unicode_val;
                            position++;
                        } else if (unicode_val < 0x800) {
                            // 2-byte character
                            charBuffer[position] = (mla_char_t) (0xC0 | (unicode_val >> 6));
                            charBuffer[position + 1] = (mla_char_t) (0x80 | (unicode_val & 0x3F));
                            position += 2;
                        } else {
                            // 3-byte character
                            charBuffer[position] = (mla_char_t) (0xE0 | (unicode_val >> 12));
                            charBuffer[position + 1] = (mla_char_t) (0x80 | ((unicode_val >> 6) & 0x3F));
                            charBuffer[position + 2] = (mla_char_t) (0x80 | (unicode_val & 0x3F));
                            position += 3;
                        }
                    }

                    // We already processed the 4 digits of the escape sequence
                    // (and potentially 6 more characters if it was a surrogate pair)
                    position--; // Adjust for the outer loop increment
                    break;
                }
                default:
                    mla_error("Invalid Unicode escape sequence")
                    return false;
            }
        } else {
            position++;
        }
    }

    // Append remaining data
    if (position > 0) {
        out_str = mla_string_concat(out_str, mla_string(charBuffer, position));
    }


    return true;
}

mla_bool_t __mla_json_deserializer_read_number_data(mla_deserializer_t &instance) {
    mla_char_t charBuffer[64]; // Buffer for storing the number string
    mla_size_t position = 0;
    mla_bool_t is_negative = false;
    mla_bool_t has_decimal = false;
    mla_bool_t has_exponent = false;

    mla_char_t buffered_char = mla_user_data_get_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);

    // Read buffer if exists
    if (buffered_char != 0) {
        charBuffer[0] = buffered_char;
        mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, 0);

        // Check if negative
        if (charBuffer[0] == '-') {
            is_negative = true;
        }

        position = 1;
    }

    // Read the entire number into the buffer
    while (position < 63) {
        mla_char_t c;
        if (instance.input.read(instance.input, 0, 1,
                                reinterpret_cast<mla_byte_t *>(&c)) != 1) {
            break;
        }

        if ((c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') {
            charBuffer[position++] = c;

            if (c == '.') {
                has_decimal = true;
            }

            if (c == 'e' || c == 'E') {
                has_exponent = true;
            }
        } else {
            // Put back non-number character
            mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, c);
            break;
        }
    }

    // Null-terminate the string
    charBuffer[position] = '\0';

    // Validate number format
    for (mla_size_t i = 0; i < position; i++) {
        char c = charBuffer[i];

        // Check for invalid combinations
        if (i == 0 && c == '.') {
            mla_error("Number cannot start with decimal point");
            return false;
        }

        if (c == '.' && i > 0 && has_decimal && charBuffer[i - 1] == '.') {
            mla_error("Multiple decimal points in number");
            return false;
        }

        if ((c == 'e' || c == 'E') && (i == 0 || (i > 0 && (charBuffer[i - 1] < '0' || charBuffer[i - 1] > '9')))) {
            mla_error("Invalid exponent format");
            return false;
        }

        if ((c == '+' || c == '-') && i > 0 && charBuffer[i - 1] != 'e' && charBuffer[i - 1] != 'E') {
            mla_error("Sign character in invalid position");
            return false;
        }
    }

    // Determine the number type and parse
    if (has_decimal || has_exponent) {
        // Parse as floating point
        mla_double_t value;

        if (!mla_parse_double(mla_string(charBuffer, position), value)) {
            mla_error("Invalid floating point number");
            return false;
        }

        // Determine if we should store as float or double based on value
        if (value <= mla_float_max && value >= mla_float_min) {
            instance.current_token.type = MLA_DESERIALIZER_VALUE_FLOAT;
            instance.current_token.simple.float_value = (mla_float_t) value;
        } else {
            instance.current_token.type = MLA_DESERIALIZER_VALUE_DOUBLE;
            instance.current_token.simple.double_value = value;
        }
    } else {
        // Parse as integer
        mla_int64_t value;
        if (!mla_parse_int64(mla_string(charBuffer, position), value)) {
            mla_error("Invalid integer number");
            return false;
        }

        // Determine the appropriate integer type based on value range
        if (is_negative) {
            if (value >= mla_int8_min && value <= mla_int8_max) {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_INT8;
                instance.current_token.simple.int8_value = (mla_int8_t) value;
            } else if (value >= mla_int16_min && value <= mla_int16_max) {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_INT16;
                instance.current_token.simple.int16_value = (mla_int16_t) value;
            } else if (value >= mla_int32_min && value <= mla_int32_max) {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_INT32;
                instance.current_token.simple.int32_value = (mla_int32_t) value;
            } else {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_INT64;
                instance.current_token.simple.int64_value = value;
            }
        } else {
            // Unsigned - reinterpret as unsigned
            mla_uint64_t uvalue = (mla_uint64_t) value;

            if (uvalue <= mla_uint8_max) {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_UINT8;
                instance.current_token.simple.uint8_value = (mla_uint8_t) uvalue;
            } else if (uvalue <= mla_uint16_max) {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_UINT16;
                instance.current_token.simple.uint16_value = (mla_uint16_t) uvalue;
            } else if (uvalue <= mla_uint32_max) {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_UINT32;
                instance.current_token.simple.uint32_value = (mla_uint32_t) uvalue;
            } else {
                instance.current_token.type = MLA_DESERIALIZER_VALUE_UINT64;
                instance.current_token.simple.uint64_value = uvalue;
            }
        }
    }

    return true;
}

mla_bool_t mla_json_deserializer_read_read_next(mla_deserializer_t &instance) {
    // Reset current token data
    instance.current_token.complex = {
        mla_string_empty(),
        mla_string_empty(),
        mla_bytes_empty()
    };
    instance.current_token.simple = {0};

    while (true) {
        mla_char_t new_char = 0;

        if (!__mla_json_deserializer_read_next_non_whitespace_char(instance, new_char)) {
            return false;
        }


        switch (new_char) {
            case '{':
                instance.current_token.type = MLA_DESERIALIZER_STRUCT_START;
                return true;

            case '}':
                instance.current_token.type = MLA_DESERIALIZER_STRUCT_END;
                return true;
            case '[':
                instance.current_token.type = MLA_DESERIALIZER_LIST_START;
                return true;

            case ']':
                instance.current_token.type = MLA_DESERIALIZER_LIST_END;
                return true;

            case '"': {
                mla_string_t str_data = mla_string_empty();

                if (!__mla_json_deserializer_read_string_data(instance, str_data)) {
                    return false;
                }

                if (!__mla_json_deserializer_read_next_non_whitespace_char(instance, new_char)) {
                    return false;
                }

                // Determine if this is a property name or a string value
                // Dependenty on the current token_type
                if (new_char == ':') {
                    instance.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
                    instance.current_token.complex.property_name = str_data;
                    return true;
                } else {

                    mla_string_t bytes_prefix = mla_string_const(mla_bytes_prefix);

                    if (mla_string_starts_with(str_data, bytes_prefix)) {

                        instance.current_token.type = MLA_DESERIALIZER_VALUE_BYTES;

                        // This is a bytes value
                        mla_string_t base64Part = mla_string_substr(str_data, mla_string_length(bytes_prefix));
                        instance.current_token.complex.bytes_value = mla_bytes_from_base64(base64Part);
                        mla_string_destroy(base64Part);

                    } else {
                        // This is a string value
                        instance.current_token.type = MLA_DESERIALIZER_VALUE_STRING;
                        instance.current_token.complex.string_value = str_data;
                    }

                    // Put the char back into buffer for next usage
                    mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, new_char);
                    return true;
                }
            }

            case 't':
                // Check if the remaining String plus the 't' matches "true"
                if (__mla_json_deserializer_string_match(instance, "rue", 3)) {
                    instance.current_token.type = MLA_DESERIALIZER_VALUE_BOOL;
                    instance.current_token.simple.bool_value = true;
                    return true;
                }
                mla_error("Expected 'true'")
                return false;

            case 'f':
                // Check if the remaining String plus the 'f' matches "false"
                if (__mla_json_deserializer_string_match(instance, "alse", 4)) {
                    instance.current_token.type = MLA_DESERIALIZER_VALUE_BOOL;
                    instance.current_token.simple.bool_value = false;
                    return true;
                }
                mla_error("Expected 'false'")
                return false;

            case 'n':
                // Check if the remaining String plus the 'n' matches "null"
                if (__mla_json_deserializer_string_match(instance, "ull", 3)) {
                    instance.current_token.type = MLA_DESERIALIZER_NULL;
                    return true;
                }
                mla_error("Expected 'null'")
                return false;

            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                // Number detected, read the full number
                mla_user_data_set_char(instance.user_data, mla_json_deserializer_char_buffer_user_data_name, new_char);
                return __mla_json_deserializer_read_number_data(instance);

            case ',':
                // Skip commas and continue to next token
                continue;

            default:
                mla_error(mla_string_concat(mla_string_const("Unexpected character in JSON input"), &new_char));
                return false;
        }
    }
}

mla_deserializer_t mla_json_deserializer(const mla_stream_input_t &input) {
    return {
        input,
        mla_user_data_empty(),
        {MLA_DESERIALIZER_NULL, {mla_string_empty(), mla_string_empty(), mla_bytes_empty()}, {0}},
        mla_json_deserializer_read_read_next
    };
}
