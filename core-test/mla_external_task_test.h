//
// Created by Copilot on 5/15/2026.
//

#ifndef MLA_EXTERNAL_TASK_TEST_H
#define MLA_EXTERNAL_TASK_TEST_H

#include "../lib/base-lib/core/external_task/mla_external_task.h"
#include "../lib/base-lib/test-support/mla_test_executor.h"
#include "../lib/base-lib/test-support/Test/mla_test.h"

#if defined(_WIN32) || defined(_WIN64)
#define __mla_external_task_test_stdout_cmd "echo|set /p=hello"
// "findstr /R ." matches any non-empty line in regex mode and outputs it.
// Using close_stdin (EOF) to force findstr to flush its buffered stdout.
#define __mla_external_task_test_stdin_echo_cmd "findstr /R ."
#define __mla_external_task_test_sleep_cmd "ping 127.0.0.1 -n 2 >nul"
#else
#define __mla_external_task_test_stdout_cmd "printf 'hello'"
#define __mla_external_task_test_stdin_echo_cmd "cat"
#define __mla_external_task_test_sleep_cmd "sleep 1"
#endif

#define __mla_external_task_test_default_timeout_ms 250

void ExternalTaskCreateInvalidInputTest() {
    mla_external_task_t task = mla_external_task_create(mla_string_empty());
    assert_true(mla_pointer_is_null(task.native_resource), "Task native resource should be null for invalid command");
}

void ExternalTaskCreateAndReadStdOutTest() {

    mla_external_task_t task = mla_external_task_create(mla_string_const(__mla_external_task_test_stdout_cmd));
    assert_false(mla_pointer_is_null(task.native_resource), "Task should be created");

    mla_byte_t buffer[8] = {0};
    mla_size_t read = mla_stream_input_read_with_timeout(task.std_out, 0, 5, buffer, __mla_external_task_test_default_timeout_ms);

    assert_equal(read, (mla_size_t)5, "Should read process stdout");
    assert_equal((mla_test_int32_t)mla_memcmp(buffer, "hello", 5), (mla_test_int32_t)0, "Stdout content should match");

    mla_external_task_stop(task);
}

void ExternalTaskWriteStdInAndReadStdOutTest() {

    mla_external_task_t task = mla_external_task_create(mla_string_const(__mla_external_task_test_stdin_echo_cmd));
    assert_false(mla_pointer_is_null(task.native_resource), "Task should be created");

    const mla_char_t* msg = "echo\n";
    mla_size_t written = mla_stream_output_write_with_timeout(task.std_in, 0, 5, reinterpret_cast<const mla_byte_t*>(msg), __mla_external_task_test_default_timeout_ms);
    assert_equal(written, (mla_size_t)5, "Should write all bytes to stdin");

    // Close stdin to send EOF to the child process.
    // This forces line-buffering commands (e.g. findstr) to flush their stdout and exit.
    mla_external_task_close_stdin(task);

    // Read just the payload word "echo" (4 bytes), ignoring the platform-specific line
    // ending (\n on Linux, \r\n on Windows) that the child appends.
    mla_byte_t buffer[8] = {0};
    mla_size_t read = mla_stream_input_read_with_timeout(task.std_out, 0, 4, buffer, __mla_external_task_test_default_timeout_ms);

    assert_equal(read, (mla_size_t)4, "Should read echoed bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(buffer, "echo", 4), (mla_test_int32_t)0, "Echoed data should match");

    mla_external_task_stop(task);
}

void ExternalTaskStateTest() {

    mla_external_task_t task = mla_external_task_create(mla_string_const(__mla_external_task_test_sleep_cmd));
    assert_false(mla_pointer_is_null(task.native_resource), "Task should be created");

    mla_external_task_state state = mla_external_task_get_state(task);
    assert_equal((mla_test_int32_t)state, (mla_test_int32_t)MLA_EXTERNAL_TASK_STATE_RUNNING, "Task should be running after create");

    mla_sleep(1200);

    state = mla_external_task_get_state(task);
    assert_equal((mla_test_int32_t)state, (mla_test_int32_t)MLA_EXTERNAL_TASK_STATE_STOPPED, "Task should be stopped after command completion");

    mla_external_task_stop(task);
}

void RegisterExternalTaskTests(mla_test_executor_t &runner) {
    mla_test_t test = mla_test("CreateInvalidInput", test_category, ExternalTaskCreateInvalidInputTest);
    mla_test_executor_register_test(runner, test);

    test = mla_test("CreateAndReadStdOut", test_category, ExternalTaskCreateAndReadStdOutTest);
    mla_test_executor_register_test(runner, test);

    test = mla_test("WriteStdInAndReadStdOut", test_category, ExternalTaskWriteStdInAndReadStdOutTest);
    mla_test_executor_register_test(runner, test);

    test = mla_test("State", test_category, ExternalTaskStateTest);
    mla_test_executor_register_test(runner, test);
}

#endif
