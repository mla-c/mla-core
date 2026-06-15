//
// Created by christian on 9/10/25.
//

#include "mla_benchmark.h"
#include "../mla_test_utils.h"

#if (mla_test_global_feature_flag_benchmark_memory == 1)
#include "../../core/mla_data_types.h"
#endif


typedef mla_test_pointer_t (*test_benchmark_malloc_hook_t)(mla_test_uint32_t size);
typedef void (*test_benchmark_free_hook_t)(mla_test_pointer_t pointer);

static test_benchmark_malloc_hook_t mla_benchmark_malloc_hook_original = nullptr;
static test_benchmark_free_hook_t mla_benchmark_free_hook_original = nullptr;

static mla_test_uint64_t mla_benchmark_allocated_memory = 0;

mla_test_pointer_t mla_benchmark_malloc_stat_hook(mla_test_uint32_t size) {
    mla_benchmark_allocated_memory += size;
    return mla_benchmark_malloc_hook_original(size);
}

static mla_test_pointer_t g_mla_benchmark_memory_arena = nullptr;
static mla_test_uint32_t g_mla_benchmark_memory_arena_size = 0;
static mla_test_uint32_t g_mla_benchmark_memory_arena_offset = 0;
static mla_test_bool_t g_mla_benchmark_memory_arena_out_of_memory_triggered = false;
static mla_test_pointer_t g_mla_benchmark_arena_mutex = nullptr;

mla_test_uint32_t mla_align_up(mla_test_uint32_t value, mla_test_uint32_t alignment) {
    return (value + (alignment - 1U)) & ~(alignment - 1U);
}

mla_test_pointer_t mla_benchmark_malloc_in_arena_hook(mla_test_uint32_t size) {

    mla_benchmark_allocated_memory += size;

    if (g_mla_benchmark_memory_arena == nullptr) {
        return nullptr;
    }

    if (!g_test_mutex.lock_mutex(g_mla_benchmark_arena_mutex)) {
        return nullptr;
    }

    // Align current offset
    mla_size_t aligned_offset = mla_align_up(g_mla_benchmark_memory_arena_offset, mla_test_global_config_benchmark_arena_alignment);

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
    if (g_mla_benchmark_memory_arena != nullptr && pointer != nullptr) {
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

#if (mla_test_global_feature_flag_benchmark_use_median == 1)

// Simple partition function for median calculation
static void mla_internal_benchmark_partition_for_median(mla_test_uint64_t* arr, mla_test_uint32_t left, mla_test_uint32_t right, mla_test_uint32_t k) {
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

            if (i >= j) {
                break;
            }

            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }

        // Early exit if partition point matches k
        if (j == k) {
            return;
        }

        // Branchless interval selection
        mla_test_bool_t use_left = (j > k);
        right = use_left ? j : right;
        left = use_left ? left : (j + 1);
    }
}

static mla_test_uint64_t mla_internal_benchmark_calculate_median(mla_test_uint64_t* times, mla_test_uint32_t count) {
    if (count == 0) {
        return 0;
    }

    if (count == 1) {
        return times[0];
    }
    
    mla_test_uint32_t mid = count / 2;
    mla_internal_benchmark_partition_for_median(times, 0, count - 1, mid);
    
    if (count % 2 == 1) {
        return times[mid];
    } else {
        // For even count, find both middle elements using quickselect
        mla_test_uint64_t mid1 = times[mid];
        mla_internal_benchmark_partition_for_median(times, 0, mid - 1, mid - 1);
        mla_test_uint64_t mid2 = times[mid - 1];
        return (mid1 + mid2) / 2;
    }
}

#endif

#if (mla_test_global_feature_flag_benchmark_memory == 1)

