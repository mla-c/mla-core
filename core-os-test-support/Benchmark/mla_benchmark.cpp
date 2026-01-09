//
// Created by christian on 9/10/25.
//

#include "mla_benchmark.h"
#include "../mla_test_utils.h"

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))
#include "../../core-os/mla_data_types.h"
#endif

#if (!defined(mla_benchmark_max_arena_size))
#define mla_benchmark_max_arena_size (100 * 1024 * 1024) // 100 MB
#endif

#if (!defined(mla_benchmark_arena_alignment))
#define mla_benchmark_arena_alignment 8u // 8 bytes alignment
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
static mla_bool_t g_mla_benchmark_memory_arena_out_of_memory_triggered = false;
static mla_test_uint64_t g_mla_benchmark_arena_mutex = 0;

mla_size_t mla_align_up(mla_size_t value, mla_size_t alignment) {
    return (value + (alignment - 1u)) & ~(alignment - 1u);
}

mla_test_pointer_t mla_benchmark_malloc_in_arena_hook(mla_test_uint32_t size) {

    mla_benchmark_allocated_memory += size;

    if (!g_mla_benchmark_memory_arena) {
        return nullptr;
    }

    if (!g_test_mutex.lock_mutex(g_mla_benchmark_arena_mutex)) {
        return nullptr;
    }

    // Align current offset
    mla_size_t aligned_offset = mla_align_up(g_mla_benchmark_memory_arena_offset, mla_benchmark_arena_alignment);

    // Bounds check including padding
    if (aligned_offset + size > g_mla_benchmark_memory_arena_size) {
        g_mla_benchmark_memory_arena_out_of_memory_triggered = true;
        return nullptr;
    }

    mla_test_uint8_t* base = (mla_test_uint8_t*)g_mla_benchmark_memory_arena;
    mla_test_uint8_t* ptr  = base + aligned_offset;

    // Advance offset
    g_mla_benchmark_memory_arena_offset = aligned_offset + size;

    g_test_mutex.unlock_mutex(g_mla_benchmark_arena_mutex);

    return (mla_test_pointer_t)ptr;

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

#if (!defined(mla_benchmark_use_median))
#define mla_benchmark_use_median 1  // Default to median (1), set to 0 for average
#endif

#if (mla_benchmark_use_median == 1)

// Simple partition function for median calculation
static void __mla_benchmark_partition_for_median(mla_test_uint64_t* arr, mla_test_uint32_t left, mla_test_uint32_t right, mla_test_uint32_t k) {
    while (left < right) {
        // Median-of-three with branchless min/max
        mla_test_uint32_t mid = left + ((right - left) >> 1);

        mla_test_uint64_t a = arr[left];
        mla_test_uint64_t b = arr[mid];
        mla_test_uint64_t c = arr[right];

        // Branchless sorting network for 3 elements
        mla_test_uint64_t tmp;
        tmp = a < b ? a : b; a = a < b ? b : a; b = tmp;
        tmp = b < c ? b : c; c = b < c ? c : b; b = tmp;
        tmp = a < b ? a : b; a = a < b ? b : a; b = tmp;

        arr[left] = a;
        arr[mid] = b;
        arr[right] = c;

        mla_test_uint64_t pivot = b;

        // Hoare partition with reduced branches
        mla_test_uint32_t i = left - 1;
        mla_test_uint32_t j = right + 1;

        while (true) {
            do { ++i; } while (arr[i] < pivot);
            do { --j; } while (arr[j] > pivot);

            if (i >= j) break;

            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }

        // Early exit if partition point matches k
        if (j == k) return;

        // Branchless interval selection
        mla_test_uint32_t use_left = (j > k);
        right = use_left ? j : right;
        left = use_left ? left : (j + 1);
    }
}

static mla_test_uint64_t __mla_benchmark_calculate_median(mla_test_uint64_t* times, mla_test_uint32_t count) {
    if (count == 0) return 0;
    if (count == 1) return times[0];
    
    mla_test_uint32_t mid = count / 2;
    __mla_benchmark_partition_for_median(times, 0, count - 1, mid);
    
    if (count % 2 == 1) {
        return times[mid];
    } else {
        // For even count, find both middle elements using quickselect
        mla_test_uint64_t mid1 = times[mid];
        __mla_benchmark_partition_for_median(times, 0, mid - 1, mid - 1);
        mla_test_uint64_t mid2 = times[mid - 1];
        return (mid1 + mid2) / 2;
    }
}

#endif

#if (!defined(mla_benchmark_memory) || (mla_benchmark_memory == 1))

void mla_benchmark_run_in_arena_fixed_size(mla_benchmark_t &benchmark, mla_test_uint32_t arena_size, mla_test_uint32_t benchmarkIterations, mla_test_output_format_t output_format) {

    if (output_format == mla_test_output_format_text) {
        mla_test_printf("AAA|");
    } else if (output_format == mla_test_output_format_json) {
        mla_test_printf(",\n")
        mla_test_printf("{\n")
        mla_test_printf("  \"WithMemoryArena\": true,\n");
    }


    g_mla_benchmark_memory_arena_offset = 0;
    g_mla_benchmark_memory_arena_size = arena_size;
    g_mla_benchmark_memory_arena = mla_malloc(arena_size);
    g_mla_benchmark_memory_arena_out_of_memory_triggered = false;
    g_mla_benchmark_arena_mutex = g_test_mutex.create_mutex();

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

#if (mla_benchmark_use_median == 1)
    // Min, Max, and Median time tracking
    mla_test_uint64_t minTime = 18446744073709551615ULL;
    mla_test_uint64_t maxTime = 0;
    
    // Allocate array for timing measurements to calculate median
    mla_test_uint64_t* times = (mla_test_uint64_t*)mla_test_malloc(sizeof(mla_test_uint64_t) * benchmarkIterations);
    
    mla_test_uint32_t actualIterations = 0;

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {

        auto start = g_benchmark_timer.current_nanoseconds();
        benchmark.run();
        auto end = g_benchmark_timer.current_nanoseconds();
        auto elapsed = end - start;

        times[i] = elapsed;
        actualIterations = i + 1;
        
        if (elapsed < minTime) {
            minTime = elapsed;
        }
        if (elapsed > maxTime) {
            maxTime = elapsed;
        }

        if (g_mla_benchmark_memory_arena_out_of_memory_triggered) {
            break;
        }
    }

    mla_test_uint64_t medianTime = 0;
    long long int allocated_memory_per_interation = 0;
    
    if (g_mla_benchmark_memory_arena_out_of_memory_triggered) {
        // The arena was not big enough to run all iterations
        allocated_memory_per_interation = 0;
        benchmarkIterations = 0;
        minTime = 999999;
        maxTime = 999999;
        medianTime = 999999;
    } else {
        medianTime = __mla_benchmark_calculate_median(times, actualIterations);
        allocated_memory_per_interation = (long long int)(mla_benchmark_allocated_memory / actualIterations);
    }

    mla_test_free(times);
#else
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

        if (g_mla_benchmark_memory_arena_out_of_memory_triggered) {
            break;
        }
    }

    auto averageTime = totalTime / benchmarkIterations;
    auto allocated_memory_per_interation = (long long int)(mla_benchmark_allocated_memory / benchmarkIterations);

    if (g_mla_benchmark_memory_arena_out_of_memory_triggered) {
        // The arena was not big enough to run all iterations
        allocated_memory_per_interation = 0;
        benchmarkIterations = 0;
        minTime = 999999;
        maxTime = 999999;
        averageTime = 999999;
    }
#endif

    if (benchmark.tearDown) {
        benchmark.tearDown();
    }

    g_low_level_access.is_gcc_pointer = mla_benchmark_is_gcc_pointer_hook_original;
    mla_benchmark_is_gcc_pointer_hook_original = nullptr;
    g_low_level_access.free = mla_benchmark_free_hook_original;
    mla_benchmark_free_hook_original = nullptr;
    g_low_level_access.malloc = mla_benchmark_malloc_hook_original;
    mla_benchmark_malloc_hook_original = nullptr;
    g_test_mutex.destroy_mutex(g_mla_benchmark_arena_mutex);
    g_mla_benchmark_arena_mutex = 0;


    char name_with_arena_sufix[31] = {0};

    // Build " (%u kb)" manually
    mla_test_uint32_t pos = 0;
    name_with_arena_sufix[pos++] = ' ';
    name_with_arena_sufix[pos++] = '(';

    // Convert number to string
    mla_test_char_t num_buffer[16];
    mla_test_uint32_t num_len = mla_uint32_to_string(num_buffer, 16, (mla_test_uint32_t)(arena_size / 1024));

    // Copy number
    for (mla_test_uint32_t i = 0; i < num_len; i++) {
        name_with_arena_sufix[pos++] = num_buffer[i];
    }

    // Add " kb)"
    name_with_arena_sufix[pos++] = ' ';
    name_with_arena_sufix[pos++] = 'k';
    name_with_arena_sufix[pos++] = 'b';
    name_with_arena_sufix[pos++] = ')';
    name_with_arena_sufix[pos] = '\0';

    mla_test_uint32_t charCount = pos;

    mla_test_uint32_t charToCopy = 31 - charCount;

    if (charToCopy > 0) {
        // We have space tocopy part of the name in front
        // Copy the beginning to the end
        mla_test_memmove(&name_with_arena_sufix[charToCopy - 1], &name_with_arena_sufix[0], charCount);
        // Copy the begining of the name
        mla_size_t name_size = (mla_size_t)mla_test_strlen(benchmark.name);
        if (name_size > charToCopy) {
            name_size = charToCopy;
        }
        mla_test_memcpy(&name_with_arena_sufix[0], benchmark.name, name_size);
        if (name_size + charCount < 30) {
            // Fillup remaning with spaces
            mla_test_memset(&name_with_arena_sufix[name_size], ' ', 31 - (name_size + charCount));
        }


    }

    if (output_format == mla_test_output_format_text) {
#if (mla_benchmark_use_median == 1)
        mla_test_printf("%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
               benchmark.category,
               name_with_arena_sufix,
               minTime,
               maxTime,
               medianTime,
               allocated_memory_per_interation,
               benchmarkIterations);
#else
        mla_test_printf("%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
               benchmark.category,
               name_with_arena_sufix,
               minTime,
               maxTime,
               averageTime,
               allocated_memory_per_interation,
               benchmarkIterations);
#endif
    } else if (output_format == mla_test_output_format_json) {
        mla_test_printf("  \"Category\": \"%s\",\n", benchmark.category);
        mla_test_printf("  \"Name\": \"%s\",\n", benchmark.name);
        mla_test_printf("  \"MinTimeNs\": %lld,\n", minTime);
        mla_test_printf("  \"MaxTimeNs\": %lld,\n", maxTime);
#if (mla_benchmark_use_median == 1)
        mla_test_printf("  \"AverageTimeNs\": %lld,\n", medianTime);
#else
        mla_test_printf("  \"AverageTimeNs\": %lld,\n", averageTime);
#endif
        mla_test_printf("  \"AllocatedMemoryPerIterationBytes\": %lld,\n", allocated_memory_per_interation);
        mla_test_printf("  \"Iterations\": %ld\n", benchmarkIterations);
        mla_test_printf("}");
    }

    mla_free(g_mla_benchmark_memory_arena);

}


