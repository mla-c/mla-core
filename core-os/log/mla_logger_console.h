//
// Created by christian on 8/8/25.
//

#ifndef MLA_C_MLA_LOGGER_CONSOLE_H
#define MLA_C_MLA_LOGGER_CONSOLE_H

#include "mla_logger.h"

static mla_string_t CONSOLE_LOGGER_NAME = mla_string_const("Console");

mla_bool_t mla_log_to_console_activate();

mla_bool_t mla_log_to_console_deactivate();

mla_bool_t mla_log_to_console_is_active();

mla_bool_t mla_log_to_console_set_level(mla_log_level level);

mla_log_level mla_log_to_console_get_level();

#endif
