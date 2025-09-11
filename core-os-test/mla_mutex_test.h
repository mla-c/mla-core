//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_MUTEX_TEST_H
#define COREOS_MLA_MUTEX_TEST_H

#include "../core-os/task/mla_mutx.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

#define mla_mutex_test_default_timeout 25

void MutexTest() {

     mla_mutex_t mutex = mla_mutex_create("Test");

     assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Mutex should be locked successfully");
     assert_false(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Mutex should not be locked while already locked");
     assert_true(mla_mutex_unlock(mutex), "Mutex should be unlocked successfully");

     assert_true(mla_mutex_lock_timeout(mutex, mla_mutex_test_default_timeout), "Mutex should be locked successfully");
     assert_true(mla_mutex_unlock(mutex), "Mutex should be unlocked successfully");
}

void RegisterMutexTests(mla_test_executor_t &p_TestExecutor) {

     mla_test_t test = mla_test("MutexTest", test_category, MutexTest);
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
