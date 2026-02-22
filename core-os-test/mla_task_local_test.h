//
// Created by christian on 2/22/26.
//

#ifndef COREOS_MLA_TASK_LOCAL_TEST_H
#define COREOS_MLA_TASK_LOCAL_TEST_H

#include "../core-os/task/mla_task_local.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

void TaskLocalCreateTest() {

    mla_task_local_t local = mla_task_local_create("TestLocal");

    assert_not_null(local.resource, "Task local resource should not be null after creation");
    assert_true(mla_string_length(local.name) > 0, "Task local name should not be empty");
}

void TaskLocalSetGetTest() {

    mla_task_local_t local = mla_task_local_create("TestLocalSetGet");

    mla_int32_t testValue = 42;
    assert_true(mla_task_local_set(local, &testValue), "Task local set should succeed");

    mla_pointer_t result = mla_task_local_get(local);
    assert_not_null(result, "Task local get should return a non-null pointer");

    mla_int32_t* resultValue = static_cast<mla_int32_t*>(result);
    assert_equal(*resultValue, 42, "Task local get should return the value that was set");
}

void TaskLocalOverwriteTest() {

    mla_task_local_t local = mla_task_local_create("TestLocalOverwrite");

    mla_int32_t firstValue = 10;
    mla_int32_t secondValue = 20;

    assert_true(mla_task_local_set(local, &firstValue), "First set should succeed");
    assert_true(mla_task_local_set(local, &secondValue), "Second set should succeed");

    mla_pointer_t result = mla_task_local_get(local);
    mla_int32_t* resultValue = static_cast<mla_int32_t*>(result);
    assert_equal(*resultValue, 20, "Task local should return the overwritten value");
}

void TaskLocalNullDefaultTest() {

    mla_task_local_t local = mla_task_local_create("TestLocalNull");

    mla_pointer_t result = mla_task_local_get(local);
    assert_null(result, "Task local should return null before any value is set");
}

void TaskLocalInvalidTest() {

    mla_task_local_t local = mla_task_local_invalid();

    assert_null(local.resource, "Invalid task local resource should be null");
    assert_false(mla_task_local_set(local, nullptr), "Set on invalid task local should fail");
    assert_null(mla_task_local_get(local), "Get on invalid task local should return null");
}

void RegisterTaskLocalTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("TaskLocalCreate", test_category, TaskLocalCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("TaskLocalSetGet", test_category, TaskLocalSetGetTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("TaskLocalOverwrite", test_category, TaskLocalOverwriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("TaskLocalNullDefault", test_category, TaskLocalNullDefaultTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("TaskLocalInvalid", test_category, TaskLocalInvalidTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

void TaskLocalCreateDestroyBenchmark() {

    mla_task_local_t local = mla_task_local_create("BenchmarkLocal");

}

static mla_task_local_t TaskLocalBenchmarkLocal = mla_task_local_create("BenchmarkSetGet");
static mla_int32_t TaskLocalBenchmarkValue = 42;

void TaskLocalSetGetBenchmark() {

    mla_task_local_set(TaskLocalBenchmarkLocal, &TaskLocalBenchmarkValue);
    mla_task_local_get(TaskLocalBenchmarkLocal);

}

void RegisterTaskLocalBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("TaskLocalCreate", benchmark_category, TaskLocalCreateDestroyBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("TaskLocalSetGet", benchmark_category, TaskLocalSetGetBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

}


#endif
