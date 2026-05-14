//
// Created by christian on 1/11/26.
//

#include "mla_data_types.h"

const mla_char_t* mla_find_filename_from_path(const mla_char_t* path) {

    const mla_char_t* last = nullptr;

    while (*path != '\0') {
        // Check for both '/' and '\' as path separators because we dont not know the OS which has procedure the birnary
        if (*path == '/' || *path == '\\') {
            last = path;
        }
        path++;
    }

    if (last != nullptr) {
        return last + 1;
    }

    return path;

}

mla_platform_pointer_t mla_platform_malloc_with_check(mla_size_t size, const mla_char_t* path, const mla_char_t* function_name) {

    mla_platform_pointer_t ptr = g_low_level_access.malloc(size);
    if (ptr == nullptr) {

        const mla_char_t* fileName =  mla_find_filename_from_path(path);
        g_low_level_access.on_malloc_failure(size, fileName, function_name);
    }
    return ptr;
}

mla_dynamic_data_t mla_dynamic_data_empty() {
    mla_dynamic_data_t data = {};
    data.asInt64 = 0;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_bool(mla_bool_t value) {
    mla_dynamic_data_t data = {};
    data.asBool = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_int8(mla_int8_t value) {
    mla_dynamic_data_t data = {};
    data.asInt8 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_uint8(mla_uint8_t value) {
    mla_dynamic_data_t data = {};
    data.asUint8 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_int16(mla_int16_t value) {
    mla_dynamic_data_t data = {};
    data.asInt16 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_uint16(mla_uint16_t value) {
    mla_dynamic_data_t data = {};
    data.asUint16 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_int32(mla_int32_t value) {
    mla_dynamic_data_t data = {};
    data.asInt32 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_uint32(mla_uint32_t value) {
    mla_dynamic_data_t data = {};
    data.asUint32 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_int64(mla_int64_t value) {
    mla_dynamic_data_t data = {};
    data.asInt64 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_uint64(mla_uint64_t value) {
    mla_dynamic_data_t data = {};
    data.asUint64 = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_float(mla_float_t value) {
    mla_dynamic_data_t data = {};
    data.asFloat = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_double(mla_double_t value) {
    mla_dynamic_data_t data = {};
    data.asDouble = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_pointer(mla_platform_pointer_t value) {
    mla_dynamic_data_t data = {};
    data.asPointer = value;
    return data;
}

mla_dynamic_data_t mla_dynamic_data_from_char(mla_char_t value) {
    mla_dynamic_data_t data = {};
    data.asChar = value;
    return data;
}