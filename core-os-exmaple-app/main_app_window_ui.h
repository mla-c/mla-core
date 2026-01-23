//
// Created by chris on 1/21/2026.
//

#ifndef MAIN_APP_WINDOW_UI_H
#define MAIN_APP_WINDOW_UI_H

#include "../core-os/ui/controls/mla_ui_control.h"
#include "../core-os/ui/surfaces/mla_ui_surface.h"
#include "../core-os/ui/controls/mla_ui_label.h"

static mla_ui_surface_t g_main_app_window_ui_surface = mla_ui_surface_invalid();

inline mla_ui_surface_draw_size_t __main_app_window_render_task_calc_text_size(const mla_string_t &fontFamily, mla_double_t fontSize, const mla_string_t &text) {

    if (g_main_app_window_ui_surface.calc_text_size == nullptr) {
        return {0, 0};
    }

    return g_main_app_window_ui_surface.calc_text_size(g_main_app_window_ui_surface, fontFamily, fontSize, text);
}

inline mla_task_process_result_state __main_app_window_render_task(mla_callback_userdata userData) {
    (void) userData;

    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t> drawCommands = mla_array_list<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();

    // Scale to current surface size
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(g_main_app_window_ui_surface);

    mla_ui_control_context_t context = mla_ui_control_context(surfaceSize.width, surfaceSize.height, __main_app_window_render_task_calc_text_size);

    // Create example labels
    mla_ui_control_t titleLabel = mla_ui_label();
    mla_ui_label_set_text(titleLabel, mla_string_const("Main Header Label"));
    mla_ui_label_set_font_size(titleLabel, MLA_UI_FONT_SIZE_PAGE_TITLE);
    mla_ui_label_set_text_kind(titleLabel, MLA_UI_TEXT_KIND_PRIMARY);
    titleLabel.layout.x = 20;
    titleLabel.layout.y = 20;
    titleLabel.layout.width = 400;
    titleLabel.layout.height = 40;
    mla_ui_control_render_to_draw_commands(context, titleLabel, drawCommands);

    mla_ui_control_t subTitleLabel = mla_ui_label();
    mla_ui_label_set_text(subTitleLabel, mla_string_const("Secondary Subtitle Style"));
    mla_ui_label_set_font_size(subTitleLabel, MLA_UI_FONT_SIZE_SECTION);
    mla_ui_label_set_text_kind(subTitleLabel, MLA_UI_TEXT_KIND_SECONDARY);
    subTitleLabel.layout.x = 20;
    subTitleLabel.layout.y = 70;
    subTitleLabel.layout.width = 400;
    subTitleLabel.layout.height = 30;
    mla_ui_control_render_to_draw_commands(context, subTitleLabel, drawCommands);

    mla_ui_control_t standardLabel = mla_ui_label();
    mla_ui_label_set_text(standardLabel, mla_string_const("Standard body text example at size 14."));
    mla_ui_label_set_font_size(standardLabel, MLA_UI_FONT_SIZE_DEFAULT);
    standardLabel.layout.x = 20;
    standardLabel.layout.y = 110;
    standardLabel.layout.width = 400;
    standardLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, standardLabel, drawCommands);

    mla_ui_control_t errorLabel = mla_ui_label();
    mla_ui_label_set_text(errorLabel, mla_string_const("Error: Connection timeout occurred"));
    mla_ui_label_set_text_kind(errorLabel, MLA_UI_TEXT_KIND_ERROR);
    errorLabel.layout.x = 20;
    errorLabel.layout.y = 140;
    errorLabel.layout.width = 400;
    errorLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, errorLabel, drawCommands);

    mla_ui_control_t successLabel = mla_ui_label();
    mla_ui_label_set_text(successLabel, mla_string_const("Success: File saved successfully"));
    mla_ui_label_set_text_kind(successLabel, MLA_UI_TEXT_KIND_SUCCESS);
    successLabel.layout.x = 20;
    successLabel.layout.y = 170;
    successLabel.layout.width = 400;
    successLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, successLabel, drawCommands);

    mla_ui_control_t linkLabel = mla_ui_label();
    mla_ui_label_set_text(linkLabel, mla_string_const("Click here for more details"));
    mla_ui_label_set_text_kind(linkLabel, MLA_UI_TEXT_KIND_LINK);
    linkLabel.layout.x = 20;
    linkLabel.layout.y = 200;
    linkLabel.layout.width = 400;
    linkLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, linkLabel, drawCommands);

    mla_ui_control_t disabledLabel = mla_ui_label();
    mla_ui_label_set_text(disabledLabel, mla_string_const("Disabled Text Example"));
    mla_ui_label_set_text_kind(disabledLabel, MLA_UI_TEXT_KIND_DISABLED);
    disabledLabel.layout.x = 20;
    disabledLabel.layout.y = 230;
    disabledLabel.layout.width = 400;
    disabledLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, disabledLabel, drawCommands);

    mla_ui_control_t linkDisabledLabel = mla_ui_label();
    mla_ui_label_set_text(linkDisabledLabel, mla_string_const("Disabled Link Example"));
    mla_ui_label_set_text_kind(linkDisabledLabel, MLA_UI_TEXT_KIND_LINK_DISABLED);
    linkDisabledLabel.layout.x = 20;
    linkDisabledLabel.layout.y = 260;
    linkDisabledLabel.layout.width = 400;
    linkDisabledLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, linkDisabledLabel, drawCommands);

    mla_ui_control_t warningLabel = mla_ui_label();
    mla_ui_label_set_text(warningLabel, mla_string_const("Warning: Battery low"));
    mla_ui_label_set_text_kind(warningLabel, MLA_UI_TEXT_KIND_WARNING);
    warningLabel.layout.x = 20;
    warningLabel.layout.y = 290;
    warningLabel.layout.width = 400;
    warningLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, warningLabel, drawCommands);

    mla_ui_control_t infoLabel = mla_ui_label();
    mla_ui_label_set_text(infoLabel, mla_string_const("Info: System normal"));
    mla_ui_label_set_text_kind(infoLabel, MLA_UI_TEXT_KIND_INFO);
    infoLabel.layout.x = 20;
    infoLabel.layout.y = 320;
    infoLabel.layout.width = 400;
    infoLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, infoLabel, drawCommands);

    mla_ui_control_t customLabel = mla_ui_label();
    mla_ui_label_set_text(customLabel, mla_string_const("Custom: Specific Hex Color"));
    mla_ui_label_set_text_kind(customLabel, MLA_UI_TEXT_KIND_CUSTOM);
    mla_ui_label_set_custom_color(customLabel, mla_string_const("#E040FB"));
    customLabel.layout.x = 20;
    customLabel.layout.y = 350;
    customLabel.layout.width = 400;
    customLabel.layout.height = 20;
    mla_ui_control_render_to_draw_commands(context, customLabel, drawCommands);


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