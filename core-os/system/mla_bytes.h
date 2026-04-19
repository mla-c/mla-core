//
// Created by chris on 9/17/2025.
//

#ifndef COREOS_MLA_BYTE_ARRAY_H
#define COREOS_MLA_BYTE_ARRAY_H

#include "../mla_data_types.h"
#include "mla_buffer.h"
#include "mla_string.h"

struct mla_bytes_t {
    mla_pointer_t heap_data;
    mla_size_t size;
};


mla_bytes_t mla_bytes_empty();
mla_bytes_t mla_bytes(mla_size_t p_Length);
mla_bytes_t mla_bytes_from_external_buffer(mla_pointer_t p_Data, const mla_size_t p_Size);

mla_string_t mla_bytes_to_base64(const mla_bytes_t& p_Bytes);
mla_bytes_t mla_bytes_from_base64(const mla_string_t& p_Base64String);

mla_string_t mla_bytes_to_string(const mla_bytes_t& p_Bytes);
mla_bytes_t mla_bytes_from_string(const mla_string_t& p_String);

mla_bytes_t mla_bytes_copy(const mla_bytes_t& p_Bytes);

const mla_byte_t* mla_bytes_get_data_readonly(const mla_bytes_t& p_Bytes);
mla_byte_t* mla_bytes_get_data_for_writing(mla_bytes_t& p_Bytes);
mla_size_t mla_bytes_length(const mla_bytes_t& p_Bytes);

void mla_bytes_destroy(mla_bytes_t& p_Bytes);

mla_bool_t mla_bytes_is_empty(const mla_bytes_t& p_Bytes);

#endif