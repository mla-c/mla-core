//
// Created by chris on 1/9/2026.
//

#ifndef MLA_C_TEST_MLA_TEST_UTILS_H
#define MLA_C_TEST_MLA_TEST_UTILS_H

#include "mla_test_data_types.h"

// Helper to copy memory (non-overlapping)
mla_test_pointer_t mla_test_memcpy(mla_test_pointer_t dest, const mla_test_pointer_t src, mla_test_uint32_t n);

// Helper to copy memory (handles overlapping)
mla_test_pointer_t mla_test_memmove(mla_test_pointer_t dest, const mla_test_pointer_t src, mla_test_uint32_t n);

// Helper to set memory to a value
mla_test_pointer_t mla_test_memset(mla_test_pointer_t ptr, mla_test_int32_t value, mla_test_uint32_t n);

// Helper to compare strings
mla_test_int32_t mla_test_strcmp(const mla_test_char_t* str1, const mla_test_char_t* str2);

mla_test_uint32_t mla_test_strlen(const mla_test_char_t* str);

// Helper to convert uint32 to string
mla_test_uint32_t mla_uint32_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint32_t value);

// Helper to convert int16 to string
mla_test_uint32_t mla_int16_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_int16_t value);

// Helper to convert uint16 to string
mla_test_uint32_t mla_uint16_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint16_t value);

// Helper to convert int32 to string
mla_test_uint32_t mla_int32_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_int32_t value);

// Helper to convert int64 to string
mla_test_uint32_t mla_int64_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_int64_t value);

// Helper to convert uint64 to string
mla_test_uint32_t mla_uint64_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint64_t value);

// Helper to convert float to string
mla_test_uint32_t mla_float_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_float_t value, mla_test_uint32_t precision);

// Helper to convert double to string
mla_test_uint32_t mla_double_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_double_t value, mla_test_uint32_t precision);

// Helper to convert pointer to hex string
mla_test_uint32_t mla_pointer_to_string(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, void* ptr);

// Helper to append a string to a buffer
mla_test_uint32_t mla_test_strcat(mla_test_char_t* buffer, mla_test_uint32_t buffer_size, mla_test_uint32_t offset, const mla_test_char_t* str);

#endif