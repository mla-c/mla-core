//
// Created by chris on 1/21/2026.
//

#ifndef MAIN_APP_WINDOW_UI_H
#define MAIN_APP_WINDOW_UI_H

#include "../core-os/ui/controls/mla_ui_control.h"
#include "../core-os/ui/surfaces/mla_ui_surface.h"
#include "../core-os/ui/controls/mla_ui_label.h"
#include "../core-os/ui/controls/mla_ui_button.h"
#include "../core-os/ui/controls/mla_ui_control_surface.h"

static mla_ui_surface_t g_main_app_window_ui_surface = mla_ui_surface_invalid();
static mla_ui_control_surface_t g_main_app_window_ui_surface_connector = mla_ui_control_surface_empty();

inline void __main_app_primary_button_clicked(mla_ui_control_t &control, const mla_ui_surface_input_event_click_t &clickEvent, mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, mla_callback_userdata userData) {
    (void)control;
    (void)clickEvent;
    (void)uiControls;
    (void)userData;

    mla_ui_control_t disabledButton = mla_ui_control_empty();

    if (!mla_ui_control_find_by_id(uiControls, mla_string_const("primary_disabled_button"), disabledButton)) {
        return;
    }

    mla_ui_button_set_disable(disabledButton, !mla_ui_button_get_disable(disabledButton));
}

