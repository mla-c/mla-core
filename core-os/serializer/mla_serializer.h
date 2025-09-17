//
// Basic serialization interface for structured data.
//
// Created by chris on 9/14/2025.
//

#ifndef COREOS_MLA_SERIALIZER_H
#define COREOS_MLA_SERIALIZER_H

#include "../system/mla_string.h"
#include "../system/mla_stream.h"
#include "../system/mla_array_list.h"
#include "../system/mla_bytes.h"

struct mla_serializer_t {

    const mla_stream_output_t output;
    mla_callback_userdata userdata;

    void (*const write_start_struct)(const mla_serializer_t& instance);
    void (*const write_end_struct)(const mla_serializer_t& instance);

    void (*const write_start_list)(const mla_serializer_t& instance);
    void (*const write_end_list)(const mla_serializer_t& instance);

    void (*const write_property_name)(const mla_serializer_t& instance, const mla_string_t& name);

    void (*const write_bool)(const mla_serializer_t& instance, const mla_bool_t value);

    void (*const write_int8)(const mla_serializer_t& instance, const mla_int8_t value);
    void (*const write_int16)(const mla_serializer_t& instance, const mla_int16_t value);
    void (*const write_int32)(const mla_serializer_t& instance, const mla_int32_t value);
    void (*const write_int64)(const mla_serializer_t& instance, const mla_int64_t value);
    void (*const write_uint8)(const mla_serializer_t& instance, const mla_uint8_t value);
    void (*const write_uint16)(const mla_serializer_t& instance, const mla_uint16_t value);
    void (*const write_uint32)(const mla_serializer_t& instance, const mla_uint32_t value);
    void (*const write_uint64)(const mla_serializer_t& instance, const mla_uint64_t value);

    void (*const write_float)(const mla_serializer_t& instance, const mla_float_t value);
    void (*const write_double)(const mla_serializer_t& instance, const mla_double_t value);

    void (*const write_string)(const mla_serializer_t& instance, const mla_string_t& value);
    void (*const write_bytes)(const mla_serializer_t& instance, const mla_bytes_t bytes);

};

enum mla_deserializer_token_type_t {
    MLA_DESERIALIZER_NULL,
    MLA_DESERIALIZER_PROPERTY_NAME,
    MLA_DESERIALIZER_STRUCT_START,
    MLA_DESERIALIZER_STRUCT_END,
    MLA_DESERIALIZER_LIST_START,
    MLA_DESERIALIZER_LIST_END,

    // Value types
    MLA_DESERIALIZER_VALUE_BOOL,
    MLA_DESERIALIZER_VALUE_INT8,
    MLA_DESERIALIZER_VALUE_INT16,
    MLA_DESERIALIZER_VALUE_INT32,
    MLA_DESERIALIZER_VALUE_INT64,
    MLA_DESERIALIZER_VALUE_UINT8,
    MLA_DESERIALIZER_VALUE_UINT16,
    MLA_DESERIALIZER_VALUE_UINT32,
    MLA_DESERIALIZER_VALUE_UINT64,
    MLA_DESERIALIZER_VALUE_FLOAT,
    MLA_DESERIALIZER_VALUE_DOUBLE,
    MLA_DESERIALIZER_VALUE_STRING,
    MLA_DESERIALIZER_VALUE_BYTES
};

mla_bool_t mla_deserializer_token_type_is_value(const mla_deserializer_token_type_t& token_type);

struct mla_deserializer_token_t {
    mla_deserializer_token_type_t type;

    struct {
        // All reference bytes an not be stored in an union
        // because they are not used at the same time as the value types
        mla_string_t property_name;
        mla_string_t string_value;
        mla_bytes_t bytes_value;
    } complex;

    union {

        mla_bool_t bool_value;
        mla_int8_t int8_value;
        mla_int16_t int16_value;
        mla_int32_t int32_value;
        mla_int64_t int64_value;
        mla_uint8_t uint8_value;
        mla_uint16_t uint16_value;
        mla_uint32_t uint32_value;
        mla_uint64_t uint64_value;
        mla_float_t float_value;
        mla_double_t double_value;
    } simple;
};

