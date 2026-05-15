//
// Created by Copilot on 5/15/2026.
//

#ifndef MLA_EXTERNAL_TASK_TEST_H
#define MLA_EXTERNAL_TASK_TEST_H

#include "../core/external_task/mla_external_task.h"
#include "../core-test-support/mla_test_executor.h"
#include "../core-test-support/Test/mla_test.h"

void ExternalTaskCreateInvalidInputTest() {
    mla_external_task_t task = mla_external_task_create(mla_string_empty());
    assert_null(task.native_resource.asPointer, "Task native resource should be null for invalid command");
}

void ExternalTaskCreateAndReadStdOutTest() {

    mla_external_task_t task = mla_external_task_create(mla_string_const("printf 'hello'"));
    assert_not_null(task.native_resource.asPointer, "Task should be created");

    mla_byte_t buffer[8] = {0};
    mla_size_t read = task.std_out.read(task.std_out, 0, 5, buffer);

    assert_equal(read, (mla_size_t)5, "Should read process stdout");
    assert_equal((mla_test_int32_t)mla_memcmp(buffer, "hello", 5), (mla_test_int32_t)0, "Stdout content should match");

    mla_external_task_stop(task);
}

void ExternalTaskWriteStdInAndReadStdOutTest() {

    mla_external_task_t task = mla_external_task_create(mla_string_const("cat"));
    assert_not_null(task.native_resource.asPointer, "Task should be created");

    const mla_char_t* msg = "echo\n";
    mla_size_t written = task.std_in.write(task.std_in, 0, 5, reinterpret_cast<const mla_byte_t*>(msg));
    assert_equal(written, (mla_size_t)5, "Should write all bytes to stdin");

    mla_byte_t buffer[8] = {0};
    mla_size_t read = task.std_out.read(task.std_out, 0, 5, buffer);

    assert_equal(read, (mla_size_t)5, "Should read echoed bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(buffer, "echo\n", 5), (mla_test_int32_t)0, "Echoed data should match");

    mla_external_task_stop(task);
}

void RegisterExternalTaskTests(mla_test_executor_t &runner) {
    mla_test_t test = mla_test("CreateInvalidInput", test_category, ExternalTaskCreateInvalidInputTest);
    mla_test_executor_register_test(runner, test);

    test = mla_test("CreateAndReadStdOut", test_category, ExternalTaskCreateAndReadStdOutTest);
    mla_test_executor_register_test(runner, test);

    test = mla_test("WriteStdInAndReadStdOut", test_category, ExternalTaskWriteStdInAndReadStdOutTest);
    mla_test_executor_register_test(runner, test);
}

#endif