void mla_benchmark_run_in_arena_fixed_size(mla_benchmark_t &benchmark, mla_test_uint32_t arena_size, mla_test_uint32_t benchmarkIterations, mla_test_output_format_t output_format) {

    if (output_format == mla_test_output_format_text) {
        mla_test_print("AAA|", 4);
    } else if (output_format == mla_test_output_format_json) {
        mla_test_print(",\n", 2);
        mla_test_print("{\n", 2);
        mla_test_print("  \"WithMemoryArena\": true,\n", 27);
    }


    if (benchmark.setUp != nullptr) {
        benchmark.setUp();
    }

    g_mla_benchmark_memory_arena_offset = 0;
    g_mla_benchmark_memory_arena_size = arena_size;
    g_mla_benchmark_memory_arena = mla_platform_malloc(arena_size);
    g_mla_benchmark_memory_arena_out_of_memory_triggered = false;
    g_mla_benchmark_arena_mutex = g_test_mutex.create_mutex();

    mla_benchmark_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_benchmark_malloc_in_arena_hook;
    mla_benchmark_free_hook_original = g_low_level_access.free;
    g_low_level_access.free = mla_benchmark_free_in_arena_hook;

    mla_benchmark_allocated_memory = 0;

#if (mla_test_global_feature_flag_benchmark_use_median == 1)
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
    mla_test_uint64_t allocated_memory_per_interation = 0;
    
    if (g_mla_benchmark_memory_arena_out_of_memory_triggered) {
        // The arena was not big enough to run all iterations
        allocated_memory_per_interation = 0;
        benchmarkIterations = 0;
        minTime = 999999;
        maxTime = 999999;
        medianTime = 999999;
    } else {
        medianTime = mla_internal_benchmark_calculate_median(times, actualIterations);
        if (actualIterations == 0) {
            allocated_memory_per_interation = 0;
        } else {
            allocated_memory_per_interation = (long long int)(mla_benchmark_allocated_memory / actualIterations);
        }
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
    auto allocated_memory_per_interation = 0;

    if (benchmarkIterations == 0) {
        allocated_memory_per_interation = 0;
    } else {
        allocated_memory_per_interation = (long long int)(mla_benchmark_allocated_memory / benchmarkIterations);
    }

    if (g_mla_benchmark_memory_arena_out_of_memory_triggered) {
        // The arena was not big enough to run all iterations
        allocated_memory_per_interation = 0;
        benchmarkIterations = 0;
        minTime = 999999;
        maxTime = 999999;
        averageTime = 999999;
    }
#endif

    g_low_level_access.free = mla_benchmark_free_hook_original;
    mla_benchmark_free_hook_original = nullptr;
    g_low_level_access.malloc = mla_benchmark_malloc_hook_original;
    mla_benchmark_malloc_hook_original = nullptr;
    g_test_mutex.destroy_mutex(g_mla_benchmark_arena_mutex);
    g_mla_benchmark_arena_mutex = 0;

    if (benchmark.tearDown != nullptr) {
        benchmark.tearDown();
    }


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
#if (mla_test_global_feature_flag_benchmark_use_median == 1)
    // Category column (24 chars)
    mla_test_char_t category_padded[25];
    mla_test_uint32_t cat_len = (mla_test_uint32_t)mla_test_strlen(benchmark.category);
    mla_test_memcpy(category_padded, benchmark.category, cat_len);
    if (cat_len < 24) {
        mla_test_memset(&category_padded[cat_len], ' ', 24 - cat_len);
    }
    category_padded[24] = '\0';
    mla_test_print(category_padded, 24);

    // Name column (30 chars)
    mla_test_print("|", 1);
    mla_test_print(name_with_arena_sufix, 30);

    // MinTime column (9 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_min[20];
    mla_test_uint32_t len_min = mla_uint64_to_string(buffer_min, sizeof(buffer_min), minTime);
    if (len_min < 9) {
        for (mla_test_uint32_t i = 0; i < 9 - len_min; i++) {
            mla_test_print(" ", 1);
        }
    }
    mla_test_print(buffer_min, len_min);

    // MaxTime column (12 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_max[20];
    mla_test_uint32_t len_max = mla_uint64_to_string(buffer_max, sizeof(buffer_max), maxTime);
    if (len_max < 12) {
        for (mla_test_uint32_t i = 0; i < 12 - len_max; i++) {
            mla_test_print(" ", 1);
        }
    }
    mla_test_print(buffer_max, len_max);

    // MedianTime column (9 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_median[20];
    mla_test_uint32_t len_median = mla_uint64_to_string(buffer_median, sizeof(buffer_median), medianTime);
    if (len_median < 9) {
        for (mla_test_uint32_t i = 0; i < 9 - len_median; i++) {
            mla_test_print(" ", 1);
        }
    }
    mla_test_print(buffer_median, len_median);

    // AllocatedMemory column (12 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_mem[20];
    mla_test_uint32_t len_mem = mla_uint64_to_string(buffer_mem, sizeof(buffer_mem), (mla_test_uint64_t)allocated_memory_per_interation);
    if (len_mem < 12) {
        for (mla_test_uint32_t i = 0; i < 12 - len_mem; i++) {
            mla_test_print(" ", 1);
        }
    }
    mla_test_print(buffer_mem, len_mem);

    // Iterations column (12 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_iter[20];
    mla_test_uint32_t len_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
    if (len_iter < 12) {
        for (mla_test_uint32_t i = 0; i < 12 - len_iter; i++) {
            mla_test_print(" ", 1);
        }
    }
    mla_test_print(buffer_iter, len_iter);

    mla_test_print("\n", 1);
#else
    // Category column (24 chars)
    mla_test_char_t category_padded[25];
    mla_test_uint32_t cat_len = (mla_test_uint32_t)mla_test_strlen(benchmark.category);
    mla_test_memcpy(category_padded, benchmark.category, cat_len);
    if (cat_len < 24) {
        mla_test_memset(&category_padded[cat_len], ' ', 24 - cat_len);
    }
    category_padded[24] = '\0';
    mla_test_print(category_padded, 24);

    // Name column (30 chars)
    mla_test_print("|", 1);
    mla_test_print(name_with_arena_sufix, 30);

    // MinTime column (9 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_min[20];
    mla_test_uint32_t len_min = mla_uint64_to_string(buffer_min, sizeof(buffer_min), minTime);
    if (len_min < 9) {
        for (mla_test_uint32_t i = 0; i < 9 - len_min; i++) mla_test_print(" ", 1);
    }
    mla_test_print(buffer_min, len_min);

    // MaxTime column (12 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_max[20];
    mla_test_uint32_t len_max = mla_uint64_to_string(buffer_max, sizeof(buffer_max), maxTime);
    if (len_max < 12) {
        for (mla_test_uint32_t i = 0; i < 12 - len_max; i++) mla_test_print(" ", 1);
    }
    mla_test_print(buffer_max, len_max);

    // AverageTime column (9 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_avg[20];
    mla_test_uint32_t len_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), averageTime);
    if (len_avg < 9) {
        for (mla_test_uint32_t i = 0; i < 9 - len_avg; i++) mla_test_print(" ", 1);
    }
    mla_test_print(buffer_avg, len_avg);

    // AllocatedMemory column (12 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_mem[20];
    mla_test_uint32_t len_mem = mla_uint64_to_string(buffer_mem, sizeof(buffer_mem), (mla_test_uint64_t)allocated_memory_per_interation);
    if (len_mem < 12) {
        for (mla_test_uint32_t i = 0; i < 12 - len_mem; i++) mla_test_print(" ", 1);
    }
    mla_test_print(buffer_mem, len_mem);

    // Iterations column (12 chars, right-aligned)
    mla_test_print("|", 1);
    mla_test_char_t buffer_iter[20];
    mla_test_uint32_t len_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
    if (len_iter < 12) {
        for (mla_test_uint32_t i = 0; i < 12 - len_iter; i++) mla_test_print(" ", 1);
    }
    mla_test_print(buffer_iter, len_iter);

    mla_test_print("\n", 1);
