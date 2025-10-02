//
// Binary Serializer/Deserializer which use a simple binary format with little-endian encoding.
//
// Created by chris on 9/15/2025.
//

#include "mla_binary_serializer.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define MLA_LITTLE_ENDIAN 1
    #define MLA_BIG_ENDIAN 0
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define MLA_LITTLE_ENDIAN 0
    #define MLA_BIG_ENDIAN 1
#else
    // Fallback to runtime detection
    static const union { mla_uint16_t i; mla_uint8_t c[2]; } endian_test = {1};
#define MLA_LITTLE_ENDIAN (endian_test.c[0] == 1)
#define MLA_BIG_ENDIAN (!MLA_LITTLE_ENDIAN)
#endif

// Byte-swapping functions
inline mla_uint16_t swap_uint16(mla_uint16_t val) {
    return (val << 8) | (val >> 8);
}

inline mla_uint32_t swap_uint32(mla_uint32_t val) {
    return ((val << 24) | ((val << 8) & 0x00FF0000) |
           ((val >> 8) & 0x0000FF00) | (val >> 24));
}

inline mla_uint64_t swap_uint64(mla_uint64_t val) {
    return ((val << 56) |
           ((val << 40) & 0x00FF000000000000ULL) |
           ((val << 24) & 0x0000FF0000000000ULL) |
           ((val << 8)  & 0x000000FF00000000ULL) |
           ((val >> 8)  & 0x00000000FF000000ULL) |
           ((val >> 24) & 0x0000000000FF0000ULL) |
           ((val >> 40) & 0x000000000000FF00ULL) |
           (val >> 56));
}

// Conversion functions for host to little endian
inline mla_int16_t host_to_le_int16(mla_int16_t val) {
    return MLA_LITTLE_ENDIAN ? val : swap_uint16(val);
}

inline mla_int32_t host_to_le_int32(mla_int32_t val) {
    return MLA_LITTLE_ENDIAN ? val : swap_uint32(val);
}

inline mla_int64_t host_to_le_int64(mla_int64_t val) {
    return MLA_LITTLE_ENDIAN ? val : swap_uint64(val);
}

inline mla_uint16_t host_to_le_uint16(mla_uint16_t val) {
    return MLA_LITTLE_ENDIAN ? val : swap_uint16(val);
}

inline mla_uint32_t host_to_le_uint32(mla_uint32_t val) {
    return MLA_LITTLE_ENDIAN ? val : swap_uint32(val);
}

inline mla_uint64_t host_to_le_uint64(mla_uint64_t val) {
    return MLA_LITTLE_ENDIAN ? val : swap_uint64(val);
}

// For floating-point types
inline mla_float_t host_to_le_float(mla_float_t val) {
    union { mla_float_t f; mla_uint32_t i; } u;
    u.f = val;
    u.i = host_to_le_uint32(u.i);
    return u.f;
}

inline mla_double_t host_to_le_double(mla_double_t val) {
    union { mla_double_t d; mla_uint64_t i; } u;
    u.d = val;
    u.i = host_to_le_uint64(u.i);
    return u.d;
}

// Little endian to host (same functions, just with more descriptive names)
#define le_to_host_int16 host_to_le_int16
#define le_to_host_int32 host_to_le_int32
#define le_to_host_int64 host_to_le_int64
#define le_to_host_uint16 host_to_le_uint16
#define le_to_host_uint32 host_to_le_uint32
#define le_to_host_uint64 host_to_le_uint64
#define le_to_host_float host_to_le_float
#define le_to_host_double host_to_le_double


void __mla_binary_serializer_write_element_type(const mla_stream_output_t& output, const mla_deserializer_token_type_t type) {

    mla_uint8_t byteType = (mla_uint8_t)type;
    output.write(output, 0, sizeof(byteType), reinterpret_cast<const mla_byte_t*>(&byteType));
}

void __mla_binary_serializer_write_string_data(const mla_stream_output_t& output, const mla_string_t& name) {

    // Write the length of the string as a 32-bit integer in little endian
    mla_size_t len_le = host_to_le_uint32(name.length);
    output.write(output, 0, sizeof(mla_size_t), reinterpret_cast<const mla_byte_t*>(&len_le));


    if (name.length > 0) {
        output.write(output, 0, name.length, reinterpret_cast<const mla_byte_t*>(name.data));
    }

}

void mla_binary_serializer_write_start_struct(mla_serializer_t& instance) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_STRUCT_START);

}

void mla_binary_serializer_write_end_struct(mla_serializer_t& instance) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_STRUCT_END);
}

void mla_binary_serializer_write_start_list(mla_serializer_t& instance) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_LIST_START);

}

void mla_binary_serializer_write_end_list(mla_serializer_t& instance) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_LIST_END);
}

void mla_binary_serializer_write_property_name(mla_serializer_t& instance, const mla_string_t& name) {
    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_PROPERTY_NAME);
    __mla_binary_serializer_write_string_data(instance.output, name);
}

