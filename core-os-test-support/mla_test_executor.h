//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_TEST_EXECUTOR_H
#define COREOS_MLA_TEST_EXECUTOR_H

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
mla_test_int32_t mla_test_executor_run_all_tests(mla_test_executor_t &executor);
void mla_test_executor_register_test(mla_test_executor_t &executor, mla_test_t &test);

#endif
