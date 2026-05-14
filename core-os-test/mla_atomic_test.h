//
// Created by chris on 11/11/2025.
//

#ifndef MLA_ATOMIC_TEST_H
#define MLA_ATOMIC_TEST_H

#include "../core-os/task/mla_atomic.h"
#include "../core-os-test-support/mla_test_executor.h"

void AtomicIncrementTest() {
    mla_atomic_int32_t v = mla_atomic_int32(0);
    mla_atomic_increment(v);
    assert_equal(v.value, (mla_int32_t)1, "increment");
}

void AtomicDecrementTest() {
    mla_atomic_int32_t v = mla_atomic_int32(2);
    mla_atomic_decrement(v);
    assert_equal(v.value, (mla_int32_t)1, "decrement");
}

void AtomicAddTest() {
    mla_atomic_int32_t v = mla_atomic_int32(5);
    mla_atomic_add(v, 7);
    assert_equal(v.value, (mla_int32_t)12, "add");
}

void AtomicSubtractTest() {
    mla_atomic_int32_t v = mla_atomic_int32(10);
    mla_atomic_subtract(v, 3);
    assert_equal(v.value, (mla_int32_t)7, "subtract");
}

void AtomicExchangeTest() {
    mla_atomic_int32_t v = mla_atomic_int32(11);
    mla_atomic_exchange(v, 42);
    assert_equal(v.value, (mla_int32_t)42, "exchange");
}

void AtomicCompareExchangeSuccessTest() {
    mla_atomic_int32_t v = mla_atomic_int32(100);
    mla_bool_t ok = mla_atomic_compare_exchange(v, 100, 200);
    assert_true(ok, "compare_exchange success flag");
    assert_equal(v.value, (mla_int32_t)200, "compare_exchange success value");
}

void AtomicCompareExchangeFailTest() {
    mla_atomic_int32_t v = mla_atomic_int32(300);
    mla_bool_t ok = mla_atomic_compare_exchange(v, 100, 400);
    assert_false(ok, "compare_exchange fail flag");
    assert_equal(v.value, (mla_int32_t)300, "compare_exchange fail value unchanged");
}

void RegisterAtomicTests(mla_test_executor_t& exec) {

    mla_test_t test = mla_test("AtomicIncrement", "atomic", AtomicIncrementTest);
    mla_test_executor_register_test(exec, test);

    test = mla_test("AtomicDecrement", "atomic", AtomicDecrementTest);
    mla_test_executor_register_test(exec, test);

    test = mla_test("AtomicAdd", "atomic", AtomicAddTest);
    mla_test_executor_register_test(exec, test);

    test = mla_test("AtomicSubtract", "atomic", AtomicSubtractTest);
    mla_test_executor_register_test(exec, test);

    test = mla_test("AtomicExchange", "atomic", AtomicExchangeTest);
    mla_test_executor_register_test(exec, test);

    test = mla_test("AtomicCompareExchangeSuccess", "atomic", AtomicCompareExchangeSuccessTest);
    mla_test_executor_register_test(exec, test);

    test = mla_test("AtomicCompareExchangeFail", "atomic", AtomicCompareExchangeFailTest);
    mla_test_executor_register_test(exec, test);
}

static inline void bench_touch(mla_int32_t v) {
    (void)v;
}

void AtomicIncrementBenchmark() {
    mla_atomic_int32_t v = mla_atomic_int32(0);
    mla_atomic_increment(v);
    bench_touch(v.value);
}

void AtomicDecrementBenchmark() {
    mla_atomic_int32_t v = mla_atomic_int32(0);
    mla_atomic_decrement(v);
    bench_touch(v.value);
}

void AtomicAddBenchmark() {
    mla_atomic_int32_t v = mla_atomic_int32(0);
    mla_atomic_add(v, 1);
    bench_touch(v.value);
}

void AtomicSubtractBenchmark() {
    mla_atomic_int32_t v = mla_atomic_int32(1);
    mla_atomic_subtract(v, 1);
    bench_touch(v.value);
}

void AtomicExchangeBenchmark() {
    mla_atomic_int32_t v = mla_atomic_int32(0);
    mla_atomic_exchange(v, 5);
    bench_touch(v.value);
}

void AtomicCompareExchangeBenchmark() {
    mla_atomic_int32_t v = mla_atomic_int32(0);
    mla_atomic_compare_exchange(v, v.value, v.value + 1);
    bench_touch(v.value);
}

void RegisterAtomicBenchmarks(mla_benchmark_executor_t &executor) {

    mla_benchmark_t bm = mla_benchmark("AtomicIncrement", benchmark_category, AtomicIncrementBenchmark);
    mla_benchmark_executor_register(executor, bm);

    bm = mla_benchmark("AtomicDecrement", benchmark_category, AtomicDecrementBenchmark);
    mla_benchmark_executor_register(executor, bm);

    bm = mla_benchmark("AtomicAdd", benchmark_category, AtomicAddBenchmark);
    mla_benchmark_executor_register(executor, bm);

    bm = mla_benchmark("AtomicSubtract", benchmark_category, AtomicSubtractBenchmark);
    mla_benchmark_executor_register(executor, bm);

    bm = mla_benchmark("AtomicExchange", benchmark_category, AtomicExchangeBenchmark);
    mla_benchmark_executor_register(executor, bm);

    bm = mla_benchmark("AtomicCompareExchange", benchmark_category, AtomicCompareExchangeBenchmark);
    mla_benchmark_executor_register(executor, bm);
}

#endif