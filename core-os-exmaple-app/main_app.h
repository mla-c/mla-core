//
// Created by christian on 9/10/25.
//

#ifndef MAIN_APP_H
#define MAIN_APP_H

// Include Global Storage
#include "../core-os/log/mla_logger_console.h"
#include "../core-os/log/mla_logging.h"
#include "main_app_cli.h"
#include "main_app_web_ui.h"
#include "main_app_window_ui.h"
#include "main_app_main_window.h"

int run() {

    // Low Level Setup
    mla_log_to_console_activate();

    mla_info("Starting Test Application...");
    mla_info("Test Application started successfully!");
    main_app_cli_init();

    //mla_ui_control_surface_process_task_t task = main_app_main_window_performance_t::renderer;
    mla_ui_control_surface_process_task_t task = main_app_main_window_t::renderer;

    main_app_web_ui_init(task);
    main_app_window_ui_init(task);

    mla_task_manager_process_all_tasks();

    mla_info("Exiting Test Application...");

    return 0;
}

#endif //MAIN_APP_H