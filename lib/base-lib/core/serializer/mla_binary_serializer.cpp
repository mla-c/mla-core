//
// Binary Serializer/Deserializer which use a simple binary format with little-endian encoding.
//
// Created by chris on 9/15/2025.
//

#include "mla_binary_serializer.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../utils/mla_endian_utils.h"


mla_bool_t mla_private_binary_serializer_write_element_type(mla_stream_output_t& output, const mla_deserializer_token_type_t type) {

    mla_uint8_t byteType = type;
    return output.write(output, 0, sizeof(byteType), mla_r_cast<const mla_byte_t*>(&byteType)) == sizeof(byteType);
}

mla_bool_t mla_private_binary_serializer_write_string_data(mla_stream_output_t& output, const mla_string_t& name) {

    mla_size_t length = mla_string_length(name);

    // Write the length of the string as a 32-bit integer in little endian
    mla_size_t len_le = mla_host_to_le_uint32(length);

    if (output.write(output, 0, sizeof(mla_size_t), mla_r_cast<const mla_byte_t*>(&len_le)) != sizeof(mla_size_t)) {
        return false;
    }

    if (length > 0) {

        const mla_char_t* data = mla_string_data(name);

        if (output.write(output, 0, length, mla_r_cast<const mla_byte_t*>(data)) != length) {
            return false;
        }
    }

    return true;

}

mla_bool_t mla_binary_serializer_write_start_struct(mla_serializer_t& instance) {

    return mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_STRUCT_START);

}

mla_bool_t mla_binary_serializer_write_end_struct(mla_serializer_t& instance) {

    return mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_STRUCT_END);
}

mla_bool_t mla_binary_serializer_write_start_list(mla_serializer_t& instance) {

    return mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_LIST_START);

}

mla_bool_t mla_binary_serializer_write_end_list(mla_serializer_t& instance) {

    return mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_LIST_END);
}

mla_bool_t mla_binary_serializer_write_property_name(mla_serializer_t& instance, const mla_string_t& name) {
    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_PROPERTY_NAME)) {
        return false;
    }
    if (!mla_private_binary_serializer_write_string_data(instance.output, name)) {
        return false;
    }
    return true;
}

mla_bool_t mla_binary_serializer_write_bool(mla_serializer_t& instance, const mla_bool_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_BOOL)) {
        return false;
    }
    return instance.output.write(instance.output, 0, sizeof(value), mla_r_cast<const mla_byte_t*>(&value)) == sizeof(value);
}

mla_bool_t mla_binary_serializer_write_int8(mla_serializer_t& instance, const mla_int8_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT8)) {
        return false;
    }
    return instance.output.write(instance.output, 0, sizeof(value), mla_r_cast<const mla_byte_t*>(&value)) == sizeof(value);

}

mla_bool_t mla_binary_serializer_write_int16(mla_serializer_t& instance, const mla_int16_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT16)) {
        return false;
    }

    mla_int16_t le_value = mla_host_to_le_int16(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);
}

mla_bool_t mla_binary_serializer_write_int32(mla_serializer_t& instance, const mla_int32_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT32)) {
        return false;
    }

    mla_int32_t le_value = mla_host_to_le_int32(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);
}

mla_bool_t mla_binary_serializer_write_int64(mla_serializer_t& instance, const mla_int64_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT64)) {
        return false;
    }

    mla_int64_t le_value = mla_host_to_le_int64(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);

}

mla_bool_t mla_binary_serializer_write_uint8(mla_serializer_t& instance, const mla_uint8_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT8)) {
        return false;
    }
    return instance.output.write(instance.output, 0, sizeof(value), mla_r_cast<const mla_byte_t*>(&value)) == sizeof(value);

}

mla_bool_t mla_binary_serializer_write_uint16(mla_serializer_t& instance, const mla_uint16_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT16)) {
        return false;
    }

    mla_uint16_t le_value = mla_host_to_le_uint16(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);

}

mla_bool_t mla_binary_serializer_write_uint32(mla_serializer_t& instance, const mla_uint32_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT32)) {
        return false;
    }

    mla_uint32_t le_value = mla_host_to_le_uint32(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);

}

mla_bool_t mla_binary_serializer_write_uint64(mla_serializer_t& instance, const mla_uint64_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT64)) {
        return false;
    }

    mla_uint64_t le_value = mla_host_to_le_uint64(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);

}

mla_bool_t mla_binary_serializer_write_float(mla_serializer_t& instance, const mla_float_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_FLOAT)) {
        return false;
    }

    mla_float_t le_value = mla_host_to_le_float(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);
}

