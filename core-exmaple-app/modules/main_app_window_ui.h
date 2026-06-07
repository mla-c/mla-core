//
// Created by chris on 1/21/2026.
//

#ifndef MAIN_APP_WINDOW_UI_H
#define MAIN_APP_WINDOW_UI_H

#include "../../lib/base-lib/core/ui/surfaces/mla_ui_surface.h"
#include "../../lib/base-lib/core/ui/controls/mla_ui_button.h"
#include "../../lib/base-lib/core/ui/controls/mla_ui_control_surface.h"
#include "../../lib/base-lib/core/ui/display/mla_ui_display_surface.h"

static mla_ui_surface_t g_main_app_window_ui_surface = mla_ui_surface_invalid();
static mla_ui_control_surface_t g_main_app_window_ui_surface_connector = mla_ui_control_surface_empty();

inline void main_app_window_ui_init(const mla_ui_control_surface_process_task_t &processTask) {

    // Create a windowed application UI here
    g_main_app_window_ui_surface = mla_ui_display_surface_create();

    if (!mla_ui_surface_is_valid(g_main_app_window_ui_surface)) {
        mla_error("Failed to create main application window UI surface!");
        return;
    }

    mla_debug("Start build UI");

    // Build the UI controls
    g_main_app_window_ui_surface_connector = mla_ui_control_surface_create(g_main_app_window_ui_surface, processTask);

    // Start the single-threaded mode for rendering
    mla_ui_control_surface_start_single_threaded_mode(g_main_app_window_ui_surface_connector);

}

#endif
