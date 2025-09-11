//
// Created by christian on 9/10/25.
//

#include "mla_benchmark.h"

#ifdef mla_memory_benchmark
#include "../../core-os/mla_data_types.h"
#endif

#include <chrono>

typedef mla_test_pointer_t (*test_benchmark_malloc_hook_t)(mla_test_uint32_t size);

static test_benchmark_malloc_hook_t mla_benchmark_malloc_hook_original = nullptr;

static mla_test_uint64_t mla_benchmark_allocated_memory = 0;

mla_test_pointer_t mla_benchmark_malloc_stat_hook(mla_test_uint32_t size) {
    mla_benchmark_allocated_memory += size;
    return mla_benchmark_malloc_hook_original(size);
}

mla_benchmark_t mla_benchmark(const mla_test_char_t *name,
                              const mla_test_char_t *category,
                              void (*run)(void),
                              void (*setUp)(void),
                              void (*tearDown)(void)) {
    return {
        name,
        category,
        1,
        run,
        setUp,
        tearDown
};
}

void mla_benchmark_set_iteration_division(mla_benchmark_t& benchmark, mla_test_uint32_t division) {
    benchmark.iterationDivision = division;
}

void mla_benchmark_destroy(mla_benchmark_t &benchmark) {
    benchmark.name = nullptr;
    benchmark.category = nullptr;
    benchmark.iterationDivision = 0;
    benchmark.run = nullptr;
    benchmark.setUp = nullptr;
    benchmark.tearDown = nullptr;
}



void mla_benchmark_run(mla_benchmark_t &benchmark) {
    if (benchmark.setUp) {
        benchmark.setUp();
    }

    mla_test_uint32_t warmupIterations = CONST_CPU_WARMUP_ITERATIONS / benchmark.iterationDivision;
    for (mla_test_uint32_t i = 0; i < warmupIterations; ++i) {
        benchmark.run();
    }

    mla_test_uint32_t benchmarkIterations = CONST_BENCHMARK_ITERATIONS / benchmark.iterationDivision;

    // Min, Max, and Average time tracking
    std::chrono::nanoseconds minTime = (std::chrono::nanoseconds::max)();
    std::chrono::nanoseconds maxTime = (std::chrono::nanoseconds::min)();
    std::chrono::nanoseconds totalTime(0);

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        benchmark.run();
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        if (elapsed < minTime) {
            minTime = elapsed;
        }
        if (elapsed > maxTime) {
            maxTime = elapsed;
        }
        totalTime += elapsed;
    }

    auto averageTime = totalTime / benchmarkIterations;

#ifdef mla_memory_benchmark

    mla_benchmark_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_benchmark_malloc_stat_hook;

    mla_benchmark_allocated_memory = 0;

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {
        benchmark.run();
    }

    g_low_level_access.malloc = mla_benchmark_malloc_hook_original;
    mla_benchmark_malloc_hook_original = nullptr;

#endif

    if (benchmark.tearDown) {
        benchmark.tearDown();
    }

#ifdef mla_memory_benchmark

    printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
           benchmark.category,
           benchmark.name,
           (long long int)minTime.count(),
           (long long int)maxTime.count(),
           (long long int)averageTime.count(),
           (long long int)(mla_benchmark_allocated_memory / benchmarkIterations),
           benchmarkIterations);

#else

    printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12ld\n",
           benchmark.category,
           benchmark.name,
           (long long int)minTime.count(),
           (long long int)maxTime.count(),
           (long long int)averageTime.count(),
           benchmarkIterations);

#endif

}