void mla_benchmark_run_in_arena(mla_benchmark_t &benchmark, mla_test_uint32_t arena_size_per_run, mla_test_output_format_t output_format) {


#if (mla_benchmark_max_arena_size > 0)

    // No memory used in the test so no need to run in an arena
    if (arena_size_per_run <= 0) {
        return;
    }

    mla_test_uint32_t benchmarkIterations = CONST_BENCHMARK_ITERATIONS / benchmark.iterationDivision;
    mla_test_uint32_t arena_size = mla_align_up(arena_size_per_run, mla_benchmark_arena_alignment) * benchmarkIterations;

    while (arena_size > mla_benchmark_max_arena_size) {
        benchmarkIterations = benchmarkIterations / 2;
        arena_size = (mla_test_uint32_t)((arena_size_per_run * benchmarkIterations) * 1.1); // Add some extra space to the arena to avoid edge cases
    }

    arena_size = mla_align_up(arena_size, mla_benchmark_arena_alignment);

    // Add some extra space to the arena to avoid edge cases
    if (arena_size > 0) {
        mla_benchmark_run_in_arena_fixed_size(benchmark, arena_size, benchmarkIterations, output_format);
    }

#else
    (void)benchmark;
    (void)arena_size_per_run;
#endif

}


#endif

void mla_benchmark_run(mla_benchmark_t &benchmark, mla_test_output_format_t output_format) {

    if (benchmark.setUp) {
        benchmark.setUp();
    }

    mla_test_uint32_t warmupIterations = CONST_CPU_WARMUP_ITERATIONS / benchmark.iterationDivision;
    for (mla_test_uint32_t i = 0; i < warmupIterations; ++i) {
        benchmark.run();
    }

    mla_test_uint32_t benchmarkIterations = CONST_BENCHMARK_ITERATIONS / benchmark.iterationDivision;

#if (mla_benchmark_use_median == 1)
    // Min, Max, and Median time tracking
    mla_test_uint64_t minTime = 18446744073709551615ULL;
    mla_test_uint64_t maxTime = 0;
    
    // Allocate array for timing measurements to calculate median
    mla_test_uint64_t* times = (mla_test_uint64_t*)mla_test_malloc(sizeof(mla_test_uint64_t) * benchmarkIterations);

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {
        auto start = g_benchmark_timer.current_nanoseconds();
        benchmark.run();
        auto end = g_benchmark_timer.current_nanoseconds();
        auto elapsed = end - start;

        times[i] = elapsed;
        
        if (elapsed < minTime) {
            minTime = elapsed;
        }
        if (elapsed > maxTime) {
            maxTime = elapsed;
        }
    }

    auto medianTime = __mla_benchmark_calculate_median(times, benchmarkIterations);

    mla_test_free(times);
#else
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
#endif

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

    if (output_format == mla_test_output_format_text) {
#if (mla_benchmark_use_median == 1)
        mla_test_printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
           benchmark.category,
           benchmark.name,
           minTime,
           maxTime,
           medianTime,
           (long long int)(mla_benchmark_allocated_memory / benchmarkIterations),
           benchmarkIterations);
#else
        mla_test_printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12lld|%12ld\n",
           benchmark.category,
           benchmark.name,
           minTime,
           maxTime,
           averageTime,
           (long long int)(mla_benchmark_allocated_memory / benchmarkIterations),
           benchmarkIterations);