struct mla_deserializer_t {

    const mla_stream_input_t input;
    mla_callback_userdata userdata;

    mla_deserializer_token_t current_token;
    mla_bool_t (*const read_next)(mla_deserializer_t& instance);
};

enum mla_deserializer_read_result_t {
    MLA_DESERIALIZER_READ_HANDLED,
    MLA_DESERIALIZER_READ_SKIPPED,
    MLA_DESERIALIZER_READ_ERROR
};

// Called for every property in a struct
typedef mla_deserializer_read_result_t (*mla_serialize_definition_read_function_t)(mla_deserializer_t& deserializer, mla_pointer_t config, const mla_string_t& property_name);

// Called to write the entire struct
typedef void (*mla_serialize_definition_write_function_t)(mla_serializer_t& serializer, const mla_pointer_t config);

/// Definition of serialization functions for a specific data structure
struct mla_serialize_definition_t {
    const mla_serialize_definition_read_function_t read_function;
    const mla_serialize_definition_write_function_t write_function;
};

mla_serialize_definition_t mla_serialize_definition(
        const mla_serialize_definition_read_function_t read_function,
        const mla_serialize_definition_write_function_t write_function
);

/////////////////////////////////////////////////////////////////////////////
/// Serializer Helpers
//////////////////////////////////////////////////////////////////////////////

void mla_serializer_write_struct(mla_serializer_t& serializer, const mla_pointer_t value, const mla_serialize_definition_write_function_t& write_function);
void mla_serializer_write_struct(mla_serializer_t& serializer, const mla_string_t& name, const mla_pointer_t value, const mla_serialize_definition_write_function_t& write_function);

void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_bool_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint8_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint16_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint32_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_uint64_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int8_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int16_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int32_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_int64_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_float_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_double_t>& list);
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<mla_string_t, mla_string_initializer>& list);

template <mla_array_list_template>
void mla_serializer_write_list(mla_serializer_t& serializer, const mla_string_t& name, const mla_array_list_t<T, TInit>& list, const mla_serialize_definition_write_function_t& write_function) {

    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        mla_serializer_write_struct(serializer, mla_array_list_get_ref(list, i), write_function);
    }

    serializer.write_end_list(serializer);

}

//////////////////////////////////////////////////////////////////////////////////
/// Deserializer Helpers
//////////////////////////////////////////////////////////////////////////////////

mla_bool_t mla_deserializer_read_struct(mla_deserializer_t& deserializer, mla_pointer_t config, const mla_serialize_definition_read_function_t& read_function);

mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_bool_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint8_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint16_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint32_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_uint64_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int8_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int16_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int32_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_int64_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_float_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_double_t>& list);
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<mla_string_t, mla_string_initializer>& list);

