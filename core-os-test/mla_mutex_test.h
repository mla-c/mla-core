//
// Created by christian on 8/10/25.
//

#ifndef MLA_C_MLA_MUTEX_TEST_H
#define MLA_C_MLA_MUTEX_TEST_H

#include "../core-os/task/mla_mutx.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

#define mla_mutex_test_default_timeout 25

void MutexTest() {
    mla_mutex_t mutex = mla_mutex_create_non_recursive("Test");

    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Mutex should be locked successfully");
    assert_false(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Mutex should not be locked while already locked");
    assert_true(mla_mutex_unlock(mutex), "Mutex should be unlocked successfully");

    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Mutex should be locked successfully");
    assert_true(mla_mutex_unlock(mutex), "Mutex should be unlocked successfully");
}

void MutexRecursiveTest() {
    mla_mutex_t mutex = mla_mutex_create("RecursiveTest");

    // First lock should succeed
    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Recursive mutex should be locked successfully on first lock");

    // Second lock on same thread should also succeed (recursive behavior)
    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Recursive mutex should be locked successfully on second lock");

    // Third lock to verify multiple recursive locks are supported
    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Recursive mutex should be locked successfully on third lock");

    // Unlock must be called once for each lock
    assert_true(mla_mutex_unlock(mutex), "Recursive mutex should be unlocked successfully on first unlock");
    assert_true(mla_mutex_unlock(mutex), "Recursive mutex should be unlocked successfully on second unlock");
    assert_true(mla_mutex_unlock(mutex), "Recursive mutex should be unlocked successfully on third unlock");

    // After all unlocks, mutex should be lockable again
    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Recursive mutex should be lockable again after full unlock");
    assert_true(mla_mutex_unlock(mutex), "Recursive mutex should be unlocked successfully after re-lock");
}

void MutexRecursiveSingleLockUnlockTest() {
    mla_mutex_t mutex = mla_mutex_create("RecursiveSingleTest");

    // Recursive mutex should behave like a normal mutex when locked/unlocked once
    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Recursive mutex should be locked successfully");
    assert_true(mla_mutex_unlock(mutex), "Recursive mutex should be unlocked successfully");

    // Should be re-lockable after unlock
    assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Recursive mutex should be re-locked successfully");
    assert_true(mla_mutex_unlock(mutex), "Recursive mutex should be re-unlocked successfully");
}

void RegisterMutexTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("MutexTest", test_category, MutexTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MutexRecursiveTest", test_category, MutexRecursiveTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MutexRecursiveSingleLockUnlockTest", test_category, MutexRecursiveSingleLockUnlockTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

void MutexCreateDestroyBenchmark() {
    mla_mutex_t mutex = mla_mutex_create("Test");
}

static mla_mutex_t MutexLockBenchmarkLock = mla_mutex_create("BenchmarkLock");

void MutexLockUnlockBenchmark() {
    mla_mutex_lock(MutexLockBenchmarkLock);
    mla_mutex_unlock(MutexLockBenchmarkLock);
}

void RegisterMutexBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    mla_benchmark_t benchmark = mla_benchmark("MutexCreate", benchmark_category, MutexCreateDestroyBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("MutexLockUnlock", benchmark_category, MutexLockUnlockBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif
