//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_MLA_BUFFER_TEST_H
#define COREOS_MLA_BUFFER_TEST_H

#include "../core-os/memory/mla_memory_hook.h"
#include "../core-os/system/mla_buffer.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

struct my_native_auto_buffer_test_t {
    mla_test_int8_t data[128];
};

struct my_native_buffer_test_t {
    mla_pointer_t data;
};

struct my_buffer_test_t {
    mla_pointer_t data;
    mla_buffer_reference_t dataOwner;
};

static mla_bool_t is_buffer_destroyed = false;

mla_bool_t my_buffer_malloc_hook(mla_size_t size, mla_pointer_t* out_ptr) {
    (void)size;
    (void)out_ptr;
    return false;
}

mla_bool_t my_buffer_free_hook(mla_pointer_t ptr) {
    (void)ptr;
    is_buffer_destroyed = true;
    return false;
}

void SimpleReleaseTest() {

    is_buffer_destroyed = false;
    mla_memory_hook_t hook = mla_memory_hook_install(my_buffer_malloc_hook, my_buffer_free_hook);

    mla_pointer_t data = mla_malloc(64);

    my_buffer_test_t container = {data, mla_buffer(data)};

    if (container.dataOwner.buffer != nullptr) {
        assert_equal(container.dataOwner.buffer->refCount.value, (mla_int32_t)1, "Reference count should be 1 after creation");
    } else {
        assert_fail("Data buffer should not be null after creation");
    }

    assert_false(is_buffer_destroyed, "Buffer should not be destroyed yet");

    {
        my_buffer_test_t container2 = container; // Copy constructor

        if (container2.dataOwner.buffer != nullptr) {
            assert_equal(container2.dataOwner.buffer->refCount.value, (mla_int32_t)2, "Reference count should be 2 after copy");
        } else {
            assert_fail("Data buffer should not be null after copy");
        }

        assert_false(is_buffer_destroyed, "Buffer should not be destroyed yet");
    }

    if (container.dataOwner.buffer != nullptr) {
        assert_equal(container.dataOwner.buffer->refCount.value, (mla_int32_t)1, "Reference count should be 1 after copy goes out of scope");
    } else {
        assert_fail("Data buffer should not be null after copy goes out of scope");
    }

    container = {nullptr, mla_buffer_reference_noOwner()}; // Clear the container
    assert_true(is_buffer_destroyed, "Buffer should be destroyed after clearing the container");

    mla_memory_hook_uninstall(hook); // Uninstall the memory hook

}

static mla_callback_userdata my_test_user_data = 0;

mla_buffer_cleanup_mode my_test_cleanup_hook(mla_pointer_t p_Data, mla_callback_userdata p_UserData) {
    (void)p_Data;
    my_test_user_data = p_UserData;
    return CLEAN_UP_NEEDED;
}

void CleanUpHookTests() {

    mla_pointer_t data = mla_malloc(64);
    my_buffer_test_t container = {data, mla_buffer(data, my_test_cleanup_hook, 42)};

    if (container.dataOwner.buffer != nullptr) {
        assert_equal(container.dataOwner.buffer->refCount.value, (mla_int32_t)1, "Reference count should be 1 after creation");
    } else {
        assert_fail("Data buffer should not be null after creation");
    }

    my_test_user_data = 0; // Reset user data before cleanup
    container = {nullptr, mla_buffer_reference_noOwner()}; // Clear the container
    assert_equal(my_test_user_data, (mla_callback_userdata)42, "User data should be passed to cleanup hook");

}


void RegisterBufferTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleReleaseTest", test_category, SimpleReleaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CleanUpHookTests", test_category, CleanUpHookTests);
    mla_test_executor_register_test(p_TestExecutor, test);

}


void BufferMemoryManagementBenchmark() {

    mla_pointer_t data = mla_malloc(64);
    my_buffer_test_t container1 = {data, mla_buffer(data)};
    my_buffer_test_t container2 = container1;
    my_buffer_test_t container3 = container2;

    container1 = {nullptr, mla_buffer_reference_noOwner()}; // Clear container1
    container2 = {nullptr, mla_buffer_reference_noOwner()}; // Clear container2
    container3 = {nullptr, mla_buffer_reference_noOwner()}; // Clear container3
}

void NativeMemoryManagementBenchmark() {

    mla_pointer_t data = mla_malloc(64);
    my_native_buffer_test_t container1 = {data};
    my_native_buffer_test_t container2 = container1;
    my_native_buffer_test_t container3 = container2;

    // Clear all containers
    container1 = {nullptr};
    container2 = {nullptr};
    container3 = {nullptr};

    // Free the memory
    mla_free(data);
}

void NativeAutomaticMemoryManagementBenchmark() {

    my_native_auto_buffer_test_t container1 = {{0}}; // Initialize with zeros
    my_native_auto_buffer_test_t container2 = container1;
    my_native_auto_buffer_test_t container3 = container2;

    // Clear all containers
    container1 = {{0}}; // Reset to zeros
    container2 = {{0}};
    container3 = {{0}};

}

void RegisterBufferBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("Mla_BufferMemoryBenchmark", benchmark_category, BufferMemoryManagementBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("NativeMemoryBenchmark", benchmark_category, NativeMemoryManagementBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("NativeAutoMemoryBenchmark", benchmark_category, NativeAutomaticMemoryManagementBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

}

#endif
