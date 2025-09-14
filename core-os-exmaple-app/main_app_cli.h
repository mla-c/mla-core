//
// Created by christian on 9/13/25.
//

#ifndef MAIN_APP_CLI_H
#define MAIN_APP_CLI_H

#include "../core-os/task/mla_task_manager.h"
#include "../core-os/task/mla_task.h"
#include "../core-os/cli/mla_cli_app.h"
#include "../core-os/task/mla_task_cli_module.h"
#include "../core-os/system/mla_stream.h"


mla_cli_app_t g_main_app_cli = mla_cli_app_empty();

inline mla_task_process_result_state __cli_task(mla_callback_userdata userdata) {

    (void)userdata;

    mla_cli_app_update_and_process_input(g_main_app_cli, mla_stream_input_stdin(), mla_stream_output_stdout());

    return TASK_PROCESS_RESULT_CONTINUE;
}

inline mla_cli_module_t __cli_build_root_module() {

    mla_cli_module_t rootModule = mla_cli_module(mla_string_const("Root"));

    // Add sub-modules

    // Task Manager
    mla_cli_module_add_sub_module(rootModule, mla_task_cli_module_create());

    return rootModule;
}

inline void main_app_cli_init() {

    // Initialize CLI related components here
    mla_info("Welcome to the CommandLine Interface (CLI)");
    mla_info("Please enter a command:");

    mla_cli_module_t root = __cli_build_root_module();
    g_main_app_cli = mla_cli_app_init(root, mla_stream_output_stdout());

    mla_task_t task = mla_task_repeating(mla_string("cli"), __cli_task, 0);
    mla_task_manager_register_task(task);

}

#endif //MAIN_APP_CLI_H
