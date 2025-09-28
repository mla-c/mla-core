//
// Created by christian on 9/10/25.
//

#include "mla_benchmark.h"

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))
#include "../../core-os/mla_data_types.h"
#endif

#if (!defined(mla_benchmark_max_arena_size))
#define mla_benchmark_max_arena_size (100 * 1024 * 1024) // 100 MB
#endif


typedef mla_test_pointer_t (*test_benchmark_malloc_hook_t)(mla_test_uint32_t size);
typedef void (*test_benchmark_free_hook_t)(mla_test_pointer_t pointer);
typedef mla_bool_t (*test_benchmark_is_gcc_pointer_hook_t)(const mla_test_pointer_t pointer);

static test_benchmark_is_gcc_pointer_hook_t mla_benchmark_is_gcc_pointer_hook_original = nullptr;
static test_benchmark_malloc_hook_t mla_benchmark_malloc_hook_original = nullptr;
static test_benchmark_free_hook_t mla_benchmark_free_hook_original = nullptr;

static mla_test_uint64_t mla_benchmark_allocated_memory = 0;

mla_test_pointer_t mla_benchmark_malloc_stat_hook(mla_test_uint32_t size) {
    mla_benchmark_allocated_memory += size;
    return mla_benchmark_malloc_hook_original(size);
}

static mla_pointer_t g_mla_benchmark_memory_arena = nullptr;
static mla_size_t g_mla_benchmark_memory_arena_size = 0;
static mla_size_t g_mla_benchmark_memory_arena_offset = 0;

mla_test_pointer_t mla_benchmark_malloc_in_arena_hook(mla_test_uint32_t size) {

    mla_benchmark_allocated_memory += size;

    if (g_mla_benchmark_memory_arena && (g_mla_benchmark_memory_arena_offset + size <= g_mla_benchmark_memory_arena_size)) {
        auto* base = static_cast<mla_test_uint8_t*>(g_mla_benchmark_memory_arena);
        auto* ptr  = base + g_mla_benchmark_memory_arena_offset;
        g_mla_benchmark_memory_arena_offset += size;
        return reinterpret_cast<mla_test_pointer_t>(ptr);
    }
    return nullptr;

}

mla_bool_t mla_benchmark_is_arena_pointer(const mla_test_pointer_t pointer) {
    if (g_mla_benchmark_memory_arena && pointer) {
        auto* base = static_cast<mla_test_uint8_t*>(g_mla_benchmark_memory_arena);
        auto* ptr  = static_cast<const mla_test_uint8_t*>(pointer);
        return (ptr >= base) && (ptr < (base + g_mla_benchmark_memory_arena_size));
    }
    return false;
}

void mla_benchmark_free_in_arena_hook(mla_test_pointer_t pointer) {
    (void)pointer;
    // No-op, memory will be freed when the arena is destroyed
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

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))

void mla_benchmark_run_in_arena_fixed_size(mla_benchmark_t &benchmark, mla_test_uint32_t arena_size, mla_test_uint32_t benchmarkIterations) {

    mla_test_printf("AAA|");

    g_mla_benchmark_memory_arena_offset = 0;
    g_mla_benchmark_memory_arena_size = arena_size;
    g_mla_benchmark_memory_arena = mla_malloc(arena_size);

    mla_benchmark_is_gcc_pointer_hook_original = g_low_level_access.is_gcc_pointer;
    g_low_level_access.is_gcc_pointer = mla_benchmark_is_arena_pointer;
    mla_benchmark_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_benchmark_malloc_in_arena_hook;
    mla_benchmark_free_hook_original = g_low_level_access.free;
    g_low_level_access.free = mla_benchmark_free_in_arena_hook;

    if (benchmark.setUp) {
        benchmark.setUp();
    }

    mla_benchmark_allocated_memory = 0;

    // Min, Max, and Average time tracking
    mla_test_uint64_t minTime = 18446744073709551615ULL;
    mla_test_uint64_t maxTime = 0;
    mla_test_uint64_t totalTime(0);

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {
        auto start = g_benchmark_timer.current_nanoseconds();
        benchmark.run();
        auto end = g_benchmark_timer.current_nanoseconds();
        auto elapsed = end - start;

        if (elapsed < minTime) {
            minTime = elapsed;
        }
        if (elapsed > maxTime) {
            maxTime = elapsed;
        }
        totalTime += elapsed;
    }

    auto averageTime = totalTime / benchmarkIterations;
    auto allocated_memory_per_interation = (long long int)(mla_benchmark_allocated_memory / benchmarkIterations);

    if (benchmark.tearDown) {
        benchmark.tearDown();
    }

    g_low_level_access.is_gcc_pointer = mla_benchmark_is_gcc_pointer_hook_original;
    mla_benchmark_is_gcc_pointer_hook_original = nullptr;
    g_low_level_access.free = mla_benchmark_free_hook_original;
    mla_benchmark_free_hook_original = nullptr;
    g_low_level_access.malloc = mla_benchmark_malloc_hook_original;
    mla_benchmark_malloc_hook_original = nullptr;


    char name_with_arena_sufix[31] = {0};
    mla_test_uint32_t charCount = snprintf(name_with_arena_sufix, 31, " (%ld kb)", (mla_size_t)(arena_size / 1024));


    mla_test_uint32_t charToCopy = 31 - charCount;

    if (charToCopy > 0) {
        // We have space tocopy part of the name in front
        // Copy the beginning to the end
        memmove(&name_with_arena_sufix[charToCopy - 1], &name_with_arena_sufix[0], charCount);
        // Copy the begining of the name
        mla_size_t name_size = (mla_size_t)strlen(benchmark.name);
        if (name_size > charToCopy) {
            name_size = charToCopy;
        }
        memcpy(&name_with_arena_sufix[0], benchmark.name, name_size);
        if (name_size + charCount < 30) {
            // Fillup remaning with spaces
            memset(&name_with_arena_sufix[name_size], ' ', 31 - (name_size + charCount));
        }


    }


    mla_test_printf("%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
               benchmark.category,
               name_with_arena_sufix,
               minTime,
               maxTime,
               averageTime,
               allocated_memory_per_interation,
               benchmarkIterations);

    mla_free(g_mla_benchmark_memory_arena);

}


void mla_benchmark_run_in_arena(mla_benchmark_t &benchmark, mla_test_uint32_t arena_size_per_run) {


#if (mla_benchmark_max_arena_size > 0)

    // No memory used in the test so no need to run in an arena
    if (arena_size_per_run <= 0) {
        return;
    }

    mla_test_uint32_t benchmarkIterations = CONST_BENCHMARK_ITERATIONS / benchmark.iterationDivision;
    mla_test_uint32_t arena_size = arena_size_per_run * benchmarkIterations;

    while (arena_size > mla_benchmark_max_arena_size) {
        benchmarkIterations = benchmarkIterations / 2;
        arena_size = (mla_test_uint32_t)((arena_size_per_run * benchmarkIterations) * 1.1); // Add some extra space to the arena to avoid edge cases
    }

    // Add some extra space to the arena to avoid edge cases
    if (arena_size > 0) {
        mla_benchmark_run_in_arena_fixed_size(benchmark, arena_size, benchmarkIterations);
    }

#else
    (void)benchmark;
    (void)arena_size_per_run;
#endif

}


#endif

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
    mla_test_uint64_t minTime = 18446744073709551615ULL;
    mla_test_uint64_t maxTime = 0;
    mla_test_uint64_t totalTime(0);

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {
        auto start = g_benchmark_timer.current_nanoseconds();
        benchmark.run();
        auto end = g_benchmark_timer.current_nanoseconds();
        auto elapsed = end - start;

        if (elapsed < minTime) {
            minTime = elapsed;
        }
        if (elapsed > maxTime) {
            maxTime = elapsed;
        }
        totalTime += elapsed;
    }

    auto averageTime = totalTime / benchmarkIterations;

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))

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

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))

    mla_test_printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
           benchmark.category,
           benchmark.name,
           minTime,
           maxTime,
           averageTime,
           (long long int)(mla_benchmark_allocated_memory / benchmarkIterations),
           benchmarkIterations);

    // Start the benchmark one more time but inside an memory arena
    mla_benchmark_run_in_arena(benchmark, (mla_test_uint32_t)(mla_benchmark_allocated_memory / benchmarkIterations));

#else

    mla_test_printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12ld\n",
           benchmark.category,
           benchmark.name,
           minTime,
           maxTime,
           averageTime,
           benchmarkIterations);

#endif

}
