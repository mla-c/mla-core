//
// Created by chris on 6/22/2026.
//

#include "mla_test_runner.h"
#include "mla_test_utils.h"

int mla_test_runner_run(mla_test_executor_t& testExecutor, mla_benchmark_executor_t& benchmarkExecutor, mla_test_bool_t runTest, mla_test_bool_t runBenchmark, mla_test_output_format_t benchmarkOutputFormat,
                         mla_test_uint32_t p_AllocationFailureSeed, mla_test_uint32_t p_AllocationFailureSeedCount) {

    // Run the tests
    ////////////////////////////////////////

    mla_test_int32_t l_FailedTest = 0;
    mla_test_bool_t* l_SuccessMap = nullptr;

#if (!defined(mla_test_global_feature_flag_test_memory) || (mla_test_global_feature_flag_test_memory == 1))

    if (p_AllocationFailureSeed > 0) {
        // Fixed seed mode: only run allocation failure tests with this seed
        mla_test_print("Fixed Seed Mode: Running only allocation failure tests...\n", 58);
        l_FailedTest = mla_test_executor_run_all_tests_with_allocation_failure(testExecutor, p_AllocationFailureSeed);

        mla_test_executor_destroy(testExecutor);
        mla_benchmark_executor_destroy(benchmarkExecutor);
        return mla_s_cast<int>(l_FailedTest);
    }

    if (p_AllocationFailureSeedCount > 0) {
        // Seed count mode: run regular tests first, then seed-based tests only for passing ones
        l_SuccessMap = mla_s_cast<mla_test_bool_t *>(mla_test_malloc(sizeof(mla_test_bool_t) * testExecutor.count));
        for (mla_test_uint32_t i = 0; i < testExecutor.count; i++) {
            l_SuccessMap[i] = false;
        }

        mla_test_print("Seed Count Mode: Running regular tests first...\n", 48);
        l_FailedTest = mla_test_executor_run_all_tests(testExecutor, l_SuccessMap);

        mla_test_print("Tests completed with ", 21);
        mla_test_char_t buffer[12];
        mla_test_uint32_t strLength = mla_test_uint32_to_string(buffer, sizeof(buffer), mla_s_cast<mla_test_uint32_t>(l_FailedTest));
        mla_test_print(buffer, strLength);
        mla_test_print(" failed tests\n", 14);

        l_FailedTest += mla_test_executor_run_all_tests_with_generated_allocation_failures(testExecutor, p_AllocationFailureSeedCount, l_SuccessMap);

        if (l_SuccessMap != nullptr) {
            mla_test_free(l_SuccessMap);
        }
    } else
#endif
    {
        // Regular mode
        if (runTest) {
            mla_test_print("Running Tests...\n", 17);
            l_FailedTest = mla_test_executor_run_all_tests(testExecutor);
            //l_FailedTest = mla_test_executor_run_test(testExecutor, 389);

            mla_test_print("Tests completed with ", 21);
            mla_test_char_t buffer[12];
            mla_test_uint32_t strLength = mla_test_uint32_to_string(buffer, sizeof(buffer), mla_s_cast<mla_test_uint32_t>(l_FailedTest));
            mla_test_print(buffer, strLength);
            mla_test_print(" failed tests\n", 14);
        }
    }

    // Running benchmarks
    ////////////////////////////////////////

    mla_test_print("\n", 1);

    if (runBenchmark && p_AllocationFailureSeed == 0 && p_AllocationFailureSeedCount == 0) {

        if (benchmarkOutputFormat == mla_test_output_format_text) {
            mla_test_print("Running Benchmarks...\n\n", 23);
        }

        mla_benchmark_executor_run_all(benchmarkExecutor, benchmarkOutputFormat);
        //mla_benchmark_executor_run(benchmarkExecutor, 90, benchmarkOutputFormat);
        //mla_benchmark_executor_run(benchmarkExecutor, 19, benchmarkOutputFormat);
        //mla_benchmark_executor_run(benchmarkExecutor, 82, benchmarkOutputFormat);

        if (benchmarkOutputFormat == mla_test_output_format_text) {
            mla_test_print("\nBenchmarks completed\n", 22);
        }
    }

    // Clean up resources
    mla_test_executor_destroy(testExecutor);
    mla_benchmark_executor_destroy(benchmarkExecutor);

    return mla_s_cast<int>(l_FailedTest);

}
