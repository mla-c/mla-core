//
// Created by chris on 8/3/2025.
//

#ifndef MLA_MEMORY_HOOK_TEST_H
#define MLA_MEMORY_HOOK_TEST_H

#include "../../lib/base-lib/core/memory/mla_memory_hook.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/mla_benchmark_executor.h"

static mla_bool_t m_hook_called = false;

mla_bool_t my_test_malloc_hook(mla_size_t size, mla_platform_pointer_t* out_ptr) {
    (void)out_ptr;
    (void)size;
    m_hook_called = true;
    return false;
}

mla_bool_t my_test_free_hook(mla_platform_pointer_t ptr) {
    (void)ptr;
    m_hook_called = true;
    return false;
}

static mla_bool_t m_hook2_called = false;

mla_bool_t my_test2_malloc_hook(mla_size_t size, mla_platform_pointer_t* out_ptr) {
    (void)out_ptr;
    (void)size;
    m_hook2_called = true;
    return false;
}

mla_bool_t my_test2_free_hook(mla_platform_pointer_t ptr) {
    (void)ptr;
    m_hook2_called = true;
    return false;
}

void InstallAndUninstallMemoryHookTest() {

    m_hook_called = false;
    mla_platform_pointer_t data = mla_platform_malloc(24);
    assert_false(m_hook_called, "Memory hook should not be called on initial malloc");
    mla_platform_free(data);
    assert_false(m_hook_called, "Memory hook should not be called on initial free");

    mla_memory_hook_t hook = mla_memory_hook_install(my_test_malloc_hook, my_test_free_hook);

    m_hook_called = false;
    data = mla_platform_malloc(24);
    assert_true(m_hook_called, "Memory hook should be called on malloc after installation");
    m_hook_called = false;
    mla_platform_free(data);
    assert_true(m_hook_called, "Memory hook should be called on free after installation");


    // Install a second hook
    m_hook2_called = false;
    m_hook_called = false;
    mla_memory_hook_t hook2 = mla_memory_hook_install(my_test2_malloc_hook, my_test2_free_hook);
    data = mla_platform_malloc(24);
    assert_true(m_hook_called, "First memory hook should be called on malloc after second installation");
    assert_true(m_hook2_called, "Second memory hook should be called on malloc after second installation");
    m_hook_called = false;
    m_hook2_called = false;
    mla_platform_free(data);
    assert_true(m_hook_called, "First memory hook should be called on free after second installation");
    assert_true(m_hook2_called, "Second memory hook should be called on free after second installation");


    mla_memory_hook_uninstall(hook2);
    mla_memory_hook_uninstall(hook);
    m_hook_called = false;
    data = mla_platform_malloc(24);
    assert_false(m_hook_called, "Memory hook should not be called after uninstallation");
    mla_platform_free(data);
    assert_false(m_hook_called, "Memory hook should not be called after uninstallation");

}

void RegisterMemoryHookTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("InstallAndUninstall", test_category, InstallAndUninstallMemoryHookTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

void NoMemoryHookBenchmark() {

    mla_platform_pointer_t data = mla_platform_malloc(24);
    mla_test_bench_sink(data);
    mla_platform_free(data);

}

static mla_memory_hook_t hook;
static mla_memory_hook_t hook2;

void SetupMemoryHookBenchmark() {
    hook = mla_memory_hook_install(my_test_malloc_hook, my_test_free_hook);
}

void TeardownMemoryHookBenchmark() {
    mla_memory_hook_uninstall(hook);
}

void MemoryHookBenchmark() {

    mla_platform_pointer_t data = mla_platform_malloc(24);
    mla_test_bench_sink(data);
    mla_platform_free(data);
}

void SetupTwoMemoryHookBenchmark() {
    hook = mla_memory_hook_install(my_test_malloc_hook, my_test_free_hook);
    hook2 = mla_memory_hook_install(my_test2_malloc_hook, my_test2_free_hook);
}

void TeardownTwoMemoryHookBenchmark() {
    mla_memory_hook_uninstall(hook2);
    mla_memory_hook_uninstall(hook);
}

void TwoMemoryHookBenchmark() {


    mla_platform_pointer_t data = mla_platform_malloc(24);
    mla_test_bench_sink(data);
    mla_platform_free(data);

}

void NativeMemoryBenchmark() {

    mla_byte_t* data = static_cast<mla_byte_t*>(mla_test_malloc(24));
    mla_test_bench_sink(data);
    mla_test_free(data);

}

void RegisterMemoryHookBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("NativeMemory", benchmark_category, NativeMemoryBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("NoMemoryHook", benchmark_category, NoMemoryHookBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("OneMemoryHook", benchmark_category, MemoryHookBenchmark, SetupMemoryHookBenchmark, TeardownMemoryHookBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("TwoMemoryHook", benchmark_category, TwoMemoryHookBenchmark, SetupTwoMemoryHookBenchmark, TeardownTwoMemoryHookBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

}

#endif
