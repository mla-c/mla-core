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

    // Initialize hash values
    mla_uint32_t h0 = 0x67452301;
    mla_uint32_t h1 = 0xEFCDAB89;
    mla_uint32_t h2 = 0x98BADCFE;
    mla_uint32_t h3 = 0x10325476;
    mla_uint32_t h4 = 0xC3D2E1F0;

    // Calculate padding
    mla_size_t original_length = input.size;
    mla_size_t bit_length = original_length * 8;
    mla_size_t padding_length = (original_length % MLA_SHA1_BLOCK_SIZE < 56)
                                     ? (56 - (original_length % MLA_SHA1_BLOCK_SIZE))
                                     : (120 - (original_length % MLA_SHA1_BLOCK_SIZE));
    mla_size_t total_length = original_length + padding_length + 8;

    // Create padded message
    mla_bytes_t padded = mla_bytes(total_length);
    mla_byte_t *padded_data = mla_bytes_get_data_for_writing(padded);

    if (padded_data == nullptr) {
        return mla_bytes_empty(); // Memory allocation failed
    }

    // Copy original data
    if (input.data != nullptr) {
        for (mla_size_t i = 0; i < original_length; i++) {
            padded_data[i] = input.data[i];
        }
    }

    // Add padding bit
    padded_data[original_length] = 0x80;

    // Zero padding
    for (mla_size_t i = original_length + 1; i < original_length + padding_length; i++) {
        padded_data[i] = 0x00;
    }

    // Add length in bits (big-endian 64-bit)
    for (mla_size_t i = 0; i < 8; i++) {
        padded_data[total_length - 8 + i] = (mla_byte_t)((bit_length >> ((7 - i) * 8)) & 0xFF);
    }

    // Process each 512-bit block
    for (mla_size_t block_start = 0; block_start < total_length; block_start += MLA_SHA1_BLOCK_SIZE) {
        mla_uint32_t w[80];

        // Prepare message schedule
        for (mla_size_t i = 0; i < 16; i++) {
            mla_size_t offset = block_start + (i * 4);
            w[i] = ((mla_uint32_t)padded_data[offset] << 24) |
                   ((mla_uint32_t)padded_data[offset + 1] << 16) |
                   ((mla_uint32_t)padded_data[offset + 2] << 8) |
                   ((mla_uint32_t)padded_data[offset + 3]);
        }

        for (mla_size_t i = 16; i < 80; i++) {
            w[i] = __mla_sha1_left_rotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        // Initialize working variables
        mla_uint32_t a = h0;
        mla_uint32_t b = h1;
        mla_uint32_t c = h2;
        mla_uint32_t d = h3;
        mla_uint32_t e = h4;

        // Main loop
        for (mla_size_t i = 0; i < 80; i++) {
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

        // Add this chunk's hash to result
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    // Produce final hash (big-endian)
    mla_bytes_t result = mla_bytes(MLA_SHA1_DIGEST_SIZE);
    mla_byte_t *result_data = mla_bytes_get_data_for_writing(result);

    if (result_data == nullptr) {
        return mla_bytes_empty(); // Memory allocation failed
    }

    for (mla_size_t i = 0; i < 4; i++) {
        result_data[i] = (mla_byte_t)((h0 >> (24 - i * 8)) & 0xFF);
        result_data[4 + i] = (mla_byte_t)((h1 >> (24 - i * 8)) & 0xFF);
        result_data[8 + i] = (mla_byte_t)((h2 >> (24 - i * 8)) & 0xFF);
        result_data[12 + i] = (mla_byte_t)((h3 >> (24 - i * 8)) & 0xFF);
        result_data[16 + i] = (mla_byte_t)((h4 >> (24 - i * 8)) & 0xFF);
    }

    return result;
}
