//
// Created by chris on 9/14/2025.
//

#ifndef COREOS_MLA_TASK_CLI_MODULE_TEST_H
#define COREOS_MLA_TASK_CLI_MODULE_TEST_H

#include "../core-os/task/mla_task_manager.h"
#include "../core-os/task/mla_task_cli_module.h"
#include "../core-os-test-support/mla_test_executor.h"

inline void ListTaskCliTaskTest() {

    mla_cli_module_t module = mla_task_cli_module_create();
    const mla_cli_command_t* cmdList = mla_cli_module_find_command(module, mla_string_const("list"));
    assert_not_null(cmdList, "List command should not be null");

    // Create a task to be listed

    // Execute the list command by call the execute and verify the output


}

inline void KillCliTaskTest() {

    mla_cli_module_t module = mla_task_cli_module_create();
    const mla_cli_command_t* cmdKill = mla_cli_module_find_command(module, mla_string_const("kill"));
    assert_not_null(cmdKill, "Kill command should not be null");

    // Create a task to kill

    // Execute the kill command by call the execute and verify the output

}

void RegisterTaskCliModuleTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("List", test_category, ListTaskCliTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Kill", test_category, KillCliTaskTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif