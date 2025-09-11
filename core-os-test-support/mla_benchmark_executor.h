//
// Created by chris on 8/2/2025.
//

#ifndef COREOSMLA_BENCHMARK_EXECUTOR_H
#define COREOSMLA_BENCHMARK_EXECUTOR_H

#include "./Benchmark/mla_benchmark.h"

struct mla_benchmark_executor_t {

    mla_benchmark_t *benchmarks;
    mla_test_uint32_t max_benchmarks;
};

mla_benchmark_executor_t mla_benchmark_executor(mla_test_uint32_t p_MaxBenchmarks = 100);
void mla_benchmark_executor_destroy(mla_benchmark_executor_t &executor);
void mla_benchmark_executor_run_all(mla_benchmark_executor_t &executor);
void mla_benchmark_executor_run(mla_benchmark_executor_t &executor, mla_test_uint32_t benchmark_number);
void mla_benchmark_executor_register(mla_benchmark_executor_t &executor, mla_benchmark_t &benchmark);

#endif
