//
// Created by christian on 8/4/25.
//

#ifndef COREOS_MLA_HASH_H
#define COREOS_MLA_HASH_H

#include "../mla_data_types.h"

struct mla_int32_hash_t {

    static mla_size_t hash(const mla_int32_t& key) {
        mla_size_t x = (mla_size_t)key;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }
};


struct mla_int16_hash_t {

    static mla_size_t hash(const mla_int16_t& key) {

        mla_size_t x = (mla_size_t)key;
        x = ((x >> 8) ^ x) * 0x45d9f3b;
        x = ((x >> 8) ^ x) * 0x45d9f3b;
        x = (x >> 8) ^ x;
        return x;

    }
};

#endif
