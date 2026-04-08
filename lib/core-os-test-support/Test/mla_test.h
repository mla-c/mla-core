//
// Created by chris on 8/2/2025.
//

#ifndef MLA_TEST_H
#define MLA_TEST_H

#include "../mla_test_data_types.h"
#include "../../core-os/mla_data_types.h"

struct mla_test_result_t {
    mla_test_bool_t success;
    mla_test_char_t *message;
    mla_test_uint64_t allocated_memory;
    mla_test_bool_t block_memory_allocations;
};

extern mla_test_result_t current_test_result;
extern mla_test_bool_t g_mla_test_run_forced_allocation_failure_pass;

struct mla_test_t {
    const mla_test_char_t *name;
    const mla_test_char_t *category;
    void (*run)(void);
    void (*setUp)(void);
    void (*tearDown)(void);
};

mla_test_t mla_test(const char *name, const char *category,
                    void (*run)(void),
                    void (*setUp)(void) = nullptr,
                    void (*tearDown)(void) = nullptr);
void mla_test_destroy(mla_test_t &test);
mla_test_bool_t mla_test_run(mla_test_t &test);

void mla_check_assert_fail(const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_true(mla_test_bool_t p_Condition, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_false(mla_test_bool_t p_Condition, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_bool_t p_Actual, mla_test_bool_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_bool_t p_Actual, mla_test_bool_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_char_t p_Actual, mla_test_char_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_char_t p_Actual, mla_test_char_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_int8_t p_Actual, mla_test_int8_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_int8_t p_Actual, mla_test_int8_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_uint8_t p_Actual, mla_test_uint8_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_uint8_t p_Actual, mla_test_uint8_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_int16_t p_Actual, mla_test_int16_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_int16_t p_Actual, mla_test_int16_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_uint16_t p_Actual, mla_test_uint16_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_uint16_t p_Actual, mla_test_uint16_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_int32_t p_Actual, mla_test_int32_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_int32_t p_Actual, mla_test_int32_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_uint32_t p_Actual, mla_test_uint32_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_uint32_t p_Actual, mla_test_uint32_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_int64_t p_Actual, mla_test_int64_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_int64_t p_Actual, mla_test_int64_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_uint64_t p_Actual, mla_test_uint64_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_uint64_t p_Actual, mla_test_uint64_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_float_t p_Actual, mla_test_float_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_float_t p_Actual, mla_test_float_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_double_t p_Actual, mla_test_double_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_double_t p_Actual, mla_test_double_t p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(mla_test_char_t *p_Actual, mla_test_char_t *p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(mla_test_char_t *p_Actual, mla_test_char_t *p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(void *p_Actual, void *p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(void *p_Actual, void *p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_equal(const void *p_Actual, const void *p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_equal(const void *p_Actual, const void *p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_null(void *p_Pointer, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_null(void *p_Pointer, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_null(const void *p_Pointer, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);
void mla_check_assert_not_null(const void *p_Pointer, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0);

template <typename T>
void mla_check_struct_assert_equal(const T& p_Actual, const T& p_Expected, const mla_test_char_t *p_Message = nullptr, mla_test_int16_t p_Line = 0) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_int32_t offset = 0;

        // Copy "Assertion failed at line "
        const mla_test_char_t* prefix = "Assertion failed at line ";
        while (*prefix && offset < 4095) {
            l_Result[offset++] = *prefix++;
        }

        // Convert line number to string
        mla_test_int16_t line = p_Line;
        mla_test_char_t line_buffer[10];
        mla_test_int32_t line_index = 0;
        if (line == 0) {
            line_buffer[line_index++] = '0';
        } else {
            mla_test_int16_t temp = line;
            mla_test_int32_t digit_count = 0;
            while (temp > 0) {
                temp /= 10;
                digit_count++;
            }
            for (mla_test_int32_t i = digit_count - 1; i >= 0; i--) {
                line_buffer[i] = '0' + (line % 10);
                line /= 10;
            }
            line_index = digit_count;
        }
        for (mla_test_int32_t i = 0; i < line_index && offset < 4095; i++) {
            l_Result[offset++] = line_buffer[i];
        }

        // Copy ": Expected equal struct, but got different struct."
        const mla_test_char_t* middle = ": Expected equal struct, but got different struct.";
        while (*middle && offset < 4095) {
            l_Result[offset++] = *middle++;
        }

        // Append custom message if provided
        if (p_Message) {
            l_Result[offset++] = ' ';
            while (*p_Message && offset < 4095) {
                l_Result[offset++] = *p_Message++;
            }
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}




#define assert_fail(p_Message) mla_check_assert_fail(p_Message, __LINE__)
#define assert_true(p_Condition, p_Message) mla_check_assert_true(p_Condition, p_Message, __LINE__)
#define assert_false(p_Condition, p_Message) mla_check_assert_false(p_Condition, p_Message, __LINE__)
#define assert_equal(p_Actual, p_Expected, p_Message) mla_check_assert_equal(p_Actual, p_Expected, p_Message, __LINE__)
#define assert_struct_equal(T, p_Actual, p_Expected, p_Message) mla_check_struct_assert_equal<T>(p_Actual, p_Expected, p_Message, __LINE__)
#define assert_not_equal(p_Actual, p_Expected, p_Message) mla_check_assert_not_equal(p_Actual, p_Expected, p_Message, __LINE__)
#define assert_null(p_Condition, p_Message) mla_check_assert_null(p_Condition, p_Message, __LINE__)
#define assert_not_null(p_Condition, p_Message) mla_check_assert_not_null(p_Condition, p_Message, __LINE__)

#define test_category mla_test__FILENAME_ONLY__

#endif
