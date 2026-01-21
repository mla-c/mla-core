//
// Created by chris on 1/21/2026.
//

#ifndef MAIN_APP_WINDOW_UI_H
#define MAIN_APP_WINDOW_UI_H

#include "../core-os/ui/surfaces/mla_ui_surface.h"

static mla_ui_surface_t g_main_app_window_ui_surface = mla_ui_surface_invalid();

inline void main_app_window_ui_init() {

    // Create a windowed application UI here
    g_main_app_window_ui_surface = mla_ui_surface_create();

    if (!mla_ui_surface_is_valid(g_main_app_window_ui_surface)) {
        mla_error("Failed to create main application window UI surface!");
        return;
    }

    mla_ui_surface_set_size(g_main_app_window_ui_surface, {800, 600});

    mla_ui_surface_render_svg(g_main_app_window_ui_surface, mla_string_const(
        "<svg width=\"800\" height=\"600\" xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect x=\"0\" y=\"0\" width=\"800\" height=\"600\" fill=\"#282c34\"/>"
        "<text x=\"400\" y=\"300\" font-family=\"Arial\" font-size=\"48\" fill=\"#61dafb\" text-anchor=\"middle\" alignment-baseline=\"middle\">"
        "Main Application Window"
        "</text>"
        "</svg>"
    ));


    mla_info("Main application window UI surface created successfully.");

}


#endif