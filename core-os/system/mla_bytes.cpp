//
// Created by chris on 9/17/2025.
//

#include "mla_bytes.h"

mla_bytes_t mla_bytes_empty() {
    return {
        nullptr,
        0,
        mla_buffer_reference_noOwner()
    };
}

mla_bytes_t mla_bytes(mla_size_t p_Length) {

    mla_byte_t* buffer = reinterpret_cast<mla_byte_t*>(mla_malloc(p_Length));

    if (buffer == nullptr) {
        return mla_bytes_empty();
    }

    mla_memset(buffer, 0, p_Length);

    return {
        buffer,
        p_Length,
        mla_buffer_reference(buffer)
    };
}


mla_bytes_t mla_bytes_from_external_buffer(mla_byte_t* p_Data, const mla_size_t p_Size) {

    if (p_Data == nullptr || p_Size == 0) {
        return mla_bytes_empty();
    }

    return {
        p_Data,
        p_Size,
        mla_buffer_reference_noOwner()
    };

}

// This function creates bytes from a buffer and takes ownership of the buffer
// You must not free the buffer after calling this function
mla_bytes_t mla_bytes_from_buffer_with_ownership(mla_byte_t* p_Data, const mla_size_t p_Size) {

    if (p_Data == nullptr) {
        return mla_bytes_empty();
    }

    return {
        p_Data,
        p_Size,
        mla_buffer_reference(p_Data)
    };
}

mla_bytes_t mla_bytes_copy(const mla_bytes_t& p_Bytes) {

    if (p_Bytes.size == 0 || p_Bytes.data == nullptr) {
        return mla_bytes_empty();
    }

    mla_byte_t* buffer = reinterpret_cast<mla_byte_t*>(mla_malloc(p_Bytes.size));

    if (buffer == nullptr) {
        return mla_bytes_empty();
    }

    mla_memcpy(buffer, p_Bytes.data, p_Bytes.size);

    return {
        buffer,
        p_Bytes.size,
        mla_buffer_reference(buffer)
    };

}

mla_byte_t* mla_bytes_get_data_for_writing(mla_bytes_t& p_Bytes) {

    return const_cast<mla_byte_t*>(p_Bytes.data);

}

void mla_bytes_destroy(mla_bytes_t& p_Bytes) {

    if (p_Bytes.data == nullptr) {
        return; // Nothing to free
    }

    p_Bytes.data = nullptr; // Clear the pointer
    p_Bytes.dataOwner = mla_buffer_reference_noOwner();
    p_Bytes.size = 0; // Reset the length

}