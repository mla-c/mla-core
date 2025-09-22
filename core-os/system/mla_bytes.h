//
// Created by chris on 9/17/2025.
//

#ifndef COREOS_MLA_BYTE_ARRAY_H
#define COREOS_MLA_BYTE_ARRAY_H

#include "../mla_data_types.h"
#include "mla_buffer.h"
#include "mla_string.h"

struct mla_bytes_t {
    const mla_byte_t* data;
    mla_size_t size;
    mla_buffer_reference_t dataOwner;
};


mla_bytes_t mla_bytes_empty();
mla_bytes_t mla_bytes(mla_size_t p_Length);
mla_bytes_t mla_bytes_from_external_buffer(mla_byte_t* p_Data, const mla_size_t p_Size);
// This function creates bytes from a buffer and takes ownership of the buffer
// You must not free the buffer after calling this function
mla_bytes_t mla_bytes_from_buffer_with_ownership(mla_byte_t* p_Data, const mla_size_t p_Size);

mla_string_t mla_bytes_to_base64(const mla_bytes_t& p_Bytes);
mla_bytes_t mla_bytes_from_base64(const mla_string_t& p_Base64String);

mla_bytes_t mla_bytes_copy(const mla_bytes_t& p_Bytes);

mla_byte_t* mla_bytes_get_data_for_writing(mla_bytes_t& p_Bytes);

void mla_bytes_destroy(mla_bytes_t& p_Bytes);

#endif