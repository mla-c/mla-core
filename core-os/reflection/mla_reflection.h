//
// Created by chris on 12/15/2025.
//

#ifndef COREOS_MLA_REFLECTION_H
#define COREOS_MLA_REFLECTION_H

#include "../lifecycle/mla_lifecycle_events.h"
#include "../system/mla_string.h"

enum mla_reflection_type_t: mla_uint8_t {
    MLA_REFLECTION_TYPE_INVALID = 0,
    MLA_REFLECTION_TYPE_BOOL,

    MLA_REFLECTION_TYPE_INT8,
    MLA_REFLECTION_TYPE_INT16,
    MLA_REFLECTION_TYPE_INT32,
    MLA_REFLECTION_TYPE_INT64,

    MLA_REFLECTION_TYPE_UINT8,
    MLA_REFLECTION_TYPE_UINT16,
    MLA_REFLECTION_TYPE_UINT32,
    MLA_REFLECTION_TYPE_UINT64,

    MLA_REFLECTION_TYPE_FLOAT,
    MLA_REFLECTION_TYPE_DOUBLE,

    MLA_REFLECTION_TYPE_STRING,
    MLA_REFLECTION_TYPE_BYTES,

    MLA_REFLECTION_TYPE_STRUCT,
    MLA_REFLECTION_TYPE_LIST,
};

struct mla_reflection_struct_field_t {
    mla_string_t name;
    mla_size_t offset;
    mla_reflection_type_t type;
    mla_reflection_type_t element_type; // For lists
    mla_string_t struct_name;
};

struct mla_reflection_struct_field_initializer {
    static mla_reflection_struct_field_t init() {
        return { mla_string_empty(), 0, MLA_REFLECTION_TYPE_INVALID, MLA_REFLECTION_TYPE_INVALID, mla_string_empty() };
    }
};

struct mla_reflection_struct_metadata_t {
    mla_string_t name;
    mla_size_t size;
    mla_bool_t frozen;
    mla_array_list_t<mla_reflection_struct_field_t, mla_reflection_struct_field_initializer> fields;
};

mla_reflection_struct_metadata_t mla_reflection_struct_metadata_invalid();
mla_reflection_struct_metadata_t mla_reflection_struct_metadata(const mla_string_t &name, mla_size_t size);
void mla_reflection_struct_metadata_add_field(mla_reflection_struct_metadata_t& metadata, const mla_reflection_struct_field_t &field);
void mla_reflection_struct_metadata_freeze(mla_reflection_struct_metadata_t& metadata);
mla_reflection_struct_field_t mla_reflection_struct_field(const mla_string_t &name, mla_size_t offset, mla_reflection_type_t type, mla_reflection_type_t element_type, const mla_string_t &struct_name);

struct mla_reflection_struct_metadata_initializer {
    static mla_reflection_struct_metadata_t init() {
        return mla_reflection_struct_metadata_invalid();
    }
};

typedef mla_reflection_struct_metadata_t (*mla_reflection_struct_metadata_provider_t)();

mla_bool_t mla_reflection_register_struct(const mla_string_t &name, mla_reflection_struct_metadata_provider_t provider);

template<typename T>
mla_bool_t mla_reflection_register_struct()
{
    mla_reflection_struct_metadata_t metadata = T::metadata();
    return mla_reflection_register_struct(metadata.name, T::metadata);
}

#define mla_reflection_auto_register_struct(sturct_type) \
void mla_reflection_auto_register_struct_##sturct_type() { \
    mla_reflection_register_struct<sturct_type>();  \
} \
mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_low_level_postSetup, mla_reflection_auto_register_struct_##sturct_type) \

#define mla_reflection_struct_field_bool(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_BOOL, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_int8(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT8, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_int16(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT16, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_int32(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT32, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_int64(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_INT64, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_uint8(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT8, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_uint16(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT16, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_uint32(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT32, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_uint64(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_UINT64, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_float(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_FLOAT, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_double(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_DOUBLE, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_string(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_STRING, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_bytes(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_BYTES, MLA_REFLECTION_TYPE_INVALID, mla_string_empty()))

#define mla_reflection_struct_field_struct(metadata, struct_type, field_name, struct_type_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_STRUCT, MLA_REFLECTION_TYPE_INVALID, mla_string_const(struct_type_name)))

#define mla_reflection_struct_field_struct_list(metadata, struct_type, field_name, element_struct_type_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_STRUCT, mla_string_const(element_struct_type_name)))

#define mla_reflection_struct_field_bool_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_BOOL, mla_string_empty()))

#define mla_reflection_struct_field_int8_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT8, mla_string_empty()))

#define mla_reflection_struct_field_int16_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT16, mla_string_empty()))

#define mla_reflection_struct_field_int32_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT32, mla_string_empty()))

#define mla_reflection_struct_field_int64_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_INT64, mla_string_empty()))

#define mla_reflection_struct_field_uint8_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT8, mla_string_empty()))

#define mla_reflection_struct_field_uint16_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT16, mla_string_empty()))

#define mla_reflection_struct_field_uint32_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT32, mla_string_empty()))

#define mla_reflection_struct_field_uint64_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_UINT64, mla_string_empty()))

#define mla_reflection_struct_field_float_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_FLOAT, mla_string_empty()))

#define mla_reflection_struct_field_double_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_DOUBLE, mla_string_empty()))

#define mla_reflection_struct_field_string_list(metadata, struct_type, field_name) \
    mla_reflection_struct_metadata_add_field(metadata, mla_reflection_struct_field(mla_string_const(#field_name), mla_offsetof(struct_type, field_name), MLA_REFLECTION_TYPE_LIST, MLA_REFLECTION_TYPE_STRING, mla_string_empty()))


#endif