#endif
    } else if (output_format == mla_test_output_format_json) {

        mla_test_print("  \"Category\": \"", 15);
        mla_test_print(benchmark.category, (mla_test_uint32_t)mla_test_strlen(benchmark.category));
        mla_test_print("\",\n  \"Name\": \"", 14);
        mla_test_print(benchmark.name, (mla_test_uint32_t)mla_test_strlen(benchmark.name));
        mla_test_print("\",\n  \"MinTimeNs\": ", 18);
        mla_test_char_t buffer[20];
        mla_test_uint32_t strLength = mla_uint64_to_string(buffer, sizeof(buffer), minTime);
        mla_test_print(buffer, strLength);
        mla_test_print(",\n  \"MaxTimeNs\": ", 17);
        strLength = mla_uint64_to_string(buffer, sizeof(buffer), maxTime);
        mla_test_print(buffer, strLength);
        mla_test_print(",\n", 2);

#if (mla_test_global_feature_flag_benchmark_use_median == 1)
        mla_print("  \"AverageTimeNs\": ", 19);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t strLength_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), medianTime);
        mla_test_print(buffer_avg, strLength_avg);
        mla_test_print(",\n", 2);
#else
        mla_print("  \"AverageTimeNs\": ", 19);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t strLength_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), averageTime);
        mla_test_print(buffer_avg, strLength_avg);
        mla_test_print(",\n", 2);
