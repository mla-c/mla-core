//
// Created by chris on 9/14/2025.
//

#ifndef COREOS_MLA_TASK_CLI_MODULE_TEST_H
#define COREOS_MLA_TASK_CLI_MODULE_TEST_H

#include "../core-os/task/mla_task_manager.h"
#include "../core-os/task/mla_task_cli_module.h"
#include "../core-os-test-support/mla_test_executor.h"


// Helper functions for testing
struct TestOutputData {
    mla_byte_t buffer[1024];
    mla_size_t position;
};

inline void test_output_write(mla_callback_userdata userdata, const mla_string_t &data) {
    auto* output = reinterpret_cast<TestOutputData*>(userdata);
    mla_size_t dataLen = data.length;
    if (output->position + dataLen < 1024) {
        mla_memcpy(output->buffer + output->position, data.data, dataLen);
        output->position += dataLen;
        output->buffer[output->position] = '\0';
    }
}

inline void test_output_write_cstring(mla_callback_userdata userdata, const mla_char_t* data) {
    auto* output = reinterpret_cast<TestOutputData*>(userdata);
    mla_size_t dataLen = mla_strlen(data);
    if (output->position + dataLen < 1024) {
        mla_memcpy(output->buffer + output->position, data, dataLen);
        output->position += dataLen;
        output->buffer[output->position] = '\0';
    }
}

inline void test_task_worker(mla_callback_userdata userData) {
    (void)userData;
    // Simple worker that does nothing
}

inline mla_task_process_result_state test_repeating_task_worker(mla_callback_userdata userData) {
    (void)userData;
    return TASK_PROCESS_RESULT_CONTINUE;
}

inline void ListTaskCliTaskTest() {

    mla_cli_module_t module = mla_task_cli_module_create();
    const mla_cli_command_t* cmdList = mla_cli_module_find_command(module, mla_string_const("ls"));
    assert_not_null(cmdList, "List command should not be null");

    // Create a task to be listed
    mla_string_t taskName = mla_string_const("TestListTask");
    mla_task_t task = mla_task_one_time(taskName, test_task_worker, 0);
    assert_true(mla_task_manager_register_task(task), "Task should be registered successfully");

    // Set up output stream
    TestOutputData outputData = {};
    outputData.position = 0;
    outputData.buffer[0] = '\0';

    mla_cli_command_execute_outstream_t outStream = {};
    outStream.userdata = reinterpret_cast<mla_callback_userdata>(&outputData);
    outStream.write = test_output_write;
    outStream.writeCString = test_output_write_cstring;

    // Execute the list command
    mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer> parameters =
        mla_hash_map_empty<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>();

    mla_bool_t result = cmdList->execute(*cmdList, parameters, outStream);
    assert_true(result, "List command should execute successfully");

    // Verify the output contains our task
    mla_string_t output = mla_string(reinterpret_cast<mla_char_t*>(outputData.buffer), outputData.position);
    assert_true(mla_string_contains(output, taskName), "Output should contain the test task name");
    assert_true(mla_string_contains(output, mla_string_const("Listing all tasks:")), "Output should contain listing header");

    // Clean up
    mla_task_manager_cleanup();
}

inline void KillCliTaskTest() {

    mla_cli_module_t module = mla_task_cli_module_create();
    const mla_cli_command_t* cmdKill = mla_cli_module_find_command(module, mla_string_const("kill"));
    assert_not_null(cmdKill, "Kill command should not be null");

    // Create a task to kill
    mla_string_t taskName = mla_string_const("TestKillTask");
    mla_task_t task = mla_task_repeating(taskName, test_repeating_task_worker, 10);
    assert_true(mla_task_manager_register_task(task), "Task should be registered successfully");
    assert_true(mla_task_manager_task_exists(taskName), "Task should exist before kill");

    // Set up output stream
    TestOutputData outputData = {};
    outputData.position = 0;
    outputData.buffer[0] = '\0';

    mla_cli_command_execute_outstream_t outStream = {};
    outStream.userdata = reinterpret_cast<mla_callback_userdata>(&outputData);
    outStream.write = test_output_write;
    outStream.writeCString = test_output_write_cstring;

    // Set up parameters for kill command
    mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer> parameters =
        mla_hash_map_empty<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>();
    mla_hash_map_push(parameters, mla_string_const("name"), taskName);

    // Execute the kill command
    mla_bool_t result = cmdKill->execute(*cmdKill, parameters, outStream);
    assert_true(result, "Kill command should execute successfully");

    // Verify the output indicates successful kill
    mla_string_t output = mla_string(reinterpret_cast<mla_char_t*>(outputData.buffer), outputData.position);
    assert_true(mla_string_contains(output, mla_string_const("Killing task")), "Output should contain killing message");
    assert_true(mla_string_contains(output, taskName), "Output should contain the task name");
    assert_true(mla_string_contains(output, mla_string_const("[OK]")), "Output should indicate success");

    // Verify the task is in aborted state or aborting state
    mla_task_info_t info = mla_task_manager_get_task_info(taskName);
    assert_true((info.state == TASK_STATE_ABORTING) || (info.state == TASK_STATE_ABORTED), "Task should be aborting or aborted");

    // Clean up
    mla_task_manager_cleanup();
}

void RegisterTaskCliModuleTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("List", test_category, ListTaskCliTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Kill", test_category, KillCliTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
