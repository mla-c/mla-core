//
// Created by christian on 9/10/25.
//

#include "mla_test_executor.h"
#include "mla_test_utils.h"
#include "Benchmark/mla_benchmark.h"

mla_test_executor_t mla_test_executor() {

    mla_test_executor_t executor;
    executor.capacity = 10;
    executor.count = 0;
    executor.tests = (mla_test_t*) mla_test_malloc(sizeof(mla_test_t) * executor.capacity);
    for (mla_test_uint32_t i = 0; i < executor.capacity; ++i) {
        executor.tests[i] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    }
    return executor;
}

void mla_test_executor_destroy(mla_test_executor_t &executor) {
    for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
        if (executor.tests[i].name) {
            mla_test_destroy(executor.tests[i]);
        }
    }
    mla_test_free(executor.tests);
    executor.tests = nullptr;
    executor.capacity = 0;
    executor.count = 0;
}

mla_test_int32_t mla_test_executor_run_test(mla_test_executor_t &executor, mla_test_uint32_t test_number) {

    mla_test_uint32_t test_index = test_number - 1; // Convert to zero-based index

    if (test_index >= executor.count || executor.tests[test_index].name == nullptr) {
        return -1; // Invalid test index
    }

    char buffer[12];
    mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), test_index + 1);
    mla_test_print(buffer, strLength);
    mla_test_print("). ", 3);

    if (mla_test_run(executor.tests[test_index])) {
        return 0; // Test passed
    } else {
        return 1; // Test failed
    }

}

mla_test_int32_t mla_test_executor_run_all_tests(mla_test_executor_t &executor, mla_test_bool_t* p_SuccessMap) {

    mla_test_int32_t failed_tests = 0;

    for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
        if (executor.tests[i].name != nullptr) {

            char buffer[12];
            mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), i + 1);
            mla_test_print(buffer, strLength);
            mla_test_print("). ", 3);

            mla_test_bool_t success = mla_test_run(executor.tests[i]);
            if (!success) {
                failed_tests++;
            }

            if (p_SuccessMap != nullptr) {
                p_SuccessMap[i] = success;
            }
        }
    }

    return failed_tests;
}

void mla_test_executor_register_test(mla_test_executor_t &executor, mla_test_t &test) {
    if (test.name == nullptr) {
        return;
    }

    if (executor.count >= executor.capacity) {
        mla_test_uint32_t increment = (executor.capacity / 4);
        if (increment < 10) increment = 10;
        mla_test_uint32_t newCapacity = executor.capacity + increment;
        mla_test_t* newTests = (mla_test_t*) mla_test_malloc(sizeof(mla_test_t) * newCapacity);

        // Copy existing tests
        for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
            newTests[i] = executor.tests[i];
        }

        // Initialize new slots
        for (mla_test_uint32_t i = executor.count; i < newCapacity; ++i) {
            newTests[i] = {nullptr, nullptr, nullptr, nullptr, nullptr};
        }

        mla_test_free(executor.tests);
        executor.tests = newTests;
        executor.capacity = newCapacity;
    }

    executor.tests[executor.count++] = test;
}

#if (!defined(mla_test_memory) || (mla_test_memory == 1))

#include "../core-os/memory/mla_memory_hook.h"

// State for seed-based allocation failure injection
static mla_test_uint32_t g_mla_test_failure_prng_state = 0;
static mla_test_uint32_t g_mla_test_seed_generator_state = 0;

mla_test_uint32_t mla_test_generate_seed() {
    if (g_mla_test_seed_generator_state == 0) {
        // Initialize state with system time and address
        mla_test_uint64_t system_time = g_benchmark_timer.current_nanoseconds();
        g_mla_test_seed_generator_state = (mla_test_uint32_t)(system_time ^ (mla_test_uint64_t)&g_mla_test_seed_generator_state);
        if (g_mla_test_seed_generator_state == 0) {
            g_mla_test_seed_generator_state = 0xACE1u; // Fallback
        }
    }

    // Xorshift32 algorithm
    g_mla_test_seed_generator_state ^= g_mla_test_seed_generator_state << 13;
    g_mla_test_seed_generator_state ^= g_mla_test_seed_generator_state >> 17;
    g_mla_test_seed_generator_state ^= g_mla_test_seed_generator_state << 5;

    return g_mla_test_seed_generator_state;
}

// Malloc hook that selectively fails allocations based on seed-driven PRNG
static mla_bool_t mla_test_executor_failure_malloc_hook(mla_size_t p_Size, mla_pointer_t* p_OutPtr) {
    (void)p_Size;

    g_mla_test_failure_prng_state = g_mla_test_failure_prng_state * 1664525u + 1013904223u;

    if (((g_mla_test_failure_prng_state >> 16) & 0x7u) == 0u) {
        *p_OutPtr = nullptr;
        return true;
    }

    return false;
}

// Free hook - pass through to original free for all pointers
static mla_bool_t mla_test_executor_failure_free_hook(mla_pointer_t p_Ptr) {
    (void)p_Ptr;
    return false;
}

