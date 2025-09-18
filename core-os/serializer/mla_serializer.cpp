//
// Created by chris on 9/16/2025.
//

#include "mla_serializer.h"

mla_serialize_definition_t mla_serialize_definition(
    const mla_serialize_definition_read_function_t read_function,
    const mla_serialize_definition_write_function_t write_function
) {
    return {
        read_function,
        write_function
    };
}

mla_bool_t mla_deserializer_token_type_is_value(const mla_deserializer_token_type_t &token_type) {
    return token_type == MLA_DESERIALIZER_VALUE_BOOL ||
           token_type == MLA_DESERIALIZER_VALUE_INT8 ||
           token_type == MLA_DESERIALIZER_VALUE_INT16 ||
           token_type == MLA_DESERIALIZER_VALUE_INT32 ||
           token_type == MLA_DESERIALIZER_VALUE_INT64 ||
           token_type == MLA_DESERIALIZER_VALUE_UINT8 ||
           token_type == MLA_DESERIALIZER_VALUE_UINT16 ||
           token_type == MLA_DESERIALIZER_VALUE_UINT32 ||
           token_type == MLA_DESERIALIZER_VALUE_UINT64 ||
           token_type == MLA_DESERIALIZER_VALUE_FLOAT ||
           token_type == MLA_DESERIALIZER_VALUE_DOUBLE ||
           token_type == MLA_DESERIALIZER_VALUE_STRING ||
           token_type == MLA_DESERIALIZER_VALUE_BYTES;
}

void mla_serializer_write_struct(mla_serializer_t &serializer, const mla_pointer_t value,
                                 const mla_serialize_definition_write_function_t &write_function) {
    serializer.write_start_struct(serializer);
    write_function(serializer, value);
    serializer.write_end_struct(serializer);
}

