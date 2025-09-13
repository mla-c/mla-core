//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_TASK_MANAGER_TEST_H
#define COREOS_MLA_TASK_MANAGER_TEST_H

#include "../core-os/task/mla_task_manager.h"
#include "../core-os-test-support/mla_test_executor.h"

static mla_bool_t OneTimeTaskTestWorkerProcess = false;

void OneTimeTaskTestWorker(mla_callback_userdata userData) {

    (void)userData;

    OneTimeTaskTestWorkerProcess = true;
}

void OneTimeTaskTest() {

    mla_string_t taskName = mla_string_const("OneTimeTask");

    mla_task_t task = mla_task_one_time(taskName, OneTimeTaskTestWorker, 0);
    assert_true(mla_task_manager_register_task(task), "Task should be registered successfully");

    mla_task_info_t info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");

    mla_task_manager_process_all_tasks();

    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");
    assert_equal(info.state, TASK_STATE_COMPLETED, "Task state should be completed");
    assert_true(OneTimeTaskTestWorkerProcess, "OneTimeTaskTestWorker should have been processed");

}

static mla_size_t RepeatingTaskTestWorkerCount = 0;

mla_task_process_result_state RepeatingTaskTestWorker(mla_callback_userdata userData) {

    if (userData == RepeatingTaskTestWorkerCount)
        return TASK_PROCESS_RESULT_DONE;

    RepeatingTaskTestWorkerCount++;
    return TASK_PROCESS_RESULT_CONTINUE;
}

void RepeatingTaskTest() {

    mla_string_t taskName = mla_string_const("RepeatingTask");

    mla_task_t task = mla_task_repeating(taskName, RepeatingTaskTestWorker, 10);

    RepeatingTaskTestWorkerCount = 0;
    assert_true(mla_task_manager_register_task(task), "Task should be registered successfully");

    mla_task_info_t info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");

    mla_task_manager_process_all_tasks();

    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");
    assert_equal(info.state, TASK_STATE_COMPLETED, "Task state should be completed");

    assert_equal(RepeatingTaskTestWorkerCount, (mla_size_t)10, "RepeatingTaskTestWorker should have been called 10 times");

}

void CleanUpTestWorker(mla_callback_userdata userData) {
    (void)userData;

}

void CleanupTest() {

    mla_string_t taskName = mla_string_const("OneTimeTask");

    mla_task_t task = mla_task_one_time(taskName, CleanUpTestWorker, 0);
    assert_true(mla_task_manager_register_task(task), "Task should be registered successfully");

    mla_task_info_t info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");

    mla_task_manager_process_all_tasks();

    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");
    assert_equal(info.state, TASK_STATE_COMPLETED, "Task state should be completed");

    mla_task_manager_cleanup();
    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.state, TASK_STATE_UNKNOWN, "Task state should be completed");
}

mla_task_process_result_state AbortTaskTestWorker(mla_callback_userdata userData) {
    (void)userData;
    return TASK_PROCESS_RESULT_CONTINUE;
}

void AbortTaskTest() {

    mla_string_t taskName = mla_string_const("AbortTask");

    mla_task_t task = mla_task_repeating(taskName, AbortTaskTestWorker, 10);
    assert_true(mla_task_manager_register_task(task), "Task should be registered successfully");

    mla_task_info_t info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.priority, TASK_PRIO_NORMAL, "Task priority should be normal");
    assert_equal(info.stack_size, TASK_STACK_SIZE_DEFAULT, "Task stack size should be default");

    assert_true(mla_task_manager_abort_task(taskName), "Task should be aborted successfully");

    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_true((info.state == TASK_STATE_ABORTING) || (info.state == TASK_STATE_ABORTED), "Task state should be aborting or aborted");

    mla_task_manager_process_all_tasks();

    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.state, TASK_STATE_ABORTED, "Task state should be aborted");

    mla_task_manager_cleanup();

    info = mla_task_manager_get_task_info(taskName);
    assert_struct_equal(mla_string_t, info.name, taskName, "Task name should match");
    assert_equal(info.state, TASK_STATE_UNKNOWN, "Task state should be unkown");
}

void RegisterTaskManagerTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("OneTimeTask", test_category, OneTimeTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RepeatingTask", test_category, RepeatingTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CleanupTask", test_category, CleanupTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AbortTask", test_category, AbortTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
