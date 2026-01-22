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

    // Define colors
    mla_ui_surface_draw_command_color_t bg_main = {243, 243, 243, 255};        // #F3F3F3
    mla_ui_surface_draw_command_color_t white = {255, 255, 255, 255};          // #FFFFFF
    mla_ui_surface_draw_command_color_t card_bg = {250, 250, 250, 255};        // #FAFAFA
    mla_ui_surface_draw_command_color_t text_primary = {31, 31, 31, 255};      // #1F1F1F
    mla_ui_surface_draw_command_color_t text_secondary = {94, 94, 94, 255};    // #5E5E5E
    mla_ui_surface_draw_command_color_t btn_primary = {0, 120, 212, 255};      // #0078D4
    mla_ui_surface_draw_command_color_t close_red = {196, 43, 28, 255};        // #C42B1C
    mla_ui_surface_draw_command_color_t minimize_yellow = {249, 168, 37, 255}; // #F9A825
    mla_ui_surface_draw_command_color_t maximize_green = {76, 175, 80, 255};   // #4CAF50
    mla_ui_surface_draw_command_color_t transparent = {0, 0, 0, 0};

    // Background
    mla_ui_surface_draw_command_t bg = mla_ui_surface_draw_command_initializer_t::init();
    bg.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    bg.rect = {0, 0, 800, 500, 12, 12, bg_main, transparent, 0};
    mla_array_list_add(drawCommands, bg);

    // Title Bar
    mla_ui_surface_draw_command_t title_bar = mla_ui_surface_draw_command_initializer_t::init();
    title_bar.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    title_bar.rect = {0, 0, 800, 44, 12, 12, white, transparent, 0};
    mla_array_list_add(drawCommands, title_bar);

    mla_ui_surface_draw_command_t title_bar_fill = mla_ui_surface_draw_command_initializer_t::init();
    title_bar_fill.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    title_bar_fill.rect = {0, 32, 800, 12, 0, 0, white, transparent, 0};
    mla_array_list_add(drawCommands, title_bar_fill);

    // App Title
    mla_ui_surface_draw_command_t app_title = mla_ui_surface_draw_command_initializer_t::init();
    app_title.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    app_title.text = {20, 28, mla_string_const("Sample Fluent App"), mla_string_const("Segoe UI"), 14, text_primary};
    mla_array_list_add(drawCommands, app_title);

    // Window Controls
    mla_ui_surface_draw_command_t close_btn = mla_ui_surface_draw_command_initializer_t::init();
    close_btn.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE;
    close_btn.circle = {740, 22, 6, close_red, transparent, 0};
    mla_array_list_add(drawCommands, close_btn);

    mla_ui_surface_draw_command_t minimize_btn = mla_ui_surface_draw_command_initializer_t::init();
    minimize_btn.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE;
    minimize_btn.circle = {760, 22, 6, minimize_yellow, transparent, 0};
    mla_array_list_add(drawCommands, minimize_btn);

    mla_ui_surface_draw_command_t maximize_btn = mla_ui_surface_draw_command_initializer_t::init();
    maximize_btn.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE;
    maximize_btn.circle = {780, 22, 6, maximize_green, transparent, 0};
    mla_array_list_add(drawCommands, maximize_btn);

    // Sidebar
    mla_ui_surface_draw_command_t sidebar = mla_ui_surface_draw_command_initializer_t::init();
    sidebar.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    sidebar.rect = {0, 44, 200, 456, 0, 0, white, transparent, 0};
    mla_array_list_add(drawCommands, sidebar);

    // Sidebar Items
    mla_ui_surface_draw_command_t sidebar_item1 = mla_ui_surface_draw_command_initializer_t::init();
    sidebar_item1.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    sidebar_item1.text = {20, 80, mla_string_const("Dashboard"), mla_string_const("Segoe UI"), 12, text_primary};
    mla_array_list_add(drawCommands, sidebar_item1);

    mla_ui_surface_draw_command_t sidebar_item2 = mla_ui_surface_draw_command_initializer_t::init();
    sidebar_item2.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    sidebar_item2.text = {20, 110, mla_string_const("Reports"), mla_string_const("Segoe UI"), 12, text_secondary};
    mla_array_list_add(drawCommands, sidebar_item2);

    mla_ui_surface_draw_command_t sidebar_item3 = mla_ui_surface_draw_command_initializer_t::init();
    sidebar_item3.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    sidebar_item3.text = {20, 140, mla_string_const("Settings"), mla_string_const("Segoe UI"), 12, text_secondary};
    mla_array_list_add(drawCommands, sidebar_item3);

    // Main Content Area
    mla_ui_surface_draw_command_t main_content = mla_ui_surface_draw_command_initializer_t::init();
    main_content.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    main_content.rect = {220, 64, 560, 416, 8, 8, white, transparent, 0};
    mla_array_list_add(drawCommands, main_content);

    // Header
    mla_ui_surface_draw_command_t header = mla_ui_surface_draw_command_initializer_t::init();
    header.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    header.text = {240, 100, mla_string_const("Overview"), mla_string_const("Segoe UI"), 20, text_primary};
    mla_array_list_add(drawCommands, header);

    // Card 1
    mla_ui_surface_draw_command_t card1 = mla_ui_surface_draw_command_initializer_t::init();
    card1.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    card1.rect = {240, 120, 240, 120, 8, 8, card_bg, transparent, 0};
    mla_array_list_add(drawCommands, card1);

    mla_ui_surface_draw_command_t card1_title = mla_ui_surface_draw_command_initializer_t::init();
    card1_title.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    card1_title.text = {260, 150, mla_string_const("Statistics"), mla_string_const("Segoe UI"), 14, text_primary};
    mla_array_list_add(drawCommands, card1_title);

    mla_ui_surface_draw_command_t card1_desc = mla_ui_surface_draw_command_initializer_t::init();
    card1_desc.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    card1_desc.text = {260, 180, mla_string_const("Data summary goes here"), mla_string_const("Segoe UI"), 12, text_secondary};
    mla_array_list_add(drawCommands, card1_desc);

    // Card 2
    mla_ui_surface_draw_command_t card2 = mla_ui_surface_draw_command_initializer_t::init();
    card2.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    card2.rect = {500, 120, 240, 120, 8, 8, card_bg, transparent, 0};
    mla_array_list_add(drawCommands, card2);

    mla_ui_surface_draw_command_t card2_title = mla_ui_surface_draw_command_initializer_t::init();
    card2_title.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    card2_title.text = {520, 150, mla_string_const("Activity"), mla_string_const("Segoe UI"), 14, text_primary};
    mla_array_list_add(drawCommands, card2_title);

    mla_ui_surface_draw_command_t card2_desc = mla_ui_surface_draw_command_initializer_t::init();
    card2_desc.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    card2_desc.text = {520, 180, mla_string_const("Recent actions listed here"), mla_string_const("Segoe UI"), 12, text_secondary};
    mla_array_list_add(drawCommands, card2_desc);

    // Primary Button
    mla_ui_surface_draw_command_t btn = mla_ui_surface_draw_command_initializer_t::init();
    btn.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    btn.rect = {240, 270, 140, 36, 6, 6, btn_primary, transparent, 0};
    mla_array_list_add(drawCommands, btn);

    mla_ui_surface_draw_command_t btn_text = mla_ui_surface_draw_command_initializer_t::init();
    btn_text.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    btn_text.text = {270, 294, mla_string_const("Get Started"), mla_string_const("Segoe UI"), 12, white};
    mla_array_list_add(drawCommands, btn_text);

    // Scale to current surface size
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(g_main_app_window_ui_surface);
    mla_double_t scaleX = (mla_double_t)surfaceSize.width / 800.0f;
    mla_double_t scaleY = (mla_double_t)surfaceSize.height / 500.0f;
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