template <mla_array_list_template>
mla_bool_t mla_serializer_read_list(mla_deserializer_t& deserializer, mla_array_list_t<T, TInit>& list, const mla_serialize_definition_read_function_t& read_function) {

    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {

        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {

            if (deserializer.current_token.type != MLA_DESERIALIZER_STRUCT_START) {
                // Wrong struct ure
                return false;
            }

            T item = TInit::init();
            if (mla_deserializer_read_struct(deserializer, &item, read_function)) {
                mla_array_list_add(list, item);
            } else {
                // Error reading struct
                return false;
            }

        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

void mla_deserializer_skip_property_value(mla_deserializer_t& deserializer);

///////////////////////////////////////////////////////////////////////////////
/// Conversion Helpers
//////////////////////////////////////////////////////////////////////////////

mla_bool_t mla_mla_deserializer_convert_to_bool(const mla_deserializer_token_t& token, mla_bool_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_int8(const mla_deserializer_token_t& token, mla_int8_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_int16(const mla_deserializer_token_t& token, mla_int16_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_int32(const mla_deserializer_token_t& token, mla_int32_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_int64(const mla_deserializer_token_t& token, mla_int64_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_uint8(const mla_deserializer_token_t& token, mla_uint8_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_uint16(const mla_deserializer_token_t& token, mla_uint16_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_uint32(const mla_deserializer_token_t& token, mla_uint32_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_uint64(const mla_deserializer_token_t& token, mla_uint64_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_float(const mla_deserializer_token_t& token, mla_float_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_double(const mla_deserializer_token_t& token, mla_double_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_string(const mla_deserializer_token_t& token, mla_string_t* out_value);
mla_bool_t mla_mla_deserializer_convert_to_bytes(const mla_deserializer_token_t& token, mla_bytes_t* out_value);


///////////////////////////////////////////////////////////////////////////////
/// Macros for easier usage
//////////////////////////////////////////////////////////////////////////////

// Deserializer
#define mla_deserializer_read_bool(instance, setter)\
    {\
    mla_bool_t value = false;\
    if (mla_mla_deserializer_convert_to_bool(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int8(instance, setter)\
    {\
    mla_int8_t value = 0;\
    if (mla_mla_deserializer_convert_to_int8(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int16(instance, setter)\
    {\
    mla_int16_t value = 0;\
    if (mla_mla_deserializer_convert_to_int16(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int32(instance, setter)\
    {\
    mla_int32_t value = 0;\
    if (mla_mla_deserializer_convert_to_int32(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_int64(instance, setter)\
    {\
    mla_int64_t value = 0;\
    if (mla_mla_deserializer_convert_to_int64(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint8(instance, setter)\
    {\
    mla_uint8_t value = 0;\
    if (mla_mla_deserializer_convert_to_uint8(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint16(instance, setter)\
    {\
    mla_uint16_t value = 0;\
    if (mla_mla_deserializer_convert_to_uint16(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint32(instance, setter)\
    {\
    mla_uint32_t value = 0;\
    if (mla_mla_deserializer_convert_to_uint32(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_uint64(instance, setter)\
    {\
    mla_uint64_t value = 0;\
    if (mla_mla_deserializer_convert_to_uint64(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_float(instance, setter)\
    {\
    mla_float_t value = 0;\
    if (mla_mla_deserializer_convert_to_float(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_double(instance, setter)\
    {\
    mla_double_t value = 0;\
    if (mla_mla_deserializer_convert_to_double(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_string(instance, setter)\
    {\
    mla_string_t value = mla_string_empty();\
    if (mla_mla_deserializer_convert_to_string(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

#define mla_deserializer_read_bytes(instance, setter)\
    {\
    mla_bytes_t value = mla_bytes_empty();\
    if (mla_mla_deserializer_convert_to_bytes(instance.current_token, &value)) {\
        setter = value;\
        return MLA_DESERIALIZER_READ_HANDLED;\
    } else {\
        return MLA_DESERIALIZER_READ_ERROR;\
    }\
    }

// Serializer

#define mla_serializer_write_bool(instance, name, value) \
    instance.write_property_name(instance, name);\
    instance.write_bool(instance, value)

#define mla_serializer_write_int8(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_int8(instance, value)

#define mla_serializer_write_int16(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_int16(instance, value)

#define mla_serializer_write_int32(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_int32(instance, value)

#define mla_serializer_write_int64(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_int64(instance, value)

#define mla_serializer_write_uint8(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_uint8(instance, value)

#define mla_serializer_write_uint16(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_uint16(instance, value)

#define mla_serializer_write_uint32(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_uint32(instance, value)

#define mla_serializer_write_uint64(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_uint64(instance, value)

#define mla_serializer_write_float(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_float(instance, value)

#define mla_serializer_write_double(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_double(instance, value)

#define mla_serializer_write_string(instance, name, value)\
    instance.write_property_name(instance, name);\
    instance.write_string(instance, value)

#define mla_serializer_write_bytes(instance, name, bytes)\
    instance.write_property_name(instance, name);\
    instance.write_bytes(instance, bytes)


#endif