//
// Created by christian on 9/10/25.
//

#include "mla_logger_console.h"

#if !defined(mla_logger_console_level)
#define mla_logger_console_level MLA_LOG_LEVEL_INFO
#endif

void __mla_log_console_writer(const mla_log_level level, mla_string_t &message, mla_string_t &context1,
                              const mla_callback_userdata userData) {

    (void)userData;

    // Simple console output for the log message
    const mla_c_string_t c_message = mla_string_to_cString(message, false);
    const mla_c_string_t c_context1 = mla_string_to_cString(context1, false);

    mla_printf("[%s] %s - %s\n", mla_log_level_to_string(level), c_context1.c_str, c_message.c_str);

    if (c_message.isOwner) {
        mla_free(const_cast<mla_char_t*>(c_message.c_str));
    }

    if (c_context1.isOwner) {
        mla_free(const_cast<mla_char_t*>(c_context1.c_str));
    }

}

mla_bool_t mla_log_to_console_activate() {

    // Check if the low-level access printf function is available
    if (g_low_level_access.printf == nullptr)
        return false;

    const mla_logger_t logger = {
        CONSOLE_LOGGER_NAME,
        mla_logger_console_level, // Default log level
        false,
        __mla_log_console_writer, // Function to write log messages
        0 // No user data for console logger
    };

    return mla_log_register_logger(logger);
}

mla_bool_t mla_log_to_console_deactivate() {
    return mla_log_unregister_logger(CONSOLE_LOGGER_NAME);
}

mla_bool_t mla_log_to_console_is_active() {
    return mla_log_indexOf_logger(CONSOLE_LOGGER_NAME) >= 0;
}

mla_bool_t mla_log_to_console_set_level(mla_log_level level) {
    return mla_log_set_logger_level(CONSOLE_LOGGER_NAME, level);
}

mla_log_level mla_log_to_console_get_level() {
    return mla_log_get_logger_level(CONSOLE_LOGGER_NAME);
}
