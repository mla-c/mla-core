//
// Created by chris on 11/13/2025.
//

#include "mla_sha1.h"
#include "../utils/mla_endian_utils.h"

// SHA-1 constants
#define MLA_SHA1_BLOCK_SIZE 64
#define MLA_SHA1_DIGEST_SIZE 20

mla_uint32_t __mla_sha1_left_rotate(mla_uint32_t value, mla_uint32_t count) {
    return (value << count) | (value >> (32 - count));
}

mla_bytes_t mla_sha1(const mla_bytes_t &input) {

    return mla_bytes_empty();
}
