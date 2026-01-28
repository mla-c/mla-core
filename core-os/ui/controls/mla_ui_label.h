//
// Created by chris on 1/19/2026.
//

#ifndef COREOS_MLA_UI_LABEL_H
#define COREOS_MLA_UI_LABEL_H

#include "mla_ui_control.h"
#include "mla_ui_style.h"

mla_ui_control_t mla_ui_label();

mla_string_t mla_ui_label_get_text(const mla_ui_control_t &label);
mla_bool_t mla_ui_label_set_text(mla_ui_control_t &label, const mla_string_t& text);

mla_uint16_t mla_ui_label_get_font_size(const mla_ui_control_t &label);
mla_bool_t mla_ui_label_set_font_size(mla_ui_control_t &label, mla_uint16_t fontSize);

mla_ui_text_kind_t mla_ui_label_get_text_kind(const mla_ui_control_t &label);
mla_bool_t mla_ui_label_set_text_kind(mla_ui_control_t &label, mla_ui_text_kind_t kind);

mla_string_t mla_ui_label_get_custom_color(const mla_ui_control_t &label);
mla_bool_t mla_ui_label_set_custom_color(mla_ui_control_t &label, const mla_string_t& color);



#endif