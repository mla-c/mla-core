//
// Created by christian on 9/10/25.
//

#include "mla_benchmark_executor.h"

mla_benchmark_executor_t mla_benchmark_executor(mla_test_uint32_t p_MaxBenchmarks) {

    mla_benchmark_executor_t executor;
    executor.max_benchmarks = p_MaxBenchmarks;
    executor.benchmarks = (mla_benchmark_t*)mla_test_malloc(sizeof(mla_benchmark_t) * p_MaxBenchmarks);
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
    mla_test_free(executor.benchmarks);
    executor.benchmarks = nullptr;
    executor.max_benchmarks = 0;
}

void __mla_benchmark_print_into_text(mla_test_output_format_t output_format) {

    if (output_format != mla_test_output_format_text) {
        return;
    }


#ifdef mla_debug_build
    mla_test_printf("#########################################################################\n");
    mla_test_printf("Benchmarking in DEBUG mode is not recommended.\n");
    mla_test_printf("Please run benchmarks in RELEASE mode for accurate results.\n");
    mla_test_printf("#########################################################################\n\n");

#endif

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))
#if (!defined(mla_benchmark_use_median) || (mla_benchmark_use_median == 1))
    mla_test_printf("%-3s|%-24s|%-30s|%9s|%12s|%9s|%12s|%12s|\n",
           "No",
           "Category",
           "Name",
           "Min (ns)",
           "Max (ns)",
           "Med (ns)",
           "Mem (bytes)",
           "Iterations");
#else
    mla_test_printf("%-3s|%-24s|%-30s|%9s|%12s|%9s|%12s|%12s|\n",
           "No",
           "Category",
           "Name",
           "Min (ns)",
           "Max (ns)",
           "Avg (ns)",
           "Mem (bytes)",
           "Iterations");
#endif
#else

#if (!defined(mla_benchmark_use_median) || (mla_benchmark_use_median == 1))
    mla_test_printf("%-3s|%-24s|%-30s|%9s|%12s|%9s|%12s|\n",
           "No",
           "Category",
           "Name",
           "Min (ns)",
           "Max (ns)",
           "Med (ns)",
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
#endif
}

void mla_benchmark_executor_run_all(mla_benchmark_executor_t &executor, mla_test_output_format_t output_format) {

    __mla_benchmark_print_into_text(output_format);

    if (output_format == mla_test_output_format_json) {
        mla_test_printf("[\n");
    }

    for (mla_test_uint32_t i = 0; i < executor.max_benchmarks; ++i) {
        if (executor.benchmarks[i].name != nullptr) {

            if (output_format == mla_test_output_format_text) {
                mla_test_printf("%3ld", i + 1);
            } else if (output_format == mla_test_output_format_json) {
                if (i > 0) {
                    mla_test_printf(",\n");
                }
                mla_test_printf("{\n");
            }

            mla_benchmark_run(executor.benchmarks[i], output_format);
        }
    }

    if (output_format == mla_test_output_format_json) {
        mla_test_printf("\n]\n");
    }
}


void mla_benchmark_executor_run(mla_benchmark_executor_t &executor, mla_test_uint32_t benchmark_number, mla_test_output_format_t output_format) {

    __mla_benchmark_print_into_text(output_format);

    mla_test_uint32_t benchmark_index = benchmark_number - 1; // Convert to zero-based index


    if (benchmark_index >= executor.max_benchmarks || executor.benchmarks[benchmark_index].name == nullptr) {
        mla_test_printf("Invalid benchmark index: %ld\n", benchmark_number);
        return; // Invalid benchmark index
    }

    if (output_format == mla_test_output_format_text) {
        mla_test_printf("%3ld", benchmark_number);
    } else if (output_format == mla_test_output_format_json) {
        mla_test_printf("{\n");
    }

    mla_benchmark_run(executor.benchmarks[benchmark_index], output_format);

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