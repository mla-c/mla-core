//
// Created by chris on 11/3/2025.
//

#include "mla_id.h"
#include "../task/mla_mutx.h"

void __mla_string_from_uint32_hex(mla_char_t* buffer, mla_uint32_t value, mla_size_t padding) {
    // Convert to hex from right to left
    for (mla_size_t i = padding; i > 0; i--) {
        mla_uint32_t nibble = value & 0xF;
        if (nibble < 10) {
            buffer[i - 1] = (mla_char_t)('0' + nibble);
        } else {
            buffer[i - 1] = (mla_char_t)('a' + (nibble - 10));
        }
        value >>= 4;
    }
}

void __mla_string_from_uint16_hex(mla_char_t* buffer, mla_uint16_t value, mla_size_t padding) {
    // Convert to hex from right to left
    for (mla_size_t i = padding; i > 0; i--) {
        mla_uint16_t nibble = value & 0xF;
        if (nibble < 10) {
            buffer[i - 1] = (mla_char_t)('0' + nibble);
        } else {
            buffer[i - 1] = (mla_char_t)('a' + (nibble - 10));
        }
        value >>= 4;
    }
}

mla_string_t mla_generate_uuid() {

    // Get random data source
    mla_uint32_t rand1 = mla_random_uint32();
    mla_uint32_t rand2 = mla_random_uint32();
    mla_uint32_t rand3 = mla_random_uint32();
    mla_uint32_t rand4 = mla_random_uint32();

    // Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // Set version (4) and variant bits
    mla_uint16_t time_hi = (rand2 & 0x0FFF) | 0x4000;  // Version 4
    mla_uint16_t clock_seq = (rand3 & 0x3FFF) | 0x8000; // Variant 10

    // Build UUID directly in buffer: 36 chars + null terminator
    mla_char_t* buffer = mla_create_char_array(37);

    if (buffer == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    __mla_string_from_uint32_hex(buffer, rand1, 8);
    buffer[8] = '-';
    __mla_string_from_uint16_hex(buffer + 9, (rand2 >> 16) & 0xFFFF, 4);
    buffer[13] = '-';
    __mla_string_from_uint16_hex(buffer + 14, time_hi, 4);
    buffer[18] = '-';
    __mla_string_from_uint16_hex(buffer + 19, clock_seq, 4);
    buffer[23] = '-';
    __mla_string_from_uint16_hex(buffer + 24, (rand3 >> 16) & 0xFFFF, 4);
    __mla_string_from_uint32_hex(buffer + 28, rand4, 8);
    buffer[36] = '\0';

    return mla_string_from_buffer_without_ownership(buffer, 36);
}



mla_string_t mla_generate_runtime_id() {

    // Static generator state
    struct mla_runtime_id_generator {
        mla_mutex_t mutex;
        mla_uint64_t counter;
    };

    static mla_runtime_id_generator g_runtime_id_generator = {
        mla_mutex_create("RuntimeID"),
        0
    };

    // Code

    if (!mla_mutex_lock(g_runtime_id_generator.mutex)) {
        return mla_string_empty();
    }

    g_runtime_id_generator.counter++;
    mla_uint64_t id = g_runtime_id_generator.counter;

    mla_mutex_unlock(g_runtime_id_generator.mutex);

    return mla_string_from_uint64(id);
}

mla_uint32_t mla_random_uint32() {

    static mla_uint32_t seed = 0;
    static mla_bool_t initialized = false;

    // Initialize seed on first call using system time
    if (!initialized) {
        // Use combination of time and memory address for better entropy
        seed = ((mla_uint32_t)(uintptr_t)&seed) ^ ((mla_uint32_t)(uintptr_t)&initialized);
        initialized = true;
    }

    // LCG formula: next = (a * seed + c) mod m
    // Using constants from Numerical Recipes
    seed = seed * 1664525u + 1013904223u;
    return seed;
}