void mla_binary_serializer_write_bool(mla_serializer_t& instance, const mla_bool_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_BOOL);
    instance.output.write(instance.output, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));
}

void mla_binary_serializer_write_int8(mla_serializer_t& instance, const mla_int8_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT8);
    instance.output.write(instance.output, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));

}

void mla_binary_serializer_write_int16(mla_serializer_t& instance, const mla_int16_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT16);

    mla_int16_t le_value = host_to_le_int16(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void mla_binary_serializer_write_int32(mla_serializer_t& instance, const mla_int32_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT32);

    mla_int32_t le_value = host_to_le_int32(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void mla_binary_serializer_write_int64(mla_serializer_t& instance, const mla_int64_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_INT64);

    mla_int64_t le_value = host_to_le_int64(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void mla_binary_serializer_write_uint8(mla_serializer_t& instance, const mla_uint8_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT8);
    instance.output.write(instance.output, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));

}

void mla_binary_serializer_write_uint16(mla_serializer_t& instance, const mla_uint16_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT16);

    mla_uint16_t le_value = host_to_le_uint16(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void mla_binary_serializer_write_uint32(mla_serializer_t& instance, const mla_uint32_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT32);

    mla_uint32_t le_value = host_to_le_uint32(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void mla_binary_serializer_write_uint64(mla_serializer_t& instance, const mla_uint64_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_UINT64);

    mla_uint64_t le_value = host_to_le_uint64(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void mla_binary_serializer_write_float(mla_serializer_t& instance, const mla_float_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_FLOAT);

    mla_float_t le_value = host_to_le_float(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void mla_binary_serializer_write_double(mla_serializer_t& instance, const mla_double_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_DOUBLE);

    mla_double_t le_value = host_to_le_double(value);
    instance.output.write(instance.output, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void mla_binary_serializer_write_string(mla_serializer_t& instance, const mla_string_t& value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_STRING);
    __mla_binary_serializer_write_string_data(instance.output, value);
}

void mla_binary_serializer_write_bytes(mla_serializer_t& instance, const mla_bytes_t& bytes) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_DESERIALIZER_VALUE_BYTES);

    mla_size_t size_le = host_to_le_uint32(bytes.size);
    instance.output.write(instance.output, 0, sizeof(size_le), reinterpret_cast<const mla_byte_t*>(&size_le));

    if (bytes.size > 0) {
        instance.output.write(instance.output, 0, bytes.size, bytes.data);
    }
}

mla_serializer_t mla_binary_serializer(const mla_stream_output_t& output) {

    return {
        output,
        0,
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


mla_string_t __mla_binary_deserializer_read_string_data(mla_deserializer_t& instance) {

    // Read the length of the string in little endian
    mla_uint32_t len_le = 0;
    instance.input.read(instance.input, 0, sizeof(len_le), reinterpret_cast<mla_byte_t*>(&len_le));
    mla_size_t length = le_to_host_uint32(len_le);

    if (length == 0) {
        return mla_string_empty();
    }

    mla_char_t* charBuffer = mla_create_char_array(length);

    if (charBuffer == nullptr) {
        // No memory available
        return mla_string_empty();
    }

    mla_size_t readed_bytes = instance.input.read(instance.input, 0, length, reinterpret_cast<mla_byte_t*>(charBuffer));

    if (readed_bytes != length) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(readed_bytes), ") do not match expected length (", mla_string_from_uint32(length), ") in deserializer"));
    }

    return  mla_string_from_buffer_with_ownership(charBuffer, readed_bytes);

}

mla_bool_t __mla_binary_deserializer_read_bool(mla_deserializer_t& instance) {

    mla_bool_t value = 0;

    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));

    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return false; // Default to false on error
    }

    return value;


}

mla_int8_t __mla_binary_deserializer_read_int8(mla_deserializer_t& instance) {

    mla_int8_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));

    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return value;

}

mla_int16_t __mla_binary_deserializer_read_int16(mla_deserializer_t& instance) {

    mla_int16_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_int16(value);
}

mla_int32_t __mla_binary_deserializer_read_int32(mla_deserializer_t& instance) {

    mla_int32_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_int32(value);

}

mla_int64_t __mla_binary_deserializer_read_int64(mla_deserializer_t& instance) {

    mla_int64_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_int64(value);
}

mla_uint8_t __mla_binary_deserializer_read_uint8(mla_deserializer_t& instance) {

    mla_uint8_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return value;
}

mla_uint16_t __mla_binary_deserializer_read_uint16(mla_deserializer_t& instance) {

    mla_uint16_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_uint16(value);

}

mla_uint32_t __mla_binary_deserializer_read_uint32(mla_deserializer_t& instance) {

    mla_uint32_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_uint32(value);
}

mla_uint64_t __mla_binary_deserializer_read_uint64(mla_deserializer_t& instance) {

    mla_uint64_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_uint64(value);

}

