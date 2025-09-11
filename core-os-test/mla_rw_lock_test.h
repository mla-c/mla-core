//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_RW_LOCK_TEST_H
#define COREOS_MLA_RW_LOCK_TEST_H

#include "../core-os/task/mla_rw_lock.h"
#include "../core-os-test-support/mla_test_executor.h"

#define mla_rw_lock_test_default_timeout 25

void RwLockReadWriteTest() {

    mla_rw_lock_t lock = mla_rw_lock(mla_string("TestLock"));

    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire read lock");

    // Test read unlock
    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock read lock");

    // Test write lock
    assert_true(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire write lock");

    // Test write unlock
    assert_true(mla_rw_unlock_write(lock), "Failed to unlock write lock");


}

void BlockReaderTest() {

    mla_rw_lock_t lock = mla_rw_lock(mla_string("TestLock"));

    // Try to acquire read lock while write lock is held
    assert_true(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire write lock");

    // This should block until the write lock is released
    assert_false(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Read lock should not be acquired while write lock is held");

    // Release the write lock
    assert_true(mla_rw_unlock_write(lock), "Failed to unlock write lock");

    // Now we can acquire the read lock
    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire read lock after write lock was released");

    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock read lock after write lock was released");
}

void BlockWriterTest() {

    mla_rw_lock_t lock = mla_rw_lock(mla_string("TestLock"));

    // Try to acquire read lock while write lock is held
    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire read lock");

    // This should block until the read lock is released
    assert_false(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Write lock should not be acquired while read lock is held");

    // Release the read lock
    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock read lock");

    // Now we can acquire the write lock
    assert_true(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire write lock after read lock was released");

    assert_true(mla_rw_unlock_write(lock), "Failed to unlock write lock after read lock was released");
}

void BlockWriterMultipleReaderTest() {

    mla_rw_lock_t lock = mla_rw_lock(mla_string("TestLock"));

    // Try to acquire read lock while write lock is held
    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire read lock");
    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire second read lock");

    // This should block until the read lock is released
    assert_false(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Write lock should not be acquired while read lock is held");

    // Release the read lock
    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock read lock");
    assert_false(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Write lock should not be acquired while read lock is held");
    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock read lock");

    // Now we can acquire the write lock
    assert_true(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire write lock after read lock was released");

    assert_true(mla_rw_unlock_write(lock), "Failed to unlock write lock after read lock was released");


}

void OnlyOneWriterTest() {

    mla_rw_lock_t lock = mla_rw_lock(mla_string("TestLock"));
    // Try to acquire write lock
    assert_true(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire write lock");
    // Try to acquire another write lock
    assert_false(mla_rw_lock_write_timout(lock, mla_rw_lock_test_default_timeout), "Write lock should not be acquired while another write lock is held");
    // Release the write lock
    assert_true(mla_rw_unlock_write(lock), "Failed to unlock write lock");


}

void MultipleReaderTest() {

    mla_rw_lock_t lock = mla_rw_lock(mla_string("TestLock"));

    // Try to acquire read lock
    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire read lock");
    // Try to acquire another read lock
    assert_true(mla_rw_lock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to acquire second read lock");
    // Release the read locks
    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock first read lock");
    assert_true(mla_rw_unlock_read_timout(lock, mla_rw_lock_test_default_timeout), "Failed to unlock second read lock");

}

void RegisterReadWriteLockTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("RwLockReadWrite", test_category, RwLockReadWriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BlockReader", test_category, BlockReaderTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BlockWriter", test_category, BlockWriterTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BlockWriterMultipleReader", test_category, BlockWriterMultipleReaderTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("OnlyOneWriter", test_category, OnlyOneWriterTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultipleReader", test_category, MultipleReaderTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

static mla_rw_lock_t ReadLockBenchmarkLock = mla_rw_lock(mla_string_const("BenchmarkLock"));

void ReadLockBenchmark() {

    mla_rw_lock_read_timout(ReadLockBenchmarkLock, mla_rw_lock_test_default_timeout);
    mla_rw_unlock_read_timout(ReadLockBenchmarkLock, mla_rw_lock_test_default_timeout);

}

struct mla_rw_lock_t WriteLockBenchmarkLock = mla_rw_lock(mla_string_const("BenchmarkWriteLock"));

void WriteLockBenchmark() {

    mla_rw_lock_write_timout(WriteLockBenchmarkLock, mla_rw_lock_test_default_timeout);
    mla_rw_unlock_write(WriteLockBenchmarkLock);

}

void RegisterReadWriteLockBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("RwLockReadLock", benchmark_category, ReadLockBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("RwLockWriteLock", benchmark_category, WriteLockBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif
