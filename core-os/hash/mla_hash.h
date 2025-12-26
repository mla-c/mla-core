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


/**
 * Highly optimized string hash function for hash maps
 * Uses an optimized variant combining FNV-1a principles with performance optimizations
 * Provides excellent distribution while being very fast
 * 
 * @param data Pointer to the string data
 * @param length Length of the string in bytes
 * @return Hash value
 */
inline mla_size_t mla_string_hash(const char* data, mla_size_t length) {
    // Use 64-bit or 32-bit based on platform
    #if defined(__LP64__) || defined(_WIN64)
        // 64-bit optimized constants
        mla_size_t hash = 14695981039346656037ULL;
        const mla_size_t prime = 1099511628211ULL;
    #else
        // 32-bit optimized constants  
        mla_size_t hash = 2166136261U;
        const mla_size_t prime = 16777619U;
    #endif
    
    // Process string byte by byte - compiler will often vectorize this
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
    for (mla_size_t i = 0; i < length; ++i) {
        hash ^= bytes[i];
        hash *= prime;
    }
    
    return hash;
}

#endif
