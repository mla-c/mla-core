//
// Created by christian on 8/4/25.
//

#ifndef MLA_HASH_H
#define MLA_HASH_H

#include "../mla_data_types.h"

struct mla_int32_hash_t {

    static mla_size_t hash(const mla_int32_t& key) {
        mla_size_t x = static_cast<mla_size_t>(key);
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }
};


struct mla_int16_hash_t {

    static mla_size_t hash(const mla_int16_t& key) {

        mla_size_t x = static_cast<mla_size_t>(key);
        x = ((x >> 8) ^ x) * 0x45d9f3b;
        x = ((x >> 8) ^ x) * 0x45d9f3b;
        x = (x >> 8) ^ x;
        return x;

    }
};


/**
 * Highly optimized string hash function for hash maps
 * Uses FNV-1a algorithm optimized for the platform
 * Provides excellent distribution while being very fast
 * 
 * @param data Pointer to the string data (must not be null if length > 0)
 * @param length Length of the string in bytes
 * @return Hash value
 */
inline mla_size_t mla_string_hash(const char* data, mla_size_t length) {
    // FNV-1a 32-bit constants
    // Note: mla_size_t is defined as mla_uint32_t in mla_data_types.h (always 32-bit)
    mla_size_t hash = 2166136261U;
    const mla_size_t prime = 16777619U;
    
    // Null pointer check for safety
    if (data == nullptr && length > 0) {
        return hash; // Return initial hash value for invalid input
    }
    
    // Process string byte by byte - compiler will often vectorize this
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
    for (mla_size_t i = 0; i < length; ++i) {
        hash ^= bytes[i];
        hash *= prime;
    }
    
    return hash;
}

#endif
