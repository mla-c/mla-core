//
// Binary Serializer/Deserializer which use a simple binary format with little-endian encoding.
//
// Created by chris on 9/15/2025.
//

#include "mla_binary_serializer.h"

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



enum mla_binary_element_type {
    MLA_BINARY_ELEMENT_TYPE_GROUP_START = 1,
    MLA_BINARY_ELEMENT_TYPE_GROUP_END = 2,
    MLA_BINARY_ELEMENT_TYPE_BOOL = 3,
    MLA_BINARY_ELEMENT_TYPE_INT8 = 4,
    MLA_BINARY_ELEMENT_TYPE_INT16 = 5,
    MLA_BINARY_ELEMENT_TYPE_INT32 = 6,
    MLA_BINARY_ELEMENT_TYPE_INT64 = 7,
    MLA_BINARY_ELEMENT_TYPE_UINT8 = 8,
    MLA_BINARY_ELEMENT_TYPE_UINT16 = 9,
    MLA_BINARY_ELEMENT_TYPE_UINT32 = 10,
    MLA_BINARY_ELEMENT_TYPE_UINT64 = 11,
    MLA_BINARY_ELEMENT_TYPE_FLOAT = 12,
    MLA_BINARY_ELEMENT_TYPE_DOUBLE = 13,
    MLA_BINARY_ELEMENT_TYPE_STRING = 14,
    MLA_BINARY_ELEMENT_TYPE_BYTES = 15
};

void __mla_binary_serializer_write_element_type(const mla_stream_output_t& output, const mla_binary_element_type type) {

    output.write(output.userdata, 0, sizeof(type), reinterpret_cast<const mla_byte_t*>(&type));
}

void __mla_binary_serializer_write_string_data(const mla_stream_output_t& output, const mla_string_t& name) {

    // Write the length of the string as a 32-bit integer in little endian
    mla_size_t len_le = host_to_le_uint32(name.length);
    output.write(output.userdata, 0, sizeof(mla_size_t), reinterpret_cast<const mla_byte_t*>(&len_le));


    if (name.length > 0) {
        output.write(output.userdata, 0, name.length, reinterpret_cast<const mla_byte_t*>(name.data));
    }

}

void __mla_binary_serializer_start_group(const mla_serializer_t& instance, const mla_string_t& name) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_GROUP_START);
    __mla_binary_serializer_write_string_data(instance.output, name);

}

void  __mla_binary_serializer_end_group(const mla_serializer_t& instance) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_GROUP_END);
}

void __mla_binary_serializer_write_bool(const mla_serializer_t& instance, const mla_string_t& name, const mla_bool_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_BOOL);
    __mla_binary_serializer_write_string_data(instance.output, name);
    instance.output.write(instance.output.userdata, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));
}

void __mla_binary_serializer_write_int8(const mla_serializer_t& instance, const mla_string_t& name, const mla_int8_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_INT8);
    __mla_binary_serializer_write_string_data(instance.output, name);
    instance.output.write(instance.output.userdata, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));

}

void __mla_binary_serializer_write_int16(const mla_serializer_t& instance, const mla_string_t& name, const mla_int16_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_INT16);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_int16_t le_value = host_to_le_int16(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void __mla_binary_serializer_write_int32(const mla_serializer_t& instance, const mla_string_t& name, const mla_int32_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_INT32);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_int32_t le_value = host_to_le_int32(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void __mla_binary_serializer_write_int64(const mla_serializer_t& instance, const mla_string_t& name, const mla_int64_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_INT64);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_int64_t le_value = host_to_le_int64(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void __mla_binary_serializer_write_uint8(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint8_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_UINT8);
    __mla_binary_serializer_write_string_data(instance.output, name);
    instance.output.write(instance.output.userdata, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));

}

void __mla_binary_serializer_write_uint16(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint16_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_UINT16);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_uint16_t le_value = host_to_le_uint16(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void __mla_binary_serializer_write_uint32(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint32_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_UINT32);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_uint32_t le_value = host_to_le_uint32(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void __mla_binary_serializer_write_uint64(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint64_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_UINT64);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_uint64_t le_value = host_to_le_uint64(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));

}

void __mla_binary_serializer_write_float(const mla_serializer_t& instance, const mla_string_t& name, const mla_float_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_FLOAT);
    __mla_binary_serializer_write_string_data(instance.output, name);
    instance.output.write(instance.output.userdata, 0, sizeof(value), reinterpret_cast<const mla_byte_t*>(&value));

    mla_float_t le_value = host_to_le_float(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void __mla_binary_serializer_write_double(const mla_serializer_t& instance, const mla_string_t& name, const mla_double_t value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_DOUBLE);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_double_t le_value = host_to_le_double(value);
    instance.output.write(instance.output.userdata, 0, sizeof(le_value), reinterpret_cast<const mla_byte_t*>(&le_value));
}

void __mla_binary_serializer_write_string(const mla_serializer_t& instance, const mla_string_t& name, const mla_string_t& value) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_STRING);
    __mla_binary_serializer_write_string_data(instance.output, name);
    __mla_binary_serializer_write_string_data(instance.output, value);
}

