//
// Created by chris on 1/19/2026.
//

#ifndef COREOS_MLA_UI_WINDOW_H
#define COREOS_MLA_UI_WINDOW_H

#include "mla_ui_control.h"

mla_ui_control_t mla_ui_window();

mla_string_t mla_ui_window_get_title(const mla_ui_control_t &window);
mla_bool_t mla_ui_window_set_title(mla_ui_control_t &window, const mla_string_t &title);

mla_bool_t mla_ui_window_get_resizable(const mla_ui_control_t &window);
mla_bool_t mla_ui_window_set_resizable(mla_ui_control_t &window, mla_bool_t resizable);

typedef void (*mla_ui_window_on_close_callback_t)(mla_ui_control_t &window);
mla_ui_window_on_close_callback_t mla_ui_window_get_close(const mla_ui_control_t &window);
mla_bool_t mla_ui_window_set_close(mla_ui_control_t &window, mla_ui_window_on_close_callback_t callback);

#endif