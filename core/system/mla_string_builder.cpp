//
// Created by chris on 5/17/2026.
//

#include "mla_string_builder.h"

static mla_size_t __mla_string_builder_max(mla_size_t p_Left, mla_size_t p_Right) {
    return p_Left > p_Right ? p_Left : p_Right;
}

static mla_size_t __mla_string_builder_normalize_capacity(mla_size_t p_Capacity) {
    return p_Capacity == 0 ? (mla_size_t)1 : p_Capacity;
}

static mla_bool_t __mla_string_builder_ensure_capacity(mla_string_builder_t& p_Builder, mla_size_t p_RequiredLength) {
    if (p_RequiredLength <= p_Builder.bufferSize && !mla_pointer_is_null(p_Builder.buffer)) {
        return true;
    }

    mla_size_t newCapacity = __mla_string_builder_normalize_capacity(p_Builder.bufferSize);

    while (newCapacity < p_RequiredLength) {
        if (newCapacity > (mla_size_max / 2)) {
            newCapacity = p_RequiredLength;
            break;
        }
        newCapacity *= 2;
    }

    newCapacity = __mla_string_builder_normalize_capacity(newCapacity);
    mla_pointer_t newBuffer = mla_create_char_array(newCapacity + 1);
    mla_char_t* newBufferData = mla_pointer_get_data<mla_char_t>(newBuffer);

    if (newBufferData == nullptr) {
        return false;
    }

    if (!mla_pointer_is_null(p_Builder.buffer) && p_Builder.position > 0) {
        mla_char_t* oldBufferData = mla_pointer_get_data<mla_char_t>(p_Builder.buffer);

        if (oldBufferData == nullptr) {
            return false;
        }

        mla_memcpy(newBufferData, oldBufferData, p_Builder.position);
    }

    newBufferData[p_Builder.position] = '\0';
    p_Builder.buffer = newBuffer;
    p_Builder.bufferSize = newCapacity;
    return true;
}

static mla_bool_t __mla_string_builder_append_data(mla_string_builder_t& p_Builder, const mla_char_t* p_Data, mla_size_t p_Length) {
    if (p_Length == 0) {
        return true;
    }

    if (p_Data == nullptr) {
        return false;
    }

    mla_size_t requiredLength = p_Builder.position + p_Length;

    if (requiredLength < p_Builder.position) {
        return false;
    }

    if (!__mla_string_builder_ensure_capacity(p_Builder, requiredLength)) {
        return false;
    }

    mla_char_t* builderData = mla_pointer_get_data<mla_char_t>(p_Builder.buffer);

    if (builderData == nullptr) {
        return false;
    }

    mla_memcpy(builderData + p_Builder.position, p_Data, p_Length);
    p_Builder.position = requiredLength;
    builderData[p_Builder.position] = '\0';
    return true;
}

mla_string_builder_t mla_string_builder_create() {
    return mla_string_builder_create((mla_size_t)mla_global_config_string_builder_default_buffer_size);
}

mla_string_builder_t mla_string_builder_create(mla_size_t p_InitialBufferSize) {
    mla_string_builder_t builder = {mla_pointer_null(), 0, 0};
    builder.bufferSize = __mla_string_builder_normalize_capacity(p_InitialBufferSize);

    if (!__mla_string_builder_ensure_capacity(builder, 0)) {
        return mla_string_builder_t{mla_pointer_null(), 0, 0};
    }

    return builder;
}

mla_string_builder_t mla_string_builder_create(const mla_pointer_t& p_Buffer, mla_size_t p_Position) {
    if (mla_pointer_is_null(p_Buffer)) {
        return mla_string_builder_create(__mla_string_builder_max((mla_size_t)mla_global_config_string_builder_default_buffer_size, p_Position));
    }

    mla_string_builder_t builder = {p_Buffer, p_Position, p_Position};
    return builder;
}

void mla_string_builder_destroy(mla_string_builder_t& p_Builder) {
    p_Builder.buffer = mla_pointer_null();
    p_Builder.bufferSize = 0;
    p_Builder.position = 0;
}

mla_string_t mla_string_builder_to_string(const mla_string_builder_t& p_Builder) {
    if (mla_pointer_is_null(p_Builder.buffer) || p_Builder.position == 0) {
        return mla_string_empty();
    }

    const mla_char_t* data = mla_pointer_get_data<mla_char_t>(p_Builder.buffer);

    if (data == nullptr) {
        return mla_string_empty();
    }

    return mla_string_copy(data, p_Builder.position);
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, const mla_string_t& p_Value) {
    const mla_char_t* valueData = mla_string_data(p_Value);
    mla_size_t valueLength = mla_string_length(p_Value);
    return __mla_string_builder_append_data(p_Builder, valueData, valueLength);
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_bool_t p_Value) {
    mla_string_t converted = mla_string_from_bool(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int8_t p_Value) {
    mla_string_t converted = mla_string_from_int8(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint8_t p_Value) {
    mla_string_t converted = mla_string_from_uint8(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int16_t p_Value) {
    mla_string_t converted = mla_string_from_int16(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint16_t p_Value) {
    mla_string_t converted = mla_string_from_uint16(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int32_t p_Value) {
    mla_string_t converted = mla_string_from_int32(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint32_t p_Value) {
    mla_string_t converted = mla_string_from_uint32(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int64_t p_Value) {
    mla_string_t converted = mla_string_from_int64(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint64_t p_Value) {
    mla_string_t converted = mla_string_from_uint64(p_Value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_float_t p_Value) {
    return mla_string_builder_append(p_Builder, p_Value, 6);
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_float_t p_Value, mla_size_t p_DecimalPlaces) {
    mla_string_t converted = mla_string_from_float(p_Value, p_DecimalPlaces);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_double_t p_Value) {
    return mla_string_builder_append(p_Builder, p_Value, 6);
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_double_t p_Value, mla_size_t p_DecimalPlaces) {
    mla_string_t converted = mla_string_from_double(p_Value, p_DecimalPlaces);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_char_t p_Value) {
    return __mla_string_builder_append_data(p_Builder, &p_Value, 1);
}

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_platform_pointer_t p_Value) {
    mla_uint64_t value = reinterpret_cast<mla_uint64_t>(p_Value);
    mla_string_t converted = mla_string_from_uint64_hex(value);
    mla_bool_t result = mla_string_builder_append(p_Builder, converted);
    mla_string_destroy(converted);
    return result;
}
