//
// Created by chris on 1/28/2026.
//

#include "mla_text_edit.h"

mla_bool_t __mla_ui_text_process_char_input_event(mla_ui_control_t &control, const mla_ui_surface_input_event_char_input_t &charInputEvent, mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, mla_callback_userdata userData) {
    (void)control;
    (void)charInputEvent;
    (void)uiControls;
    (void)userData;
    return true;
}


mla_bool_t __mla_ui_button_render_to_drawCommands(const mla_ui_control_context_t &context, const mla_ui_control_t &element, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_array_list_t<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t> &inputAreas) {


}

mla_ui_control_t mla_text_edit() {

    mla_ui_control_t button = mla_ui_control();
    button.renderToDrawCommands = __mla_ui_button_render_to_drawCommands;
    button.processCharInputEvent = __mla_ui_text_process_char_input_event;
    return button;
}

mla_ui_text_edit_style_t mla_text_edit_get_style(const mla_ui_control_t &textEdit) {
    return static_cast<mla_ui_text_edit_style_t>(mla_ui_control_get_value_as_uint8(textEdit, mla_string_const("style"), MLA_UI_TEXT_EDIT_STYLE_STANDARD));
}

mla_bool_t mla_text_edit_set_style(mla_ui_control_t &textEdit, mla_ui_text_edit_style_t style) {
    return mla_ui_control_set_value_as_uint8(textEdit, mla_string_const("style"), static_cast<mla_uint8_t>(style));
}

mla_string_t mla_text_edit_get_text(const mla_ui_control_t &textEdit) {
    return mla_ui_control_get_value_as_string(textEdit, mla_string_const("text"));
}

mla_bool_t mla_text_edit_set_text(mla_ui_control_t &textEdit, const mla_string_t& text) {
    return mla_ui_control_set_value_as_string(textEdit, mla_string_const("text"), text);
}

mla_bool_t mla_ui_button_get_disable(const mla_ui_control_t &button) {
    return mla_ui_control_get_value_as_bool(button, mla_string_const("disabled"), false);
}

mla_bool_t mla_ui_button_set_disable(mla_ui_control_t &button, mla_bool_t disable) {
    return mla_ui_control_set_value_as_bool(button, mla_string_const("disabled"), disable);
}