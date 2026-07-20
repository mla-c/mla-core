//
// Created by christian on 9/10/25.
//

#include "mla_benchmark_executor.h"
#include "mla_test_utils.h"

mla_benchmark_executor_t mla_benchmark_executor() {

    mla_benchmark_executor_t executor;
    executor.capacity = 10;
    executor.count = 0;
    executor.benchmarks = static_cast<mla_benchmark_t *>(mla_test_malloc(sizeof(mla_benchmark_t) * executor.capacity));
    for (mla_test_uint32_t i = 0; i < executor.capacity; ++i) {
        executor.benchmarks[i] = {nullptr, nullptr, 0, false, nullptr, nullptr, nullptr};
    }
    return executor;
}

void mla_benchmark_executor_destroy(mla_benchmark_executor_t &executor) {
    for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
        if (executor.benchmarks[i].name != nullptr) {
            mla_benchmark_destroy(executor.benchmarks[i]);
        }
    }
    mla_test_free(executor.benchmarks);
    executor.benchmarks = nullptr;
    executor.capacity = 0;
    executor.count = 0;
}

void mla_private_benchmark_print_into_text(mla_test_output_format_t output_format) {

    if (output_format != mla_test_output_format_text) {
        return;
    }


#ifdef mla_debug_build
    mla_test_print("#########################################################################\n", 74);
    mla_test_print("Benchmarking in DEBUG mode is not recommended.\n", 47);
    mla_test_print("Please run benchmarks in RELEASE mode for accurate results.\n", 60);
    mla_test_print("#########################################################################\n\n", 74);

#endif

#if (mla_test_global_feature_flag_benchmark_memory == 1)
    mla_test_print("No |Category                |Name                          |Min (ns) |Max (ns)    |Med (ns) |Mem (bytes) |Iterations\n", 117);
#else
    mla_test_print("No |Category                |Name                          |Min (ns) |Max (ns)    |Avg (ns) |Iterations\n", 105);
#endif
}

void mla_benchmark_executor_run_all(mla_benchmark_executor_t &executor, mla_test_output_format_t output_format) {

    mla_private_benchmark_print_into_text(output_format);

    if (output_format == mla_test_output_format_json) {
        mla_test_print("[\n", 2);
    }

    for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
        if (executor.benchmarks[i].name != nullptr) {

            if (output_format == mla_test_output_format_text) {
                mla_test_char_t buffer[12];
                mla_test_uint32_t strLength = mla_test_uint32_to_string(buffer, sizeof(buffer), i + 1);

                if (strLength < 3) {
                    for (mla_test_uint32_t k = 0; k < 3 - strLength; k++) {
                        mla_test_print(" ", 1);
                    }
                }

                mla_test_print(buffer, strLength);
            } else if (output_format == mla_test_output_format_json) {
                if (i > 0) {
                    mla_test_print(",\n", 2);
                }
                mla_test_print("{\n", 2);
            }

            mla_benchmark_run(executor.benchmarks[i], output_format);
        }
    }

    if (output_format == mla_test_output_format_json) {
        mla_test_print("\n]\n", 3);
    }
}


void mla_benchmark_executor_run(mla_benchmark_executor_t &executor, mla_test_uint32_t benchmark_number, mla_test_output_format_t output_format) {

    mla_private_benchmark_print_into_text(output_format);

    mla_test_uint32_t benchmark_index = benchmark_number - 1; // Convert to zero-based index


    if (benchmark_index >= executor.count || executor.benchmarks[benchmark_index].name == nullptr) {
        mla_test_print("Invalid benchmark index: ", 25);
        mla_test_char_t buffer[12];
        mla_test_uint32_t strLength = mla_test_uint32_to_string(buffer, sizeof(buffer), benchmark_number);
        mla_test_print(buffer, strLength);
        mla_test_print("\n", 1);
        return; // Invalid benchmark index
    }

    if (output_format == mla_test_output_format_text) {
        mla_test_char_t buffer[12];
        mla_test_uint32_t strLength = mla_test_uint32_to_string(buffer, sizeof(buffer), benchmark_number);

        if (strLength < 3) {
            for (mla_test_uint32_t k = 0; k < 3 - strLength; k++) {
                mla_test_print(" ", 1);
            }
        }

        mla_test_print(buffer, strLength);
    } else if (output_format == mla_test_output_format_json) {
        mla_test_print("{\n", 2);
    }

    mla_benchmark_run(executor.benchmarks[benchmark_index], output_format);

}


void mla_benchmark_executor_register(mla_benchmark_executor_t &executor, mla_benchmark_t &benchmark) {
    if (benchmark.name == nullptr) {
        return;
    }

    if (executor.count >= executor.capacity) {
        mla_test_uint32_t increment = (executor.capacity / 4);
        if (increment < 10) {
            increment = 10;
        }
        mla_test_uint32_t newCapacity = executor.capacity + increment;
        mla_benchmark_t* newBenchmarks = static_cast<mla_benchmark_t *>(mla_test_malloc(sizeof(mla_benchmark_t) * newCapacity));

        // Copy existing benchmarks
        for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
            newBenchmarks[i] = executor.benchmarks[i];
        }

        // Initialize new slots
        for (mla_test_uint32_t i = executor.count; i < newCapacity; ++i) {
            newBenchmarks[i] = {nullptr, nullptr, 0, false, nullptr, nullptr, nullptr};
        }

        mla_test_free(executor.benchmarks);
        executor.benchmarks = newBenchmarks;
        executor.capacity = newCapacity;
    }

    executor.benchmarks[executor.count++] = benchmark;
}