// Suppresses on_malloc_failure output during allocation failure injection
static void mla_test_executor_on_malloc_failure_noop(mla_size_t p_Size, const mla_char_t* p_Filename, const mla_char_t* p_FunctionName) {
    (void)p_Size;
    (void)p_Filename;
    (void)p_FunctionName;
}

mla_test_int32_t mla_test_executor_run_test_with_allocation_failure(mla_test_executor_t &executor, mla_test_uint32_t p_TestNumber, mla_test_uint32_t p_Seed) {

    mla_test_uint32_t testIndex = p_TestNumber - 1;

    if (testIndex >= executor.count || executor.tests[testIndex].name == nullptr) {
        return -1;
    }

    char buffer[12];
    mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), testIndex + 1);
    mla_test_print(buffer, strLength);
    mla_test_print("). ", 3);

    // Suppress on_malloc_failure during failure injection
    void (*originalOnFailure)(mla_size_t, const mla_char_t*, const mla_char_t*) = g_low_level_access.on_malloc_failure;
    g_low_level_access.on_malloc_failure = mla_test_executor_on_malloc_failure_noop;

    // Reset PRNG state to seed for deterministic behavior
    g_mla_test_failure_prng_state = p_Seed;

    // Install memory failure hook
    mla_memory_hook_t hook = mla_memory_hook_install(
        mla_test_executor_failure_malloc_hook,
        mla_test_executor_failure_free_hook
    );

    mla_test_bool_t result = mla_test_run(executor.tests[testIndex]);

    mla_memory_hook_uninstall(hook);
    g_low_level_access.on_malloc_failure = originalOnFailure;

    return result ? 0 : 1;
}

mla_test_int32_t mla_test_executor_run_all_tests_with_allocation_failure(mla_test_executor_t &executor, mla_test_uint32_t p_Seed, const mla_test_bool_t* p_SuccessMap) {

    mla_test_print("Running Tests with Allocation Failure (Seed: ", 45);
    char seedBuffer[12];
    mla_test_uint32_t seedLen = mla_uint32_to_string(seedBuffer, sizeof(seedBuffer), p_Seed);
    mla_test_print(seedBuffer, seedLen);
    mla_test_print(")...\n", 5);

    // Suppress on_malloc_failure during failure injection
    void (*originalOnFailure)(mla_size_t, const mla_char_t*, const mla_char_t*) = g_low_level_access.on_malloc_failure;
    g_low_level_access.on_malloc_failure = mla_test_executor_on_malloc_failure_noop;

    // Install memory failure hook once for the entire run
    mla_memory_hook_t hook = mla_memory_hook_install(
        mla_test_executor_failure_malloc_hook,
        mla_test_executor_failure_free_hook
    );

    mla_test_int32_t failedTests = 0;

    for (mla_test_uint32_t i = 0; i < executor.count; ++i) {
        if (executor.tests[i].name != nullptr) {

            if (p_SuccessMap != nullptr && !p_SuccessMap[i]) {
                continue;
            }

            // Reset PRNG state for each test to ensure deterministic behavior
            g_mla_test_failure_prng_state = p_Seed;

            char buffer[12];
            mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), i + 1);
            mla_test_print(buffer, strLength);
            mla_test_print("). ", 3);

            if (!mla_test_run(executor.tests[i])) {
                failedTests++;
            }
        }
    }

    mla_memory_hook_uninstall(hook);
    g_low_level_access.on_malloc_failure = originalOnFailure;

    mla_test_print("Tests with Allocation Failure (Seed: ", 37);
    mla_test_print(seedBuffer, seedLen);
    mla_test_print(") completed with ", 17);
    char failBuffer[12];
    mla_test_uint32_t failLen = mla_uint32_to_string(failBuffer, sizeof(failBuffer), (mla_test_uint32_t)failedTests);
    mla_test_print(failBuffer, failLen);
    mla_test_print(" failed tests\n", 14);

    return failedTests;
}

mla_test_int32_t mla_test_executor_run_all_tests_with_generated_allocation_failures(mla_test_executor_t &executor, mla_test_uint32_t p_SeedCount, const mla_test_bool_t* p_SuccessMap) {

    mla_test_int32_t totalFailed = 0;

    for (mla_test_uint32_t i = 1; i <= p_SeedCount; ++i) {
        mla_test_uint32_t seed = mla_test_generate_seed();
        totalFailed += mla_test_executor_run_all_tests_with_allocation_failure(executor, seed, p_SuccessMap);
    }

    mla_test_print("\nAllocation Failure Tests completed with ", 41);
    char buffer[12];
    mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), (mla_test_uint32_t)totalFailed);
    mla_test_print(buffer, strLength);
    mla_test_print(" total failed tests across ", 27);
    strLength = mla_uint32_to_string(buffer, sizeof(buffer), p_SeedCount);
    mla_test_print(buffer, strLength);
    mla_test_print(" seeds\n", 7);

    return totalFailed;
}

#endif
