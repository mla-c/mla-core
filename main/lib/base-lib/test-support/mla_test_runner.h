//
// Created by chris on 6/22/2026.
//

#ifndef MLA_TEST_RUNNER_H
#define MLA_TEST_RUNNER_H

#include "mla_test_executor.h"
#include "mla_benchmark_executor.h"


int mla_test_runner_run(mla_test_executor_t& testExecutor, mla_benchmark_executor_t& benchmarkExecutor, mla_test_bool_t runTest, mla_test_bool_t runBenchmark, mla_test_output_format_t benchmarkOutputFormat,
                        mla_test_uint32_t p_AllocationFailureSeed = 0, mla_test_uint32_t p_AllocationFailureSeedCount = 0);

#endif //MLA_TEST_RUNNER_H
