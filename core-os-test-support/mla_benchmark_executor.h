//
// Created by chris on 8/2/2025.
//

#ifndef MLA_BENCHMARK_EXECUTOR_H
#define MLA_BENCHMARK_EXECUTOR_H

#include "./Benchmark/mla_benchmark.h"

struct mla_benchmark_executor_t {

    mla_benchmark_t *benchmarks;
    mla_test_uint32_t capacity;
    mla_test_uint32_t count;
};

mla_benchmark_executor_t mla_benchmark_executor();
void mla_benchmark_executor_destroy(mla_benchmark_executor_t &executor);
void mla_benchmark_executor_run_all(mla_benchmark_executor_t &executor, mla_test_output_format_t output_format);
void mla_benchmark_executor_run(mla_benchmark_executor_t &executor, mla_test_uint32_t benchmark_number, mla_test_output_format_t output_format);
void mla_benchmark_executor_register(mla_benchmark_executor_t &executor, mla_benchmark_t &benchmark);

#endif