#endif

        mla_print("  \"AllocatedMemoryPerIterationBytes\": ", 38);
        mla_test_char_t buffer_mem[20];
        mla_test_uint32_t strLength_mem = mla_uint64_to_string(buffer_mem, sizeof(buffer_mem), (mla_test_uint64_t)allocated_memory_per_interation);
        mla_test_print(buffer_mem, strLength_mem);
        mla_test_print(",\n  \"Iterations\": ", 18);
        mla_test_char_t buffer_iter[20];
        mla_test_uint32_t strLength_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
        mla_test_print(buffer_iter, strLength_iter);
        mla_test_print("\n}", 2);

    }

    mla_platform_free(g_mla_benchmark_memory_arena);

}


void mla_benchmark_run_in_arena(mla_benchmark_t &benchmark, mla_test_uint32_t arena_size_per_run, mla_test_output_format_t output_format) {


#if (mla_test_global_config_benchmark_max_arena_size > 0)

    // No memory used in the test so no need to run in an arena
    if (arena_size_per_run <= 0) {
        return;
    }

    mla_test_uint32_t benchmarkIterations = CONST_BENCHMARK_ITERATIONS / benchmark.iterationDivision;
    mla_test_uint64_t arena_size = static_cast<mla_test_uint64_t>(mla_align_up(arena_size_per_run, mla_test_global_config_benchmark_arena_alignment)) * benchmarkIterations;

    while (arena_size > (mla_test_uint64_t)mla_test_global_config_benchmark_max_arena_size) {
        benchmarkIterations = benchmarkIterations / 2;
        arena_size = (mla_test_uint64_t)((arena_size_per_run * benchmarkIterations) * 1.1); // Add some extra space to the arena to avoid edge cases
    }

    arena_size = mla_align_up((mla_test_uint32_t)arena_size, mla_test_global_config_benchmark_arena_alignment);

    // Add some extra space to the arena to avoid edge cases
    if (arena_size > 0) {
        mla_benchmark_run_in_arena_fixed_size(benchmark, (mla_test_uint32_t)arena_size, benchmarkIterations, output_format);
    }

#else
    (void)benchmark;
    (void)arena_size_per_run;
#endif

}


#endif