void __mla_binary_serializer_write_bytes(const mla_serializer_t& instance, const mla_string_t& name, const mla_pointer_t bytes, const mla_size_t size) {

    __mla_binary_serializer_write_element_type(instance.output, MLA_BINARY_ELEMENT_TYPE_BYTES);
    __mla_binary_serializer_write_string_data(instance.output, name);

    mla_size_t size_le = host_to_le_uint32(size);
    instance.output.write(instance.output.userdata, 0, sizeof(size_le), reinterpret_cast<const mla_byte_t*>(&size_le));

    if (size > 0) {
        instance.output.write(instance.output.userdata, 0, size, reinterpret_cast<const mla_byte_t*>(bytes));
    }
}

mla_serializer_t mla_binary_serializer(const mla_stream_output_t& output) {

    return {
        output,
        __mla_binary_serializer_start_group,
        __mla_binary_serializer_end_group,
        __mla_binary_serializer_write_bool,
        __mla_binary_serializer_write_int8,
        __mla_binary_serializer_write_int16,
        __mla_binary_serializer_write_int32,
        __mla_binary_serializer_write_int64,
        __mla_binary_serializer_write_uint8,
        __mla_binary_serializer_write_uint16,
        __mla_binary_serializer_write_uint32,
        __mla_binary_serializer_write_uint64,
        __mla_binary_serializer_write_float,
        __mla_binary_serializer_write_double,
        __mla_binary_serializer_write_string,
        __mla_binary_serializer_write_bytes
    };
}

mla_deserializer_token __mla_binary_deserializer_next(const mla_deserializer_t& instance) {

}

mla_string_t __mla_binary_deserializer_current_name(const mla_deserializer_t& instance) {

}

mla_bool_t __mla_binary_deserializer_read_bool(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_int8_t __mla_binary_deserializer_read_int8(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_int16_t __mla_binary_deserializer_read_int16(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_int32_t __mla_binary_deserializer_read_int32(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_int64_t __mla_binary_deserializer_read_int64(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_uint8_t __mla_binary_deserializer_read_uint8(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_uint16_t __mla_binary_deserializer_read_uint16(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_uint32_t __mla_binary_deserializer_read_uint32(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_uint64_t __mla_binary_deserializer_read_uint64(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_float_t __mla_binary_deserializer_read_float(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_double_t __mla_binary_deserializer_read_double(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_string_t __mla_binary_deserializer_read_string(const mla_deserializer_t& instance, const mla_string_t& name) {

}

mla_size_t __mla_binary_deserializer_read_bytes(const mla_deserializer_t& instance, const mla_string_t& name, mla_pointer_t buffer, const mla_size_t buffer_size) {

}


mla_deserializer_t mla_binary_deserializer(const mla_stream_input_t& input) {

    return {
        input,
        __mla_binary_deserializer_next,
        __mla_binary_deserializer_current_name,
        __mla_binary_deserializer_read_bool,
        __mla_binary_deserializer_read_int8,
        __mla_binary_deserializer_read_int16,
        __mla_binary_deserializer_read_int32,
        __mla_binary_deserializer_read_int64,
        __mla_binary_deserializer_read_uint8,
        __mla_binary_deserializer_read_uint16,
        __mla_binary_deserializer_read_uint32,
        __mla_binary_deserializer_read_uint64,
        __mla_binary_deserializer_read_float,
        __mla_binary_deserializer_read_double,
        __mla_binary_deserializer_read_string,
        __mla_binary_deserializer_read_bytes
    };
}