mla_bool_t mla_binary_serializer_write_double(mla_serializer_t& instance, const mla_double_t value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_DOUBLE)) {
        return false;
    }

    mla_double_t le_value = mla_host_to_le_double(value);
    return instance.output.write(instance.output, 0, sizeof(le_value), mla_r_cast<const mla_byte_t*>(&le_value)) == sizeof(le_value);
}

mla_bool_t mla_binary_serializer_write_string(mla_serializer_t& instance, const mla_string_t& value) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_STRING)) {
        return false;
    }
    return mla_private_binary_serializer_write_string_data(instance.output, value);
}

mla_bool_t mla_binary_serializer_write_bytes(mla_serializer_t& instance, const mla_bytes_t& bytes) {

    if (!mla_private_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_BYTES)) {
        return false;
    }

    mla_size_t size_le = mla_host_to_le_uint32(bytes.size);
    if (instance.output.write(instance.output, 0, sizeof(size_le), mla_r_cast<const mla_byte_t*>(&size_le)) != sizeof(size_le)) {
        return false;
    }

    const mla_byte_t* bytes_buffer = mla_bytes_get_data_readonly(bytes);

    if (bytes.size > 0 && bytes_buffer != nullptr) {
        return instance.output.write(instance.output, 0, bytes.size, bytes_buffer) == bytes.size;
    }

    return true;
}

mla_serializer_t mla_binary_serializer(const mla_stream_output_t& output) {

    return {
        output,
        mla_user_data_empty(),
        mla_binary_serializer_write_start_struct,
        mla_binary_serializer_write_end_struct,
        mla_binary_serializer_write_start_list,
        mla_binary_serializer_write_end_list,
        mla_binary_serializer_write_property_name,
        mla_binary_serializer_write_bool,
        mla_binary_serializer_write_int8,
        mla_binary_serializer_write_int16,
        mla_binary_serializer_write_int32,
        mla_binary_serializer_write_int64,
        mla_binary_serializer_write_uint8,
        mla_binary_serializer_write_uint16,
        mla_binary_serializer_write_uint32,
        mla_binary_serializer_write_uint64,
        mla_binary_serializer_write_float,
        mla_binary_serializer_write_double,
        mla_binary_serializer_write_string,
        mla_binary_serializer_write_bytes
    };
}


mla_string_t mla_private_binary_deserializer_read_string_data(mla_deserializer_t& instance) {

    // Read the length of the string in little endian
    mla_uint32_t len_le = 0;
    instance.input.read(instance.input, 0, sizeof(len_le), mla_r_cast<mla_byte_t*>(&len_le));
    mla_size_t length = mla_le_to_host_uint32(len_le);

    if (length == 0) {
        return mla_string_empty();
    }

    mla_pointer_t charBuffer = mla_create_char_array(length);
    mla_char_t* char_data = mla_pointer_get_data<mla_char_t>(charBuffer);

    if (char_data == nullptr) {
        return mla_string_empty();
    }

    mla_size_t readed_bytes = instance.input.read(instance.input, 0, length, mla_r_cast<mla_byte_t*>(char_data));

    if (readed_bytes != length) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(readed_bytes), ") do not match expected length (", mla_string_from_uint32(length), ") in deserializer"));
    }

    return  mla_string(charBuffer, readed_bytes);

}

mla_bool_t mla_private_binary_deserializer_read_bool(mla_deserializer_t& instance) {

    mla_bool_t value = false;

    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));

    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return false; // Default to false on error
    }

    return value;


}

mla_int8_t mla_private_binary_deserializer_read_int8(mla_deserializer_t& instance) {

    mla_int8_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));

    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return value;

}

mla_int16_t mla_private_binary_deserializer_read_int16(mla_deserializer_t& instance) {

    mla_int16_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_int16(value);
}

mla_int32_t mla_private_binary_deserializer_read_int32(mla_deserializer_t& instance) {

    mla_int32_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_int32(value);

}

mla_int64_t mla_private_binary_deserializer_read_int64(mla_deserializer_t& instance) {

    mla_int64_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_int64(value);
}

mla_uint8_t mla_private_binary_deserializer_read_uint8(mla_deserializer_t& instance) {

    mla_uint8_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return value;
}

mla_uint16_t mla_private_binary_deserializer_read_uint16(mla_deserializer_t& instance) {

    mla_uint16_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_uint16(value);

}

mla_uint32_t mla_private_binary_deserializer_read_uint32(mla_deserializer_t& instance) {

    mla_uint32_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_uint32(value);
}