void mla_benchmark_run(mla_benchmark_t &benchmark, mla_test_output_format_t output_format) {

    if (benchmark.setUp != nullptr) {
        benchmark.setUp();
    }

    mla_test_uint32_t warmupIterations = CONST_CPU_WARMUP_ITERATIONS / benchmark.iterationDivision;
    for (mla_test_uint32_t i = 0; i < warmupIterations; ++i) {
        benchmark.run();
    }

    mla_test_uint32_t benchmarkIterations = CONST_BENCHMARK_ITERATIONS / benchmark.iterationDivision;

#if (mla_test_global_feature_flag_benchmark_use_median == 1)
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

    auto medianTime = mla_internal_benchmark_calculate_median(times, benchmarkIterations);

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

#if (mla_test_global_feature_flag_benchmark_memory == 1)

    mla_benchmark_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_benchmark_malloc_stat_hook;

    mla_benchmark_allocated_memory = 0;

    for (mla_test_uint32_t i = 0; i < benchmarkIterations; ++i) {
        benchmark.run();
    }

    g_low_level_access.malloc = mla_benchmark_malloc_hook_original;
    mla_benchmark_malloc_hook_original = nullptr;

#endif

    if (benchmark.tearDown != nullptr) {
        benchmark.tearDown();
    }

#if (mla_test_global_feature_flag_benchmark_memory == 1)

    if (output_format == mla_test_output_format_text) {
#if (mla_test_global_feature_flag_benchmark_use_median == 1)
        // Category column (24 chars, left-aligned)
        mla_test_print("|", 1);
        mla_test_char_t category_padded[25];
        mla_test_uint32_t cat_len = mla_test_strlen(benchmark.category);
        mla_test_memcpy(category_padded, benchmark.category, cat_len);
        if (cat_len < 24) {
            mla_test_memset(&category_padded[cat_len], ' ', 24 - cat_len);
        }
        category_padded[24] = '\0';
        mla_test_print(category_padded, 24);

        // Name column (30 chars, left-aligned)
        mla_test_print("|", 1);
        mla_test_char_t name_padded[31];
        mla_test_uint32_t name_len = mla_test_strlen(benchmark.name);
        if (name_len > 30) {
            name_len = 30;
        }
        mla_test_memcpy(name_padded, benchmark.name, name_len);
        if (name_len < 30) {
            mla_test_memset(&name_padded[name_len], ' ', 30 - name_len);
        }
        name_padded[30] = '\0';
        mla_test_print(name_padded, 30);

        // MinTime column (9 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_min[20];
        mla_test_uint32_t len_min = mla_uint64_to_string(buffer_min, sizeof(buffer_min), minTime);
        if (len_min < 9) {
            for (mla_test_uint32_t i = 0; i < 9 - len_min; i++) {
                mla_test_print(" ", 1);
            }
        }
        mla_test_print(buffer_min, len_min);

        // MaxTime column (12 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_max[20];
        mla_test_uint32_t len_max = mla_uint64_to_string(buffer_max, sizeof(buffer_max), maxTime);
        if (len_max < 12) {
            for (mla_test_uint32_t i = 0; i < 12 - len_max; i++) {
                mla_test_print(" ", 1);
            }
        }
        mla_test_print(buffer_max, len_max);

        // MedianTime column (9 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_median[20];
        mla_test_uint32_t len_median = mla_uint64_to_string(buffer_median, sizeof(buffer_median), medianTime);
        if (len_median < 9) {
            for (mla_test_uint32_t i = 0; i < 9 - len_median; i++) {
                mla_test_print(" ", 1);
            }
        }
        mla_test_print(buffer_median, len_median);

        // AllocatedMemory column (12 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_mem[20];
        mla_test_uint64_t allocated_memory_per_iteration = benchmarkIterations > 0 ? (mla_benchmark_allocated_memory / benchmarkIterations) : 0;
        mla_test_uint32_t len_mem = mla_uint64_to_string(buffer_mem, sizeof(buffer_mem), allocated_memory_per_iteration);
        if (len_mem < 12) {
            for (mla_test_uint32_t i = 0; i < 12 - len_mem; i++) {
                mla_test_print(" ", 1);
            }
        }
        mla_test_print(buffer_mem, len_mem);

        // Iterations column (12 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_iter[20];
        mla_test_uint32_t len_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
        if (len_iter < 12) {
            for (mla_test_uint32_t i = 0; i < 12 - len_iter; i++) {
                mla_test_print(" ", 1);
            }
        }
        mla_test_print(buffer_iter, len_iter);

        mla_test_print("\n", 1);
#else
        // Category column (24 chars, left-aligned)
        mla_test_print("|", 1);
        mla_test_char_t category_padded[25];
        mla_test_uint32_t cat_len = mla_test_strlen(benchmark.category);
        mla_test_memcpy(category_padded, benchmark.category, cat_len);
        if (cat_len < 24) {
            mla_test_memset(&category_padded[cat_len], ' ', 24 - cat_len);
        }
        category_padded[24] = '\0';
        mla_test_print(category_padded, 24);

        // Name column (30 chars, left-aligned)
        mla_test_print("|", 1);
        mla_test_char_t name_padded[31];
        mla_test_uint32_t name_len = mla_test_strlen(benchmark.name);
        if (name_len > 30) {
            name_len = 30;
        }
        mla_test_memcpy(name_padded, benchmark.name, name_len);
        if (name_len < 30) {
            mla_test_memset(&name_padded[name_len], ' ', 30 - name_len);
        }
        name_padded[30] = '\0';
        mla_test_print(name_padded, 30);

        // MinTime column (9 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_min[20];
        mla_test_uint32_t len_min = mla_uint64_to_string(buffer_min, sizeof(buffer_min), minTime);
        if (len_min < 9) {
            for (mla_test_uint32_t i = 0; i < 9 - len_min; i++) mla_test_print(" ", 1);
        }
        mla_test_print(buffer_min, len_min);

        // MaxTime column (12 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_max[20];
        mla_test_uint32_t len_max = mla_uint64_to_string(buffer_max, sizeof(buffer_max), maxTime);
        if (len_max < 12) {
            for (mla_test_uint32_t i = 0; i < 12 - len_max; i++) mla_test_print(" ", 1);
        }
        mla_test_print(buffer_max, len_max);

        // MedianTime column (9 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t len_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_median), averageTime);
        if (len_avg < 9) {
            for (mla_test_uint32_t i = 0; i < 9 - len_avg; i++) mla_test_print(" ", 1);
        }
        mla_test_print(buffer_avg, len_avg);

        // AllocatedMemory column (12 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_mem[20];
        mla_test_uint32_t len_mem = mla_uint64_to_string(buffer_mem, sizeof(buffer_mem), (mla_test_uint64_t)(mla_benchmark_allocated_memory / benchmarkIterations));
        if (len_mem < 12) {
            for (mla_test_uint32_t i = 0; i < 12 - len_mem; i++) mla_test_print(" ", 1);
        }
        mla_test_print(buffer_mem, len_mem);

        // Iterations column (12 chars, right-aligned)
        mla_test_print("|", 1);
        mla_test_char_t buffer_iter[20];
        mla_test_uint32_t len_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
        if (len_iter < 12) {
            for (mla_test_uint32_t i = 0; i < 12 - len_iter; i++) mla_test_print(" ", 1);
        }
        mla_test_print(buffer_iter, len_iter);

        mla_test_print("\n", 1);
#endif
    } else if (output_format == mla_test_output_format_json) {

        mla_test_print("  \"WithMemoryArena\": false,\n", 28);
        mla_test_print("  \"Category\": \"", 15);
        mla_test_print(benchmark.category, (mla_test_uint32_t)mla_test_strlen(benchmark.category));
        mla_test_print("\",\n  \"Name\": \"", 14);
        mla_test_print(benchmark.name, (mla_test_uint32_t)mla_test_strlen(benchmark.name));
        mla_test_print("\",\n  \"MinTimeNs\": ", 18);
        mla_test_char_t buffer[20];
        mla_test_uint32_t strLength = mla_uint64_to_string(buffer, sizeof(buffer), minTime);
        mla_test_print(buffer, strLength);
        mla_test_print(",\n  \"MaxTimeNs\": ", 17);
        strLength = mla_uint64_to_string(buffer, sizeof(buffer), maxTime);
        mla_test_print(buffer, strLength);
        mla_test_print(",\n", 2);

#if (mla_test_global_feature_flag_benchmark_use_median == 1)
        mla_print("  \"AverageTimeNs\": ", 19);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t strLength_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), medianTime);
        mla_test_print(buffer_avg, strLength_avg);
        mla_test_print(",\n", 2);
