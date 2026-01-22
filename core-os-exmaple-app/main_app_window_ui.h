//
// Created by chris on 1/21/2026.
//

#ifndef MAIN_APP_WINDOW_UI_H
#define MAIN_APP_WINDOW_UI_H

#include "../core-os/ui/surfaces/mla_ui_surface.h"

static mla_ui_surface_t g_main_app_window_ui_surface = mla_ui_surface_invalid();

inline mla_task_process_result_state __main_app_window_render_task(mla_callback_userdata userData) {

    (void) userData;

    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t> drawCommands = mla_array_list<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();

    // Fluent UI (Modern) Colors
    mla_ui_surface_draw_command_color_t bg_window = {255, 255, 255, 255};      // Pure White background (Mica Alt style)
    mla_ui_surface_draw_command_color_t border_color = {229, 229, 229, 255};   // Very subtle outline
    mla_ui_surface_draw_command_color_t text_primary = {32, 32, 32, 255};      // Primary Content
    mla_ui_surface_draw_command_color_t text_caption = {96, 96, 96, 255};      // Window Title (Grey)
    mla_ui_surface_draw_command_color_t control_icon = {25, 25, 25, 255};      // Window Controls
    mla_ui_surface_draw_command_color_t transparent = {0, 0, 0, 0};

    // 1. Main Window Background & Border
    // Unified background (no separate header bar) to achieve the modern "borderless" look
    mla_ui_surface_draw_command_t window_bg = mla_ui_surface_draw_command_initializer_t::init();
    window_bg.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    window_bg.rect = {0, 0, 800, 600, 8, 8, bg_window, border_color, 1};
    mla_array_list_add(drawCommands, window_bg);

    // 2. Window Caption (Title)
    // Top-left aligned, smaller font, grey color (Segoe UI)
    mla_ui_surface_draw_command_t header_text = mla_ui_surface_draw_command_initializer_t::init();
    header_text.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    // x=16 padding, y=28 (approx vertical center for typical 32-40px titlebar area)
    header_text.text = {16, 26, mla_string_const("Application"), mla_string_const("Segoe UI"), 12, text_caption};
    mla_array_list_add(drawCommands, header_text);

    // 3. Window Controls (Top Right)
    // Manually drawing simple glyphs for Min, Max, Close

    // Minimize (-)
    mla_ui_surface_draw_command_t min_cmd = mla_ui_surface_draw_command_initializer_t::init();
    min_cmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE;
    min_cmd.line = {705, 18, 715, 18, control_icon, 1};
    mla_array_list_add(drawCommands, min_cmd);

    // Maximize (Square)
    mla_ui_surface_draw_command_t max_cmd = mla_ui_surface_draw_command_initializer_t::init();
    max_cmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    max_cmd.rect = {737, 9, 10, 10, 0, 0, transparent, control_icon, 1};
    mla_array_list_add(drawCommands, max_cmd);

    // Close (X)
    mla_ui_surface_draw_command_t close_1 = mla_ui_surface_draw_command_initializer_t::init();
    close_1.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE;
    close_1.line = {775, 10, 785, 20, control_icon, 1};
    mla_array_list_add(drawCommands, close_1);

    mla_ui_surface_draw_command_t close_2 = mla_ui_surface_draw_command_initializer_t::init();
    close_2.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE;
    close_2.line = {785, 10, 775, 20, control_icon, 1};
    mla_array_list_add(drawCommands, close_2);

    // 4. Content Text
    // "Hello World" centered with larger typography
    mla_ui_surface_draw_command_t content_text = mla_ui_surface_draw_command_initializer_t::init();
    content_text.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    content_text.text = {320, 300, mla_string_const("Hello World"), mla_string_const("Segoe UI"), 32, text_primary};
    mla_array_list_add(drawCommands, content_text);

    // Scale to current surface size
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(g_main_app_window_ui_surface);
    mla_double_t scaleX = (mla_double_t)surfaceSize.width / 800.0f;
    mla_double_t scaleY = (mla_double_t)surfaceSize.height / 600.0f;
    mla_ui_surface_draw_commands_scale(drawCommands, scaleX, scaleY);

    mla_ui_surface_render_draw_commands(g_main_app_window_ui_surface, drawCommands);

    mla_array_list_destroy(drawCommands);

    return TASK_PROCESS_RESULT_CONTINUE;
}


inline void main_app_window_ui_init() {

    // Create a windowed application UI here
    g_main_app_window_ui_surface = mla_ui_surface_create();

    if (!mla_ui_surface_is_valid(g_main_app_window_ui_surface)) {
        mla_error("Failed to create main application window UI surface!");
        return;
    }

    mla_ui_surface_set_size(g_main_app_window_ui_surface, {800, 600});

    mla_task_t render_task = mla_task_repeating(
            mla_string_const("MainWindowRender"),
            __main_app_window_render_task,
            0
        );

    // Create rendering thread
    if (!mla_task_manager_register_task(render_task)) {
        mla_error("Failed to register main application window rendering task!");
        return;
    }

    mla_info("Main application window UI surface created successfully.");

}


#endif