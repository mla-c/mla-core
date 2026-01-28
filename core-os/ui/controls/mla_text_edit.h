//
// Created by chris on 1/28/2026.
//

#ifndef COREOS_MLA_TEXT_EDIT_H
#define COREOS_MLA_TEXT_EDIT_H

#include "mla_ui_control.h"
#include "mla_ui_style.h"

mla_ui_control_t mla_text_edit();

mla_ui_text_edit_style_t mla_text_edit_get_style(const mla_ui_control_t &textEdit);
mla_bool_t mla_text_edit_set_style(mla_ui_control_t &textEdit, mla_ui_text_edit_style_t style);

mla_string_t mla_text_edit_get_text(const mla_ui_control_t &textEdit);
mla_bool_t mla_text_edit_set_text(mla_ui_control_t &textEdit, const mla_string_t& text);

mla_bool_t mla_ui_button_get_disable(const mla_ui_control_t &button);
mla_bool_t mla_ui_button_set_disable(mla_ui_control_t &button, mla_bool_t disable);

#endif