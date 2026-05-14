//
// Created by chris on 8/2/2025.
//

#ifndef MLA_MLA_TEST_EXECUTOR_H
#define MLA_MLA_TEST_EXECUTOR_H

#include "mla_test_data_types.h"
#include "Test/mla_test.h"

struct mla_test_executor_t {
    mla_test_uint32_t capacity;
    mla_test_uint32_t count;
    mla_test_t* tests;
};


mla_test_executor_t mla_test_executor();
void mla_test_executor_destroy(mla_test_executor_t &executor);
mla_test_int32_t mla_test_executor_run_test(mla_test_executor_t &executor, mla_test_uint32_t test_number);
mla_test_int32_t mla_test_executor_run_all_tests(mla_test_executor_t &executor, mla_test_bool_t* p_SuccessMap = nullptr);
void mla_test_executor_register_test(mla_test_executor_t &executor, mla_test_t &test);

#if (mla_test_global_feature_flag_test_memory == 1)
mla_test_uint32_t mla_test_generate_seed();
mla_test_int32_t mla_test_executor_run_all_tests_with_allocation_failure(mla_test_executor_t &executor, mla_test_uint32_t p_Seed, const mla_test_bool_t* p_SuccessMap = nullptr);
mla_test_int32_t mla_test_executor_run_test_with_allocation_failure(mla_test_executor_t &executor, mla_test_uint32_t p_TestNumber, mla_test_uint32_t p_Seed);
mla_test_int32_t mla_test_executor_run_all_tests_with_generated_allocation_failures(mla_test_executor_t &executor, mla_test_uint32_t p_SeedCount, const mla_test_bool_t* p_SuccessMap = nullptr);
#endif

#endif