#else
        mla_print("  \"AverageTimeNs\": ", 19);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t strLength_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), averageTime);
        mla_test_print(buffer_avg, strLength_avg);
        mla_test_print(",\n", 2);
#endif

        mla_print("  \"AllocatedMemoryPerIterationBytes\": ", 38);
        mla_test_char_t buffer_mem[20];
        mla_test_uint64_t allocated_memory_per_iteration = benchmarkIterations > 0 ? (mla_benchmark_allocated_memory / benchmarkIterations) : 0;
        mla_test_uint32_t strLength_mem = mla_uint64_to_string(buffer_mem, sizeof(buffer_mem), allocated_memory_per_iteration);
        mla_test_print(buffer_mem, strLength_mem);
        mla_test_print(",\n  \"Iterations\": ", 18);
        mla_test_char_t buffer_iter[20];
        mla_test_uint32_t strLength_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
        mla_test_print(buffer_iter, strLength_iter);
        mla_test_print("\n}", 2);
    }

    // Start the benchmark one more time but inside an memory arena
    if (benchmarkIterations > 0) {
        mla_benchmark_run_in_arena(benchmark, (mla_test_uint32_t)(mla_benchmark_allocated_memory / benchmarkIterations), output_format);
    }

#else

    if (output_format == mla_test_output_format_text) {
#if (mla_test_global_feature_flag_benchmark_use_median == 1)
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

        mla_test_print("  \"Category\": \"", 15);
        mla_test_print(benchmark.category, (mla_test_uint32_t)mla_test_strlen(benchmark.category));
        mla_test_print("\",\n  \"Name\": \"", 14);
        mla_test_print(benchmark.name, (mla_test_uint32_t)mla_test_strlen(benchmark.name));
        mla_test_print("\",\n  \"MinTimeNs\": ", 18);
        mla_test_char_t buffer[20];
        mla_test_uint32_t strLength = mla_uint64_to_string(buffer, sizeof(buffer), minTime);
        mla_test_print(buffer, strLength);
        mla_test_print(",\n  \"MaxTimeNs\": ", 17);
        strLength = mla_uint64_to_string(buffer, sizeof(buffer), maxTime);
        mla_test_print(buffer, strLength);
        mla_test_print(",\n", 2);

#if (mla_test_global_feature_flag_benchmark_use_median == 1)
        mla_print("  \"AverageTimeNs\": ", 19);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t strLength_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), medianTime);
        mla_test_print(buffer_avg, strLength_avg);
        mla_test_print(",\n", 2);
#else
        mla_print("  \"AverageTimeNs\": ", 19);
        mla_test_char_t buffer_avg[20];
        mla_test_uint32_t strLength_avg = mla_uint64_to_string(buffer_avg, sizeof(buffer_avg), averageTime);
        mla_test_print(buffer_avg, strLength_avg);
        mla_test_print(",\n", 2);
#endif

        mla_test_print("  \"Iterations\": ", 16);
        mla_test_char_t buffer_iter[20];
        mla_test_uint32_t strLength_iter = mla_uint64_to_string(buffer_iter, sizeof(buffer_iter), (mla_test_uint64_t)benchmarkIterations);
        mla_test_print(buffer_iter, strLength_iter);
        mla_test_print("\n}", 2);
    }


#endif

}