#endif
    } else if (output_format == mla_test_output_format_json) {
        mla_test_printf("  \"WithMemoryArena\": false,\n");
        mla_test_printf("  \"Category\": \"%s\",\n", benchmark.category);
        mla_test_printf("  \"Name\": \"%s\",\n", benchmark.name);
        mla_test_printf("  \"MinTimeNs\": %lld,\n", minTime);
        mla_test_printf("  \"MaxTimeNs\": %lld,\n", maxTime);
#if (mla_benchmark_use_median == 1)
        mla_test_printf("  \"AverageTimeNs\": %lld,\n", medianTime);
#else
        mla_test_printf("  \"AverageTimeNs\": %lld,\n", averageTime);
#endif
        mla_test_printf("  \"AllocatedMemoryPerIterationBytes\": %lld,\n", (long long int)(mla_benchmark_allocated_memory / benchmarkIterations));
        mla_test_printf("  \"Iterations\": %ld\n", benchmarkIterations);
        mla_test_printf("}");
    }

    // Start the benchmark one more time but inside an memory arena
    mla_benchmark_run_in_arena(benchmark, (mla_test_uint32_t)(mla_benchmark_allocated_memory / benchmarkIterations), output_format);

#else

    if (output_format == mla_test_output_format_text) {
#if (mla_benchmark_use_median == 1)
        mla_test_printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12ld\n",
               benchmark.category,
               benchmark.name,
               minTime,
               maxTime,
               medianTime,
               benchmarkIterations);
#else
        mla_test_printf("|%-24s|%-30s|%9lld|%12lld|%9lld|%12ld\n",
               benchmark.category,
               benchmark.name,
               minTime,
               maxTime,
               averageTime,
               benchmarkIterations);
#endif
    } else if (output_format == mla_test_output_format_json) {

        mla_test_printf("  \"Category\": \"%s\",\n", benchmark.category);
        mla_test_printf("  \"Name\": \"%s\",\n", benchmark.name);
        mla_test_printf("  \"MinTimeNs\": %lld,\n", minTime);
        mla_test_printf("  \"MaxTimeNs\": %lld,\n", maxTime);
#if (mla_benchmark_use_median == 1)
        mla_test_printf("  \"AverageTimeNs\": %lld,\n", medianTime);
#else
        mla_test_printf("  \"AverageTimeNs\": %lld,\n", averageTime);
#endif
        mla_test_printf("  \"Iterations\": %ld\n", benchmarkIterations);
        mla_test_printf("}");
    }


#endif

}
