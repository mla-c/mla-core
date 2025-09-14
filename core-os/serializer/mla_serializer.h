//
// Basic serialization interface for structured data.
//
// Created by chris on 9/14/2025.
//

#ifndef COREOS_MLA_SERIALIZER_H
#define COREOS_MLA_SERIALIZER_H

#include "../system/mla_string.h"
#include "../system/mla_stream.h"

struct mla_serializer_t {

    const mla_stream_output_t output;

    void (*const start_group)(const mla_serializer_t& instance, const mla_string_t& name);
    void (*const end_group)(const mla_serializer_t& instance);
    void (*const write_bool)(const mla_serializer_t& instance, const mla_string_t& name, const mla_bool_t value);

    void (*const write_int8)(const mla_serializer_t& instance, const mla_string_t& name, const mla_int8_t value);
    void (*const write_int16)(const mla_serializer_t& instance, const mla_string_t& name, const mla_int16_t value);
    void (*const write_int32)(const mla_serializer_t& instance, const mla_string_t& name, const mla_int32_t value);
    void (*const write_int64)(const mla_serializer_t& instance, const mla_string_t& name, const mla_int64_t value);
    void (*const write_uint8)(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint8_t value);
    void (*const write_uint16)(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint16_t value);
    void (*const write_uint32)(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint32_t value);
    void (*const write_uint64)(const mla_serializer_t& instance, const mla_string_t& name, const mla_uint64_t value);

    void (*const write_float)(const mla_serializer_t& instance, const mla_string_t& name, const mla_float_t value);
    void (*const write_double)(const mla_serializer_t& instance, const mla_string_t& name, const mla_double_t value);

    void (*const write_string)(const mla_serializer_t& instance, const mla_string_t& name, const mla_string_t& value);
    void (*const write_bytes)(const mla_serializer_t& instance, const mla_string_t& name, const mla_pointer_t bytes, const mla_size_t size);

};

enum mla_deserializer_token {
    MLA_DESERIALIZER_END,
    MLA_DESERIALIZER_GROUP_START,
    MLA_DESERIALIZER_GROUP_END,
    MLA_DESERIALIZER_VALUE
};

struct mla_deserializer_t {

    const mla_stream_input_t input;

    mla_deserializer_token (*const next)(const mla_deserializer_t& instance);
    mla_string_t (*const current_name)(const mla_deserializer_t& instance);

    mla_bool_t (*const read_bool)(const mla_deserializer_t& instance, const mla_string_t& name);

    mla_int8_t (*const read_int8)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_int16_t (*const read_int16)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_int32_t (*const read_int32)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_int64_t (*const read_int64)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_uint8_t (*const read_uint8)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_uint16_t (*const read_uint16)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_uint32_t (*const read_uint32)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_uint64_t (*const read_uint64)(const mla_deserializer_t& instance, const mla_string_t& name);

    mla_float_t (*const read_float)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_double_t (*const read_double)(const mla_deserializer_t& instance, const mla_string_t& name);

    mla_string_t (*const read_string)(const mla_deserializer_t& instance, const mla_string_t& name);
    mla_size_t (*const read_bytes)(const mla_deserializer_t& instance, const mla_string_t& name, mla_pointer_t buffer, const mla_size_t buffer_size);

};

///////////////////////////////////////////////////////////////////////////////
/// Macros for easier usage
//////////////////////////////////////////////////////////////////////////////

// Deserializer
#define mla_deserializer_next(instance) instance.next(instance)
#define mla_deserializer_current_name(instance) instance.current_name(instance)
#define mla_deserializer_read_bool(instance, name) instance.read_bool(instance, name)
#define mla_deserializer_read_int8(instance, name) instance.read_int8(instance, name)
#define mla_deserializer_read_int16(instance, name) instance.read_int16(instance, name)
#define mla_deserializer_read_int32(instance, name) instance.read_int32(instance, name)
#define mla_deserializer_read_int64(instance, name) instance.read_int64(instance, name)
#define mla_deserializer_read_uint8(instance, name) instance.read_uint8(instance, name)
#define mla_deserializer_read_uint16(instance, name) instance.read_uint16(instance, name)
#define mla_deserializer_read_uint32(instance, name) instance.read_uint32(instance, name)
#define mla_deserializer_read_uint64(instance, name) instance.read_uint64(instance, name)
#define mla_deserializer_read_float(instance, name) instance.read_float(instance, name)
#define mla_deserializer_read_double(instance, name) instance.read_double(instance, name)
#define mla_deserializer_read_string(instance, name) instance.read_string(instance, name)
#define mla_deserializer_read_bytes(instance, name, buffer, buffer_size) instance.read_bytes(instance, name, buffer, buffer_size)

// Serializer
#define mla_serializer_start_group(instance, name) instance.start_group(instance, name)
#define mla_serializer_end_group(instance) instance.end_group(instance)
#define mla_serializer_write_bool(instance, name, value) instance.write_bool(instance, name, value)
#define mla_serializer_write_int8(instance, name, value) instance.write_int8(instance, name, value)
#define mla_serializer_write_int16(instance, name, value) instance.write_int16(instance, name, value)
#define mla_serializer_write_int32(instance, name, value) instance.write_int32(instance, name, value)
#define mla_serializer_write_int64(instance, name, value) instance.write_int64(instance, name, value)
#define mla_serializer_write_uint8(instance, name, value) instance.write_uint8(instance, name, value)
#define mla_serializer_write_uint16(instance, name, value) instance.write_uint16(instance, name, value)
#define mla_serializer_write_uint32(instance, name, value) instance.write_uint32(instance, name, value)
#define mla_serializer_write_uint64(instance, name, value) instance.write_uint64(instance, name, value)
#define mla_serializer_write_float(instance, name, value) instance.write_float(instance, name, value)
#define mla_serializer_write_double(instance, name, value) instance.write_double(instance, name, value)
#define mla_serializer_write_string(instance, name, value) instance.write_string(instance, name, value)
#define mla_serializer_write_bytes(instance, name, bytes, size) instance.write_bytes(instance, name, bytes, size)


#endif