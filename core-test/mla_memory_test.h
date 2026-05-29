//
// Created by chris on 12/18/2025.
//

#ifndef MLA_MEMORY_TEST_H
#define MLA_MEMORY_TEST_H

#include "../base-lib/core/mla_data_types.h"
#include "../base-lib/test-support/mla_benchmark_executor.h"

void MlaMemset16KbBenchmark() {

    const mla_size_t size = 16 * 1024; // 16 KB
    mla_byte_t* buffer = (mla_byte_t*)mla_platform_malloc(size);

    mla_memset(buffer, 0, size);

    // Prevent optimization from removing the operation
    volatile mla_byte_t temp = buffer[0];
    (void)temp; // Prevent unused variable warning

    mla_platform_free(buffer);
}

void NativeMemset16KbBenchmark() {

    const mla_size_t size = 16 * 1024; // 16 KB
    mla_byte_t* buffer = (mla_byte_t*)mla_test_malloc(size);

    mla_test_memset(buffer, 0, size);

    // Prevent optimization from removing the operation
    volatile mla_byte_t temp = buffer[0];
    (void)temp; // Prevent unused variable warning

    mla_test_free(buffer);
}

void RegisterMemoryBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("MlaMemset16Kb", benchmark_category, MlaMemset16KbBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("NativeMemset16Kb", benchmark_category, NativeMemset16KbBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

}

#endif
