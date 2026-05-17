//
// Created by chris on 5/17/2026.
//

#ifndef MLA_STRING_BUILDER_H
#define MLA_STRING_BUILDER_H

#include "mla_string.h"

struct mla_string_builder_t {
    mla_pointer_t buffer;
    mla_size_t bufferSize;
    mla_size_t position;
};

mla_string_builder_t mla_string_builder_create();
mla_string_builder_t mla_string_builder_create(mla_size_t p_InitialBufferSize);
mla_string_builder_t mla_string_builder_create(const mla_pointer_t& p_Buffer, mla_size_t p_Position);

void mla_string_builder_destroy(mla_string_builder_t& p_Builder);
mla_string_t mla_string_builder_to_string(const mla_string_builder_t& p_Builder);

mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, const mla_string_t& p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_bool_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int8_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint8_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int16_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint16_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int32_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint32_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_int64_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_uint64_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_float_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_float_t p_Value, mla_size_t p_DecimalPlaces);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_double_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_double_t p_Value, mla_size_t p_DecimalPlaces);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_char_t p_Value);
mla_bool_t mla_string_builder_append(mla_string_builder_t& p_Builder, mla_platform_pointer_t p_Value);

#endif