inline mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> __main_app_build_ui() {

    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> uiControls = mla_array_list<mla_ui_control_t, mla_ui_control_initializer_t>();

    // Create example labels (Left Column)
    mla_ui_control_t titleLabel = mla_ui_label();
    mla_ui_label_set_text(titleLabel, mla_string_const("Main Header Label"));
    mla_ui_label_set_font_size(titleLabel, MLA_UI_FONT_SIZE_PAGE_TITLE);
    mla_ui_label_set_text_kind(titleLabel, MLA_UI_TEXT_KIND_PRIMARY);
    titleLabel.layout.x = 20;
    titleLabel.layout.y = 20;
    titleLabel.layout.width = 400;
    titleLabel.layout.height = 40;
    mla_array_list_add(uiControls, titleLabel);

    mla_ui_control_t subTitleLabel = mla_ui_label();
    mla_ui_label_set_text(subTitleLabel, mla_string_const("Secondary Subtitle Style"));
    mla_ui_label_set_font_size(subTitleLabel, MLA_UI_FONT_SIZE_SECTION);
    mla_ui_label_set_text_kind(subTitleLabel, MLA_UI_TEXT_KIND_SECONDARY);
    subTitleLabel.layout.x = 20;
    subTitleLabel.layout.y = 70;
    subTitleLabel.layout.width = 400;
    subTitleLabel.layout.height = 30;
    mla_array_list_add(uiControls, subTitleLabel);

    mla_ui_control_t standardLabel = mla_ui_label();
    mla_ui_label_set_text(standardLabel, mla_string_const("Standard body text example at size 14."));
    mla_ui_label_set_font_size(standardLabel, MLA_UI_FONT_SIZE_DEFAULT);
    standardLabel.layout.x = 20;
    standardLabel.layout.y = 110;
    standardLabel.layout.width = 400;
    standardLabel.layout.height = 20;
    mla_array_list_add(uiControls, standardLabel);

    mla_ui_control_t errorLabel = mla_ui_label();
    mla_ui_label_set_text(errorLabel, mla_string_const("Error: Connection timeout occurred"));
    mla_ui_label_set_text_kind(errorLabel, MLA_UI_TEXT_KIND_ERROR);
    errorLabel.layout.x = 20;
    errorLabel.layout.y = 140;
    errorLabel.layout.width = 400;
    errorLabel.layout.height = 20;
    mla_array_list_add(uiControls, errorLabel);

    mla_ui_control_t successLabel = mla_ui_label();
    mla_ui_label_set_text(successLabel, mla_string_const("Success: File saved successfully"));
    mla_ui_label_set_text_kind(successLabel, MLA_UI_TEXT_KIND_SUCCESS);
    successLabel.layout.x = 20;
    successLabel.layout.y = 170;
    successLabel.layout.width = 400;
    successLabel.layout.height = 20;
    mla_array_list_add(uiControls, successLabel);

    mla_ui_control_t linkLabel = mla_ui_label();
    mla_ui_label_set_text(linkLabel, mla_string_const("Click here for more details"));
    mla_ui_label_set_text_kind(linkLabel, MLA_UI_TEXT_KIND_LINK);
    linkLabel.layout.x = 20;
    linkLabel.layout.y = 200;
    linkLabel.layout.width = 400;
    linkLabel.layout.height = 20;
    mla_array_list_add(uiControls, linkLabel);

    mla_ui_control_t disabledLabel = mla_ui_label();
    mla_ui_label_set_text(disabledLabel, mla_string_const("Disabled Text Example"));
    mla_ui_label_set_text_kind(disabledLabel, MLA_UI_TEXT_KIND_DISABLED);
    disabledLabel.layout.x = 20;
    disabledLabel.layout.y = 230;
    disabledLabel.layout.width = 400;
    disabledLabel.layout.height = 20;
    mla_array_list_add(uiControls, disabledLabel);

    mla_ui_control_t linkDisabledLabel = mla_ui_label();
    mla_ui_label_set_text(linkDisabledLabel, mla_string_const("Disabled Link Example"));
    mla_ui_label_set_text_kind(linkDisabledLabel, MLA_UI_TEXT_KIND_LINK_DISABLED);
    linkDisabledLabel.layout.x = 20;
    linkDisabledLabel.layout.y = 260;
    linkDisabledLabel.layout.width = 400;
    linkDisabledLabel.layout.height = 20;
    mla_array_list_add(uiControls, linkDisabledLabel);

    mla_ui_control_t warningLabel = mla_ui_label();
    mla_ui_label_set_text(warningLabel, mla_string_const("Warning: Battery low"));
    mla_ui_label_set_text_kind(warningLabel, MLA_UI_TEXT_KIND_WARNING);
    warningLabel.layout.x = 20;
    warningLabel.layout.y = 290;
    warningLabel.layout.width = 400;
    warningLabel.layout.height = 20;
    mla_array_list_add(uiControls, warningLabel);

    mla_ui_control_t infoLabel = mla_ui_label();
    mla_ui_label_set_text(infoLabel, mla_string_const("Info: System normal"));
    mla_ui_label_set_text_kind(infoLabel, MLA_UI_TEXT_KIND_INFO);
    infoLabel.layout.x = 20;
    infoLabel.layout.y = 320;
    infoLabel.layout.width = 400;
    infoLabel.layout.height = 20;
    mla_array_list_add(uiControls, infoLabel);

    mla_ui_control_t customLabel = mla_ui_label();
    mla_ui_label_set_text(customLabel, mla_string_const("Custom: Specific Hex Color"));
    mla_ui_label_set_text_kind(customLabel, MLA_UI_TEXT_KIND_CUSTOM);
    mla_ui_label_set_custom_color(customLabel, mla_string_const("#E040FB"));
    customLabel.layout.x = 20;
    customLabel.layout.y = 350;
    customLabel.layout.width = 400;
    customLabel.layout.height = 20;
    mla_array_list_add(uiControls, customLabel);

    // Create example buttons (Right Columns)

    // --- Primary ---
    mla_ui_control_t primaryBtn = mla_ui_button();
    mla_ui_button_set_text(primaryBtn, mla_string_const("Primary"));
    mla_ui_button_set_style(primaryBtn, MLA_UI_BUTTON_STYLE_PRIMARY);
    mla_ui_button_set_click_event(primaryBtn, __main_app_primary_button_clicked);
    primaryBtn.layout.x = 440;
    primaryBtn.layout.y = 20;
    primaryBtn.layout.width = 120;
    primaryBtn.layout.height = 32;
    mla_array_list_add(uiControls, primaryBtn);

    mla_ui_control_t primaryBtnDis = mla_ui_button();
    primaryBtnDis.id = mla_string_const("primary_disabled_button");
    mla_ui_button_set_text(primaryBtnDis, mla_string_const("Primary Dis"));
    mla_ui_button_set_style(primaryBtnDis, MLA_UI_BUTTON_STYLE_PRIMARY);
    mla_ui_button_set_disable(primaryBtnDis, true);
    primaryBtnDis.layout.x = 580;
    primaryBtnDis.layout.y = 20;
    primaryBtnDis.layout.width = 120;
    primaryBtnDis.layout.height = 32;
    mla_array_list_add(uiControls, primaryBtnDis);

    // --- Secondary ---
    mla_ui_control_t secondaryBtn = mla_ui_button();
    mla_ui_button_set_text(secondaryBtn, mla_string_const("Secondary"));
    mla_ui_button_set_style(secondaryBtn, MLA_UI_BUTTON_STYLE_SECONDARY);
    secondaryBtn.layout.x = 440;
    secondaryBtn.layout.y = 70;
    secondaryBtn.layout.width = 120;
    secondaryBtn.layout.height = 32;

    mla_ui_control_t secondaryBtnDis = mla_ui_button();
    mla_ui_button_set_text(secondaryBtnDis, mla_string_const("Secondary Dis"));
    mla_ui_button_set_style(secondaryBtnDis, MLA_UI_BUTTON_STYLE_SECONDARY);
    mla_ui_button_set_disable(secondaryBtnDis, true);
    secondaryBtnDis.layout.x = 580;
    secondaryBtnDis.layout.y = 70;
    secondaryBtnDis.layout.width = 120;
    secondaryBtnDis.layout.height = 32;
    mla_array_list_add(uiControls, secondaryBtnDis);

    // --- Tertiary ---
    mla_ui_control_t tertiaryBtn = mla_ui_button();
    mla_ui_button_set_text(tertiaryBtn, mla_string_const("Tertiary"));
    mla_ui_button_set_style(tertiaryBtn, MLA_UI_BUTTON_STYLE_TERTIARY);
    tertiaryBtn.layout.x = 440;
    tertiaryBtn.layout.y = 120;
    tertiaryBtn.layout.width = 120;
    tertiaryBtn.layout.height = 32;
    mla_array_list_add(uiControls, tertiaryBtn);

    mla_ui_control_t tertiaryBtnDis = mla_ui_button();
    mla_ui_button_set_text(tertiaryBtnDis, mla_string_const("Tertiary Dis"));
    mla_ui_button_set_style(tertiaryBtnDis, MLA_UI_BUTTON_STYLE_TERTIARY);
    mla_ui_button_set_disable(tertiaryBtnDis, true);
    tertiaryBtnDis.layout.x = 580;
    tertiaryBtnDis.layout.y = 120;
    tertiaryBtnDis.layout.width = 120;
    tertiaryBtnDis.layout.height = 32;
    mla_array_list_add(uiControls, tertiaryBtnDis);

    // --- Link ---
    mla_ui_control_t linkBtn = mla_ui_button();
    mla_ui_button_set_text(linkBtn, mla_string_const("Link Style"));
    mla_ui_button_set_style(linkBtn, MLA_UI_BUTTON_STYLE_LINK);
    linkBtn.layout.x = 440;
    linkBtn.layout.y = 170;
    linkBtn.layout.width = 120;
    linkBtn.layout.height = 32;
    mla_array_list_add(uiControls, linkBtn);

    mla_ui_control_t linkBtnDis = mla_ui_button();
    mla_ui_button_set_text(linkBtnDis, mla_string_const("Link Style Dis"));
    mla_ui_button_set_style(linkBtnDis, MLA_UI_BUTTON_STYLE_LINK);
    mla_ui_button_set_disable(linkBtnDis, true);
    linkBtnDis.layout.x = 580;
    linkBtnDis.layout.y = 170;
    linkBtnDis.layout.width = 120;
    linkBtnDis.layout.height = 32;
    mla_array_list_add(uiControls, linkBtnDis);

    return uiControls;
}

mla_bool_t main_app_window_ui_rendering(mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &root, const mla_ui_surface_size_t& surfaceSize, const mla_ui_surface_input_states_t &input_states) {
    (void)root;
    (void)surfaceSize;
    (void)input_states;
    return true;
}

inline void main_app_window_ui_init() {

    // Create a windowed application UI here
    g_main_app_window_ui_surface = mla_ui_surface_create();

    if (!mla_ui_surface_is_valid(g_main_app_window_ui_surface)) {
        mla_error("Failed to create main application window UI surface!");
        return;
    }

    // Build the UI controls
    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> uiControls = __main_app_build_ui();
    g_main_app_window_ui_surface_connector = mla_ui_control_surface_create(g_main_app_window_ui_surface);

    // Start the ui threads
    mla_ui_control_surface_start(g_main_app_window_ui_surface_connector, uiControls, main_app_window_ui_rendering);



}


#endif

