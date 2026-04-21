//
// Created by chris on 11/13/2025.
//

#include "mla_sha1.h"
#include "../utils/mla_endian_utils.h"

#define MLA_SHA1_BLOCK_SIZE 64
#define MLA_SHA1_DIGEST_SIZE 20

mla_uint32_t leftRotate32bits(mla_uint32_t n, unsigned int rotate) {
    rotate &= 31;
    return (n << rotate) | (n >> ((32 - rotate) & 31));
}

mla_bytes_t mla_sha1(const mla_bytes_t &input) {

    // Initial SHA-1 state
    mla_uint32_t h0 = 0x67452301;
    mla_uint32_t h1 = 0xEFCDAB89;
    mla_uint32_t h2 = 0x98BADCFE;
    mla_uint32_t h3 = 0x10325476;
    mla_uint32_t h4 = 0xC3D2E1F0;

    // Original message size
    mla_uint64_t msg_size = input.size;


    mla_uint64_t rem = msg_size % 64; // block size = 64 bytes
    mla_uint64_t pad_len = (rem < 56) ? (56 - rem) : (56 + 64 - rem);
    mla_uint64_t padded_message_size = msg_size + pad_len + 8; // +8 for length

    // Allocate padded message
    mla_bytes_t padded_message = mla_bytes((mla_size_t)padded_message_size);

    mla_byte_t *padded = mla_bytes_get_data_for_writing(padded_message);
    const mla_byte_t * input_data = mla_bytes_get_data_readonly(input);

    if (padded == nullptr)
        return mla_bytes_empty();

    // Zero initialize
    for (mla_uint64_t k = 0; k < padded_message_size; ++k)
        padded[k] = 0x00;

    // Copy original input
    if (input_data && msg_size > 0)
        mla_memcpy(padded, input_data, (mla_size_t)msg_size);

    // Append 0x80
    padded[msg_size] = 0x80;

    // Append original length in bits (big-endian)
    mla_uint64_t bit_len = msg_size * 8;
    for (mla_uint8_t j = 0; j < 8; ++j) {
        padded[padded_message_size - 8 + j] =
            (mla_byte_t)((bit_len >> (56 - 8 * j)) & 0xFF);
    }

    // Process each 512-bit block
    for (mla_uint64_t chunk = 0; chunk < padded_message_size / 64; ++chunk) {

        mla_uint32_t W[80];

        // First 16 words (big-endian)
        for (mla_uint32_t t = 0; t < 16; ++t) {
            mla_uint64_t idx = chunk * 64 + t * 4;
            W[t] = (padded[idx] << 24) |
                   (padded[idx + 1] << 16) |
                   (padded[idx + 2] << 8) |
                   (padded[idx + 3]);
        }

        // Extend W[16..79]
        for (mla_uint32_t t = 16; t < 80; ++t)
            W[t] = leftRotate32bits(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);

        // Initialize working variables
        mla_uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        // 80 rounds
        for (mla_uint32_t t = 0; t < 80; ++t) {
            mla_uint32_t F, K;

            if (t < 20) {
                F = (b & c) | ((~b) & d);
                K = 0x5A827999;
            } else if (t < 40) {
                F = b ^ c ^ d;
                K = 0x6ED9EBA1;
            } else if (t < 60) {
                F = (b & c) | (b & d) | (c & d);
                K = 0x8F1BBCDC;
            } else {
                F = b ^ c ^ d;
                K = 0xCA62C1D6;
            }

            mla_uint32_t temp = leftRotate32bits(a, 5) + F + e + K + W[t];
            e = d;
            d = c;
            c = leftRotate32bits(b, 30);
            b = a;
            a = temp;
        }

        // Add the compressed chunk to the running hash
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    // Output SHA-1 digest
    mla_bytes_t signature = mla_bytes(20);

    mla_byte_t *out = mla_bytes_get_data_for_writing(signature);

    if (out == nullptr)
        return mla_bytes_empty();

    for (mla_uint8_t j = 0; j < 4; ++j) {
        out[j + 0]  = (h0 >> (24 - 8*j)) & 0xFF;
        out[j + 4]  = (h1 >> (24 - 8*j)) & 0xFF;
        out[j + 8]  = (h2 >> (24 - 8*j)) & 0xFF;
        out[j + 12] = (h3 >> (24 - 8*j)) & 0xFF;
        out[j + 16] = (h4 >> (24 - 8*j)) & 0xFF;
    }

    return signature;
}


