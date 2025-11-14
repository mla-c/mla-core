//
// Created by chris on 11/13/2025.
//

#include "mla_sha1.h"
#include "../utils/mla_endian_utils.h"
#include "../system/mla_memory.h"

// SHA-1 constants
#define MLA_SHA1_BLOCK_SIZE 64
#define MLA_SHA1_DIGEST_SIZE 20

mla_uint32_t __mla_sha1_left_rotate(mla_uint32_t value, mla_uint32_t count) {
    return (value << count) | (value >> (32 - count));
}

void mla_sha1_transform(mla_uint32_t *state, const mla_byte_t *block) {
    mla_uint32_t w[80];

    for (int i = 0; i < 16; i++) {
        w[i] = mla_bytes_to_uint32_be(&block[i * 4]);
    }

    for (int i = 16; i < 80; i++) {
        w[i] = __mla_sha1_left_rotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    mla_uint32_t a = state[0];
    mla_uint32_t b = state[1];
    mla_uint32_t c = state[2];
    mla_uint32_t d = state[3];
    mla_uint32_t e = state[4];

    for (int i = 0; i < 80; i++) {
        mla_uint32_t f, k;

        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        mla_uint32_t temp = __mla_sha1_left_rotate(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = __mla_sha1_left_rotate(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}


mla_bytes_t mla_sha1(const mla_bytes_t &input) {
    mla_uint32_t h[5] = {
            0x67452301,
            0xEFCDAB89,
            0x98BADCFE,
            0x10325476,
            0xC3D2E1F0
    };

    mla_uint64_t total_bits = input.size * 8;
    mla_size_t padded_size = input.size + 1;
    while (padded_size % 64 != 56) {
        padded_size++;
    }

    mla_bytes_t padded_input = mla_bytes(padded_size + 8);
    mla_byte_t* padded_data_writer = mla_bytes_get_data_for_writing(padded_input);

    mla_memcpy(padded_data_writer, input.data, input.size);
    padded_data_writer[input.size] = 0x80;

    for (mla_size_t i = input.size + 1; i < padded_size; i++) {
        padded_data_writer[i] = 0;
    }

    mla_uint64_be_to_bytes(total_bits, &padded_data_writer[padded_size]);

    for (mla_size_t i = 0; i < padded_input.size; i += 64) {
        mla_sha1_transform(h, &padded_data_writer[i]);
    }

    mla_bytes_t output = mla_bytes(MLA_SHA1_DIGEST_SIZE);
    mla_byte_t* output_data_writer = mla_bytes_get_data_for_writing(output);

    for (int i = 0; i < 5; i++) {
        mla_uint32_be_to_bytes(h[i], &output_data_writer[i * 4]);
    }
    mla_bytes_destroy(padded_input);
    return output;
}