void mla_serializer_write_struct(mla_serializer_t &serializer, const mla_string_t &name, const mla_pointer_t value,
                                 const mla_serialize_definition_write_function_t &write_function) {
    serializer.write_property_name(serializer, name);
    mla_serializer_write_struct(serializer, value, write_function);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_bool_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_bool(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_uint8_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_uint8(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_uint16_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_uint16(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_uint32_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_uint32(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_uint64_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_uint64(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_int8_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_int8(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_int16_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_int16(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_int32_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_int32(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_int64_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_int64(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_float_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_float(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_double_t> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_double(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}

void mla_serializer_write_list(mla_serializer_t &serializer, const mla_string_t &name,
                               const mla_array_list_t<mla_string_t, mla_string_initializer> &list) {
    serializer.write_property_name(serializer, name);
    serializer.write_start_list(serializer);
    for (mla_size_t i = 0; i < mla_array_list_size(list); ++i) {
        serializer.write_string(serializer, *mla_array_list_get_ref(list, i));
    }
    serializer.write_end_list(serializer);
}


mla_bool_t mla_deserializer_read_struct(mla_deserializer_t &deserializer, mla_pointer_t data,
                                        const mla_serialize_definition_read_function_t &read_function) {
    if (deserializer.current_token.type != MLA_DESERIALIZER_STRUCT_START) {
        // Not a struct start
        return false;
    }

    while (deserializer.read_next(deserializer)) {
        if (deserializer.current_token.type == MLA_DESERIALIZER_PROPERTY_NAME) {
            mla_string_t property_name = deserializer.current_token.complex.property_name;

            // Read property value
            if (!deserializer.read_next(deserializer)) {
                // not successful. Steam ends unexpectedly
                return false;
            }

            mla_deserializer_read_result_t result = read_function(deserializer, data, property_name);

            if (result == MLA_DESERIALIZER_READ_ERROR) {
                return false;
            } else if (result == MLA_DESERIALIZER_READ_SKIPPED) {
                mla_deserializer_skip_property_value(deserializer);
            }
        } else if (deserializer.current_token.type == MLA_DESERIALIZER_STRUCT_END) {
            // readed the end of the structed
            return true;
        } else {
            // Unexpected token type
            return false;
        }
    }

    return true;
}


mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_bool_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_bool_t value = deserializer.current_token.simple.bool_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_uint8_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_uint8_t value = deserializer.current_token.simple.uint8_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_uint16_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_uint16_t value = deserializer.current_token.simple.uint16_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_uint32_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_uint32_t value = deserializer.current_token.simple.uint32_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_uint64_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_uint64_t value = deserializer.current_token.simple.uint64_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_int8_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_int8_t value = deserializer.current_token.simple.int8_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_int16_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_int16_t value = deserializer.current_token.simple.int16_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_int32_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {

        while (deserializer.read_next(deserializer)) {

            if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_END) {
                // We are done
                return true;
            }

            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_int32_t value = deserializer.current_token.simple.int32_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_int64_t> &list) {

    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_int64_t value = deserializer.current_token.simple.int64_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_float_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_float_t value = deserializer.current_token.simple.float_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer, mla_array_list_t<mla_double_t> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_double_t value = deserializer.current_token.simple.double_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

mla_bool_t mla_serializer_read_list(mla_deserializer_t &deserializer,
                                    mla_array_list_t<mla_string_t, mla_string_initializer> &list) {
    if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
        while (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END && deserializer.read_next(deserializer)) {
            if (!mla_deserializer_token_type_is_value(deserializer.current_token.type)) {
                // Wrong struct ure
                return false;
            }

            mla_string_t value = deserializer.current_token.complex.string_value;
            mla_array_list_add(list, value);
        }
    }

    if (deserializer.current_token.type != MLA_DESERIALIZER_LIST_END) {
        // Expected LIST_END
        return false;
    }

    return true;
}

void mla_deserializer_skip_property_value(mla_deserializer_t &deserializer) {
    switch (deserializer.current_token.type) {
        case MLA_DESERIALIZER_NULL:
            deserializer.read_next(deserializer);
            break;
        case MLA_DESERIALIZER_PROPERTY_NAME:
            deserializer.read_next(deserializer);
            break;
        case MLA_DESERIALIZER_STRUCT_START:

            while (deserializer.current_token.type != MLA_DESERIALIZER_STRUCT_END && deserializer.read_next(
                       deserializer)) {
                if (deserializer.current_token.type == MLA_DESERIALIZER_PROPERTY_NAME) {
                    // Read property name
                    deserializer.read_next(deserializer);
                    // Skip property value
                    mla_deserializer_skip_property_value(deserializer);
                }
            }
            if (deserializer.current_token.type == MLA_DESERIALIZER_STRUCT_END) {
                deserializer.read_next(deserializer);
            }

            break;
        case MLA_DESERIALIZER_STRUCT_END:
            deserializer.read_next(deserializer);
            break;
        case MLA_DESERIALIZER_LIST_START: {
            mla_size_t nesting_level = 1;

            while (nesting_level > 0 && deserializer.read_next(deserializer)) {
                if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_START) {
                    nesting_level++;
                } else if (deserializer.current_token.type == MLA_DESERIALIZER_LIST_END) {
                    nesting_level--;
                } else if (deserializer.current_token.type == MLA_DESERIALIZER_STRUCT_START) {
                    // Skip struct
                    mla_deserializer_skip_property_value(deserializer);
                }
            }
        }


        break;
        case MLA_DESERIALIZER_LIST_END:
        case MLA_DESERIALIZER_VALUE_BOOL:
        case MLA_DESERIALIZER_VALUE_INT8:
        case MLA_DESERIALIZER_VALUE_INT16:
        case MLA_DESERIALIZER_VALUE_INT32:
        case MLA_DESERIALIZER_VALUE_INT64:
        case MLA_DESERIALIZER_VALUE_UINT8:
        case MLA_DESERIALIZER_VALUE_UINT16:
        case MLA_DESERIALIZER_VALUE_UINT32:
        case MLA_DESERIALIZER_VALUE_UINT64:
        case MLA_DESERIALIZER_VALUE_FLOAT:
        case MLA_DESERIALIZER_VALUE_DOUBLE:
        case MLA_DESERIALIZER_VALUE_STRING:
        case MLA_DESERIALIZER_VALUE_BYTES:
            deserializer.read_next(deserializer);
            break;
    }
}


mla_bool_t mla_mla_deserializer_convert_to_bool(const mla_deserializer_token_t &token, mla_bool_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_BOOL:
            *out_value = token.simple.bool_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_int8(const mla_deserializer_token_t &token, mla_int8_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            *out_value = token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:

            if (token.simple.int16_value < mla_int8_min || token.simple.int16_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.int16_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT32:

            if (token.simple.int32_value < mla_int8_min || token.simple.int32_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.int32_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT64:

            if (token.simple.int64_value < mla_int8_min || token.simple.int64_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.int64_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT8:

            if (token.simple.uint8_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.uint8_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT16:

            if (token.simple.uint16_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.uint16_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT32:

            if (token.simple.uint32_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.uint32_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT64:

            if (token.simple.uint64_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.uint64_value;
            return true;

        case MLA_DESERIALIZER_VALUE_FLOAT:

            if (token.simple.float_value < mla_int8_min || token.simple.float_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.float_value;
            return true;

        case MLA_DESERIALIZER_VALUE_DOUBLE:

            if (token.simple.double_value < mla_int8_min || token.simple.double_value > mla_int8_max) {
                return false;
            }
            *out_value = (mla_int8_t) token.simple.double_value;
            return true;

        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_int16(const mla_deserializer_token_t &token, mla_int16_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:

            *out_value = 0;
            return true;

        case MLA_DESERIALIZER_VALUE_INT8:

            *out_value = token.simple.int8_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT16:

            *out_value = token.simple.int16_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT32:

            if (token.simple.int32_value < mla_int16_min || token.simple.int32_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.int32_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT64:

            if (token.simple.int64_value < mla_int16_min || token.simple.int64_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.int64_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT8:

            *out_value = (mla_int16_t) token.simple.uint8_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT16:

            if (token.simple.uint16_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.uint16_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT32:

            if (token.simple.uint32_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.uint32_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT64:

            if (token.simple.uint64_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.uint64_value;
            return true;

        case MLA_DESERIALIZER_VALUE_FLOAT:

            if (token.simple.float_value < mla_int16_min || token.simple.float_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.float_value;
            return true;

        case MLA_DESERIALIZER_VALUE_DOUBLE:

            if (token.simple.double_value < mla_int16_min || token.simple.double_value > mla_int16_max) {
                return false;
            }
            *out_value = (mla_int16_t) token.simple.double_value;
            return true;

        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_int32(const mla_deserializer_token_t &token, mla_int32_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:

            *out_value = token.simple.int8_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT16:

            *out_value = token.simple.int16_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT32:

            *out_value = token.simple.int32_value;
            return true;

        case MLA_DESERIALIZER_VALUE_INT64:

            if (token.simple.int64_value > mla_int32_max) {
                return false;
            }
            *out_value = (mla_int32_t) token.simple.int64_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT8:

            *out_value = (mla_int32_t) token.simple.uint8_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT16:

            if (token.simple.uint16_value > mla_int32_max) {
                return false;
            }
            *out_value = (mla_int32_t) token.simple.uint16_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT32:

            if (token.simple.uint32_value > mla_int32_max) {
                return false;
            }
            *out_value = (mla_int32_t) token.simple.uint32_value;
            return true;

        case MLA_DESERIALIZER_VALUE_UINT64:

            if (token.simple.uint64_value > mla_int32_max) {
                return false;
            }
            *out_value = (mla_int32_t) token.simple.uint64_value;
            return true;

        case MLA_DESERIALIZER_VALUE_FLOAT:

            if (token.simple.float_value > (mla_float_t)mla_int32_max) {
                return false;
            }
            *out_value = (mla_int32_t) token.simple.float_value;
            return true;

        case MLA_DESERIALIZER_VALUE_DOUBLE:

            if (token.simple.double_value > (mla_float_t)mla_int32_max) {
                return false;
            }
            *out_value = (mla_int32_t) token.simple.double_value;
            return true;

        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_int64(const mla_deserializer_token_t &token, mla_int64_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            *out_value = token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            *out_value = token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            *out_value = token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            *out_value = token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            *out_value = token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            *out_value = token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            if (token.simple.uint64_value > mla_int64_max) {
                return false;
            }
            *out_value = (mla_int64_t) token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            if (token.simple.float_value > (mla_float_t)mla_int64_max) {
                return false;
            }
            *out_value = (mla_int64_t) token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            if (token.simple.double_value > (mla_float_t)mla_int64_max) {
                return false;
            }
            *out_value = (mla_int64_t) token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_uint8(const mla_deserializer_token_t &token, mla_uint8_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            if (token.simple.int8_value < 0) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            if (token.simple.int16_value < 0 || token.simple.int16_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            if (token.simple.int32_value < 0 || token.simple.int32_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            if (token.simple.int64_value < 0 || token.simple.int64_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            if (token.simple.uint16_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            if (token.simple.uint32_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            if (token.simple.uint64_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            if (token.simple.float_value < 0 || token.simple.float_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            if (token.simple.double_value < 0 || token.simple.double_value > mla_uint8_max) {
                return false;
            }
            *out_value = (mla_uint8_t) token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_uint16(const mla_deserializer_token_t &token, mla_uint16_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            if (token.simple.int8_value < 0) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            if (token.simple.int16_value < 0) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            if (token.simple.int32_value < 0 || token.simple.int32_value > mla_uint16_max) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            if (token.simple.int64_value < 0 || token.simple.int64_value > mla_uint16_max) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            *out_value = token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            if (token.simple.uint32_value > mla_uint16_max) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            if (token.simple.uint64_value > mla_uint16_max) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            if (token.simple.float_value < 0 || token.simple.float_value > mla_uint16_max) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            if (token.simple.double_value < 0 || token.simple.double_value > mla_uint16_max) {
                return false;
            }
            *out_value = (mla_uint16_t) token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_uint32(const mla_deserializer_token_t &token, mla_uint32_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            if (token.simple.int8_value < 0) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            if (token.simple.int16_value < 0) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            if (token.simple.int32_value < 0) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            if (token.simple.int64_value < 0 || token.simple.int64_value > mla_uint32_max) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            *out_value = token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            *out_value = token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            if (token.simple.uint64_value > mla_uint32_max) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            if (token.simple.float_value < 0 || token.simple.float_value > (mla_float_t)mla_uint32_max) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            if (token.simple.double_value < 0 || token.simple.double_value > mla_uint32_max) {
                return false;
            }
            *out_value = (mla_uint32_t) token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_uint64(const mla_deserializer_token_t &token, mla_uint64_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            if (token.simple.int8_value < 0) {
                return false;
            }
            *out_value = (mla_uint64_t) token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            if (token.simple.int16_value < 0) {
                return false;
            }
            *out_value = (mla_uint64_t) token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            if (token.simple.int32_value < 0) {
                return false;
            }
            *out_value = (mla_uint64_t) token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            if (token.simple.int64_value < 0) {
                return false;
            }
            *out_value = (mla_uint64_t) token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            *out_value = token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            *out_value = token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            *out_value = token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            if (token.simple.float_value < 0) {
                return false;
            }
            *out_value = (mla_uint64_t) token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            if (token.simple.double_value < 0) {
                return false;
            }
            *out_value = (mla_uint64_t) token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_float(const mla_deserializer_token_t &token, mla_float_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0.0f;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            *out_value = (mla_float_t) token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            *out_value = (mla_float_t) token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            *out_value = (mla_float_t) token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            *out_value = (mla_float_t) token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = (mla_float_t) token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            *out_value = (mla_float_t) token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            *out_value = (mla_float_t) token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            *out_value = (mla_float_t) token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            *out_value = token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            *out_value = (mla_float_t) token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_double(const mla_deserializer_token_t &token, mla_double_t *out_value) {
    switch (token.type) {
        case MLA_DESERIALIZER_NULL:
            *out_value = 0.0;
            return true;
        case MLA_DESERIALIZER_VALUE_INT8:
            *out_value = (mla_double_t) token.simple.int8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT16:
            *out_value = (mla_double_t) token.simple.int16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT32:
            *out_value = (mla_double_t) token.simple.int32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_INT64:
            *out_value = (mla_double_t) token.simple.int64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT8:
            *out_value = (mla_double_t) token.simple.uint8_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT16:
            *out_value = (mla_double_t) token.simple.uint16_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT32:
            *out_value = (mla_double_t) token.simple.uint32_value;
            return true;
        case MLA_DESERIALIZER_VALUE_UINT64:
            *out_value = (mla_double_t) token.simple.uint64_value;
            return true;
        case MLA_DESERIALIZER_VALUE_FLOAT:
            *out_value = (mla_double_t) token.simple.float_value;
            return true;
        case MLA_DESERIALIZER_VALUE_DOUBLE:
            *out_value = token.simple.double_value;
            return true;
        default:
            return false;
    }
}

mla_bool_t mla_mla_deserializer_convert_to_string(const mla_deserializer_token_t &token, mla_string_t *out_value) {
    if (token.type == MLA_DESERIALIZER_VALUE_STRING) {
        *out_value = token.complex.string_value;
        return true;
    }
    return false;
}

mla_bool_t mla_mla_deserializer_convert_to_bytes(const mla_deserializer_token_t &token, mla_bytes_t *out_value) {
    if (token.type == MLA_DESERIALIZER_VALUE_BYTES) {
        *out_value = token.complex.bytes_value;
        return true;
    }
    return false;
}
