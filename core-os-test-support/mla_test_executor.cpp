//
// Created by christian on 9/10/25.
//

#include "mla_test_executor.h"
#include "mla_test_utils.h"

mla_test_executor_t mla_test_executor(mla_test_uint32_t p_MaxTests) {

    mla_test_executor_t executor;
    executor.max_tests = p_MaxTests;
    executor.tests = (mla_test_t*) mla_test_malloc(sizeof(mla_test_t) * p_MaxTests);
    for (mla_test_uint32_t i = 0; i < p_MaxTests; ++i) {
        executor.tests[i] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    }
    return executor;
}

void mla_test_executor_destroy(mla_test_executor_t &executor) {
    for (mla_test_uint32_t i = 0; i < executor.max_tests; ++i) {
        if (executor.tests[i].name) {
            mla_test_destroy(executor.tests[i]);
        }
    }
    mla_test_free(executor.tests);
    executor.tests = nullptr;
    executor.max_tests = 0;
}

mla_test_int32_t mla_test_executor_run_test(mla_test_executor_t &executor, mla_test_uint32_t test_number) {

    mla_test_uint32_t test_index = test_number - 1; // Convert to zero-based index

    if (test_index >= executor.max_tests || executor.tests[test_index].name == nullptr) {
        return -1; // Invalid test index
    }

    char buffer[12];
    mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), test_index + 1);
    mla_test_print(buffer, strLength);
    mla_test_print("). ", 3);

    if (mla_test_run(executor.tests[test_index])) {
        return 0; // Test passed
    } else {
        return 1; // Test failed
    }

}

mla_test_int32_t mla_test_executor_run_all_tests(mla_test_executor_t &executor) {

    mla_test_int32_t failed_tests = 0;

    for (mla_test_uint32_t i = 0; i < executor.max_tests; ++i) {
        if (executor.tests[i].name != nullptr) {

            char buffer[12];
            mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), i + 1);
            mla_test_print(buffer, strLength);
            mla_test_print("). ", 3);

            if (!mla_test_run(executor.tests[i])) {
                failed_tests++;
            }
        }
    }

    return failed_tests;
}

void mla_test_executor_register_test(mla_test_executor_t &executor, mla_test_t &test) {
    if (test.name == nullptr) {
        return;
    }

    // Find the first empty slot in the tests array
    for (mla_test_uint32_t i = 0; i < executor.max_tests; ++i) {
        if (executor.tests[i].name == nullptr) {
            executor.tests[i] = test;
            return;
        }
    }
}
