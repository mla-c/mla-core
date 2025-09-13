//
// Created by christian on 9/13/25.
//

#ifndef MAIN_APP_CLI_H
#define MAIN_APP_CLI_H

#include "../core-os/task/mla_task_manager.h"
#include "../core-os/task/mla_task.h"

inline void __cli_task(mla_callback_userdata userdata) {

    (void)userdata;
    mla_info("Welcome to the CommandLine Interface (CLI)");
    mla_info("Please enter a command");



}

inline void main_app_cli_init() {
    // Initialize CLI related components here

    mla_task_t task = mla_task_one_time(mla_string("cli"), __cli_task, 0);
    mla_task_manager_register_task(task);

}

#endif //MAIN_APP_CLI_H
