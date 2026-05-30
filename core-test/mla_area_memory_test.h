//
// Created by Jules.
//

#ifndef MLA_AREA_MEMORY_TEST_H
#define MLA_AREA_MEMORY_TEST_H

#include "../lib/base-lib/core/mla_area_pointer_memory_manager.h"
#include "../lib/base-lib/test-support/mla_test_executor.h"
#include "../lib/base-lib/test-support/mla_benchmark_executor.h"

inline void MlaAreaMemoryAllocationTest() {
    mla_area_pointer_memory_manager_t area_manager = mla_area_pointer_memory_manager_create(1024);

    mla_pointer_t ptr1 = mla_malloc_buffer_with_manager(&area_manager.manager, 100);
    mla_check_assert_false(mla_pointer_is_null(ptr1), "Allocation failed");

    // Each mla_pointer_t = 1 ref on page (from constructor)
    mla_check_assert_equal(mla_pointer_ref_count(ptr1), 1, "Incorrect ref count after first allocation");

    {
        mla_pointer_t ptr2 = mla_malloc_buffer_with_manager(&area_manager.manager, 200);
        mla_check_assert_false(mla_pointer_is_null(ptr2), "Allocation failed");
        // Page refcount should be 2 now (1 from ptr1, 1 from ptr2)
        mla_check_assert_equal(mla_pointer_ref_count(ptr1), 2, "Incorrect ref count after second allocation");
    }
}

inline void MlaAreaMemoryPageOverflowTest() {
    mla_area_pointer_memory_manager_t area_manager = mla_area_pointer_memory_manager_create(256);

    mla_pointer_t ptr1 = mla_malloc_buffer_with_manager(&area_manager.manager, 100);
    mla_pointer_t ptr2 = mla_malloc_buffer_with_manager(&area_manager.manager, 100);
    mla_pointer_t ptr3 = mla_malloc_buffer_with_manager(&area_manager.manager, 100);

    mla_check_assert_false(mla_pointer_is_null(ptr1), "Allocation 1 failed");
    mla_check_assert_false(mla_pointer_is_null(ptr2), "Allocation 2 failed");
    mla_check_assert_false(mla_pointer_is_null(ptr3), "Allocation 3 failed");
}

static mla_bool_t g_cleanup_called = false;
inline void __test_cleanup_hook(mla_platform_pointer_t data, const mla_dynamic_data_t& userData) {
    (void)data; (void)userData;
    g_cleanup_called = true;
}

inline void MlaAreaMemoryCleanupTest() {
    g_cleanup_called = false;
    {
        mla_area_pointer_memory_manager_t area_manager = mla_area_pointer_memory_manager_create(1024);
        {
            mla_pointer_t ptr = mla_malloc_with_manager(&area_manager.manager, 100, __test_cleanup_hook, mla_dynamic_data_empty());
        }
        // At this point ptr is destroyed, page ref count hit 0 and called cleanup.
    }
    mla_check_assert_true(g_cleanup_called, "Cleanup hook not called");
}

void RegisterAreaMemoryTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("AllocationTest", test_category, MlaAreaMemoryAllocationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("PageOverflowTest", test_category, MlaAreaMemoryPageOverflowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CleanupTest", test_category, MlaAreaMemoryCleanupTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

// Benchmarks
static mla_area_pointer_memory_manager_t g_bench_area_manager;

inline void MlaAreaMemoryBenchmark_Setup() {
    g_bench_area_manager = mla_area_pointer_memory_manager_create(64 * 1024);
}

inline void MlaAreaMemoryBenchmark_TearDown() {
}

inline void MlaAreaMemoryBenchmark() {
    // Keep page alive
    mla_pointer_t keepAlive = mla_malloc_buffer_with_manager(&g_bench_area_manager.manager, 1);
    mla_pointer_t ptr = mla_malloc_buffer_with_manager(&g_bench_area_manager.manager, 32);
    (void)ptr;
    (void)keepAlive;
}

inline void MlaDefaultMemoryBenchmark() {
    mla_pointer_t keepAlive = mla_malloc_buffer_with_manager(&g_bench_area_manager.manager, 1);
    mla_pointer_t ptr = mla_malloc_buffer(32);
    (void)ptr;
    (void)keepAlive;
}

void RegisterAreaMemoryBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    mla_benchmark_t benchmark = mla_benchmark("AreaMemoryAlloc100x32", benchmark_category, MlaAreaMemoryBenchmark, MlaAreaMemoryBenchmark_Setup, MlaAreaMemoryBenchmark_TearDown);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("DefaultMemoryAlloc100x32", benchmark_category, MlaDefaultMemoryBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}

#endif
