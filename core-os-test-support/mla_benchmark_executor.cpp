//
// Created by christian on 9/10/25.
//

#include "mla_benchmark_executor.h"

mla_benchmark_executor_t mla_benchmark_executor(mla_test_uint32_t p_MaxBenchmarks) {

    mla_benchmark_executor_t executor;
    executor.max_benchmarks = p_MaxBenchmarks;
    executor.benchmarks = new mla_benchmark_t[p_MaxBenchmarks];
    for (mla_test_uint32_t i = 0; i < p_MaxBenchmarks; ++i) {
        executor.benchmarks[i] = {nullptr, nullptr, 0, nullptr, nullptr, nullptr};
    }
    return executor;
}

void mla_benchmark_executor_destroy(mla_benchmark_executor_t &executor) {
    for (mla_test_uint32_t i = 0; i < executor.max_benchmarks; ++i) {
        if (executor.benchmarks[i].name) {
            mla_benchmark_destroy(executor.benchmarks[i]);
        }
    }
    delete[] executor.benchmarks;
    executor.benchmarks = nullptr;
    executor.max_benchmarks = 0;
}

void __mla_benchmark_print_into_text() {

#ifdef mla_debug_build
    mla_test_printf("#########################################################################\n");
    mla_test_printf("Benchmarking in DEBUG mode is not recommended.\n");
    mla_test_printf("Please run benchmarks in RELEASE mode for accurate results.\n");
    mla_test_printf("#########################################################################\n\n");

#endif

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))
    mla_test_printf("%-3s|%-24s|%-30s|%9s|%12s|%9s|%12s|%12s|\n",
           "No",
           "Category",
           "Name",
           "Min (ns)",
           "Max (ns)",
           "Avg (ns)",
           "Mem (bytes)",
           "Iterations");
#else

    mla_test_printf("%-3s|%-24s|%-30s|%9s|%12s|%9s|%12s|\n",
           "No",
           "Category",
           "Name",
           "Min (ns)",
           "Max (ns)",
           "Avg (ns)",
           "Iterations");
#endif
}

void mla_benchmark_executor_run_all(mla_benchmark_executor_t &executor) {

    __mla_benchmark_print_into_text();

    for (mla_test_uint32_t i = 0; i < executor.max_benchmarks; ++i) {
        if (executor.benchmarks[i].name != nullptr) {
            mla_test_printf("%3ld", i + 1);
            mla_benchmark_run(executor.benchmarks[i]);
        }
    }
}


void mla_benchmark_executor_run(mla_benchmark_executor_t &executor, mla_test_uint32_t benchmark_number) {

    __mla_benchmark_print_into_text();

    mla_test_uint32_t benchmark_index = benchmark_number - 1; // Convert to zero-based index


    if (benchmark_index >= executor.max_benchmarks || executor.benchmarks[benchmark_index].name == nullptr) {
        mla_test_printf("Invalid benchmark index: %ld\n", benchmark_number);
        return; // Invalid benchmark index
    }

    mla_test_printf("%3ld", benchmark_number);
    mla_benchmark_run(executor.benchmarks[benchmark_index]);

}


void mla_benchmark_executor_register(mla_benchmark_executor_t &executor, mla_benchmark_t &benchmark) {
    if (benchmark.name == nullptr) {
        return;
    }

    for (mla_test_uint32_t i = 0; i < executor.max_benchmarks; ++i) {
        if (executor.benchmarks[i].name == nullptr) {
            executor.benchmarks[i] = benchmark;
            return;
        }
    }
}