mla_uint64_t mla_private_binary_deserializer_read_uint64(mla_deserializer_t& instance) {

    mla_uint64_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_uint64(value);

}

mla_float_t mla_private_binary_deserializer_read_float(mla_deserializer_t& instance) {

    mla_float_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }

    return mla_le_to_host_float(value);

}

mla_double_t mla_private_binary_deserializer_read_double(mla_deserializer_t& instance) {

    mla_double_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), mla_r_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return mla_le_to_host_double(value);

}

mla_string_t mla_private_binary_deserializer_read_string(mla_deserializer_t& instance) {

    return mla_private_binary_deserializer_read_string_data(instance);

}

mla_bytes_t mla_private_binary_deserializer_read_bytes(mla_deserializer_t& instance) {

    mla_size_t read_bytes = 0;
    mla_uint32_t size_le = 0;
    instance.input.read(instance.input, 0, sizeof(size_le), mla_r_cast<mla_byte_t*>(&size_le));
    mla_size_t size = mla_le_to_host_uint32(size_le);

    if (size == 0) {
        return mla_bytes_empty();
    }

    mla_pointer_t bytesBuffer = mla_malloc_buffer(sizeof(mla_byte_t) * size);
    mla_byte_t* buffer_data = mla_pointer_get_data<mla_byte_t>(bytesBuffer);

    if (buffer_data == nullptr) {
        return mla_bytes_empty();
    }

    read_bytes = instance.input.read(instance.input, 0, size, buffer_data);

    if (read_bytes != size) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(size), ") in deserializer"));
        return mla_bytes_empty();
    }

    return mla_bytes_from_external_buffer(bytesBuffer, read_bytes);

}


mla_bool_t mla_binary_deserializer_read_read_next(mla_deserializer_t& instance) {

    mla_uint8_t type = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(type), mla_r_cast<mla_byte_t*>(&type));

    if (read_bytes == 0) {
        return  false;
    }

    if (read_bytes != sizeof(type)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(type)), ") in deserializer"));
        return false;
    }

    instance.current_token.type = mla_s_cast<mla_deserializer_token_type_t>(type);

    // Reset current token data
    instance.current_token.complex = {
        mla_string_empty(),
        mla_string_empty(),
        mla_bytes_empty()
    };
    instance.current_token.simple = {};

    switch (instance.current_token.type) {
        case MLA_DESERIALIZER_NULL:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_PROPERTY_NAME:
            instance.current_token.complex.property_name = mla_private_binary_deserializer_read_string_data(instance);
            break;
        case MLA_DESERIALIZER_STRUCT_START:
        case MLA_DESERIALIZER_STRUCT_END:
        case MLA_DESERIALIZER_LIST_START:
        case MLA_DESERIALIZER_LIST_END:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_VALUE_BOOL:
            instance.current_token.simple.bool_value = mla_private_binary_deserializer_read_bool(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT8:
            instance.current_token.simple.int8_value = mla_private_binary_deserializer_read_int8(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT16:
            instance.current_token.simple.int16_value = mla_private_binary_deserializer_read_int16(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT32:
            instance.current_token.simple.int32_value = mla_private_binary_deserializer_read_int32(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT64:
            instance.current_token.simple.int64_value = mla_private_binary_deserializer_read_int64(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT8:
            instance.current_token.simple.uint8_value = mla_private_binary_deserializer_read_uint8(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT16:
            instance.current_token.simple.uint16_value = mla_private_binary_deserializer_read_uint16(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT32:
            instance.current_token.simple.uint32_value = mla_private_binary_deserializer_read_uint32(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT64:
            instance.current_token.simple.uint64_value = mla_private_binary_deserializer_read_uint64(instance);
            break;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            instance.current_token.simple.float_value = mla_private_binary_deserializer_read_float(instance);
            break;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            instance.current_token.simple.double_value = mla_private_binary_deserializer_read_double(instance);
            break;
        case MLA_DESERIALIZER_VALUE_STRING:
            instance.current_token.complex.string_value = mla_private_binary_deserializer_read_string(instance);
            break;
        case MLA_DESERIALIZER_VALUE_BYTES:
            instance.current_token.complex.bytes_value = mla_private_binary_deserializer_read_bytes(instance);
            break;
    }

    return true;
}

mla_deserializer_t mla_binary_deserializer(const mla_stream_input_t& input) {

    return {
        input,
        mla_user_data_empty(),
        {MLA_DESERIALIZER_NULL, {mla_string_empty(), mla_string_empty(), mla_bytes_empty()}, {}},
        mla_binary_deserializer_read_read_next
    };
}