mla_float_t __mla_binary_deserializer_read_float(mla_deserializer_t& instance) {

    mla_float_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }

    return le_to_host_float(value);

}

mla_double_t __mla_binary_deserializer_read_double(mla_deserializer_t& instance) {

    mla_double_t value = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(value), reinterpret_cast<mla_byte_t*>(&value));
    if (read_bytes != sizeof(value)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(value)), ") in deserializer"));
        return 0; // Default to 0 on error
    }
    return le_to_host_double(value);

}

mla_string_t __mla_binary_deserializer_read_string(mla_deserializer_t& instance) {

    return __mla_binary_deserializer_read_string_data(instance);

}

mla_bytes_t __mla_binary_deserializer_read_bytes(mla_deserializer_t& instance) {

    mla_size_t read_bytes = 0;
    mla_uint32_t size_le = 0;
    instance.input.read(instance.input, 0, sizeof(size_le), reinterpret_cast<mla_byte_t*>(&size_le));
    mla_size_t size = le_to_host_uint32(size_le);

    if (size == 0) {
        return mla_bytes_empty();
    }

    mla_byte_t* bytesBuffer = reinterpret_cast<mla_byte_t*>(mla_malloc(sizeof(mla_byte_t) * size));

    if (bytesBuffer == nullptr) {
        return mla_bytes_empty();
    }

    read_bytes = instance.input.read(instance.input, 0, size, bytesBuffer);

    if (read_bytes != size) {
        mla_free(bytesBuffer);
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(size), ") in deserializer"));
        return mla_bytes_empty();
    }

    return mla_bytes_from_buffer_with_ownership(bytesBuffer, read_bytes);

}


mla_bool_t mla_binary_deserializer_read_read_next(mla_deserializer_t& instance) {

    mla_uint8_t type = 0;
    mla_size_t read_bytes = instance.input.read(instance.input, 0, sizeof(type), reinterpret_cast<mla_byte_t*>(&type));

    if (read_bytes == 0) {
        return  false;
    }

    if (read_bytes != sizeof(type)) {
        mla_warning(mla_string_concat("Readed bytes (", mla_string_from_uint32(read_bytes), ") do not match expected length (", mla_string_from_uint32(sizeof(type)), ") in deserializer"));
        return false;
    }

    instance.current_token.type = static_cast<mla_deserializer_token_type_t>(type);

    // Reset current token data
    instance.current_token.complex = {
        mla_string_empty(),
        mla_string_empty(),
        mla_bytes_empty()
    };
    instance.current_token.simple = {0};

    switch (instance.current_token.type) {
        case MLA_DESERIALIZER_NULL:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_PROPERTY_NAME:
            instance.current_token.complex.property_name = __mla_binary_deserializer_read_string_data(instance);
            break;
        case MLA_DESERIALIZER_STRUCT_START:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_STRUCT_END:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_LIST_START:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_LIST_END:
            // Nothing to do
            break;
        case MLA_DESERIALIZER_VALUE_BOOL:
            instance.current_token.simple.bool_value = __mla_binary_deserializer_read_bool(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT8:
            instance.current_token.simple.int8_value = __mla_binary_deserializer_read_int8(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT16:
            instance.current_token.simple.int16_value = __mla_binary_deserializer_read_int16(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT32:
            instance.current_token.simple.int32_value = __mla_binary_deserializer_read_int32(instance);
            break;
        case MLA_DESERIALIZER_VALUE_INT64:
            instance.current_token.simple.int64_value = __mla_binary_deserializer_read_int64(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT8:
            instance.current_token.simple.uint8_value = __mla_binary_deserializer_read_uint8(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT16:
            instance.current_token.simple.uint16_value = __mla_binary_deserializer_read_uint16(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT32:
            instance.current_token.simple.uint32_value = __mla_binary_deserializer_read_uint32(instance);
            break;
        case MLA_DESERIALIZER_VALUE_UINT64:
            instance.current_token.simple.uint64_value = __mla_binary_deserializer_read_uint64(instance);
            break;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            instance.current_token.simple.float_value = __mla_binary_deserializer_read_float(instance);
            break;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            instance.current_token.simple.double_value = __mla_binary_deserializer_read_double(instance);
            break;
        case MLA_DESERIALIZER_VALUE_STRING:
            instance.current_token.complex.string_value = __mla_binary_deserializer_read_string(instance);
            break;
        case MLA_DESERIALIZER_VALUE_BYTES:
            instance.current_token.complex.bytes_value = __mla_binary_deserializer_read_bytes(instance);
            break;
    }

    return true;
}

mla_deserializer_t mla_binary_deserializer(const mla_stream_input_t& input) {

    return {
        input,
        0,
        {MLA_DESERIALIZER_NULL, {mla_string_empty(), mla_string_empty(), mla_bytes_empty()}, {0}},
        mla_binary_deserializer_read_read_next
    };
}