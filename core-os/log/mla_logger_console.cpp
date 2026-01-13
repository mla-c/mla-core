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

    const mla_char_t* messageData = mla_string_data(message);
    mla_size_t messageLength = mla_string_length(message);

    const mla_char_t* context1Data = mla_string_data(context1);
    mla_size_t context1Length = mla_string_length(context1);

    mla_print("[", 1);
    mla_print(mla_log_level_to_string(level), mla_log_level_to_string_length(level));
    mla_print("] ", 2);

    mla_print(context1Data, context1Length);
    mla_print(" - ", 3);
    mla_print(messageData, messageLength);
    mla_print("\n", 1);

}

mla_bool_t mla_log_to_console_activate() {

    // Check if the low-level access printf function is available
    if (g_low_level_access.print == nullptr)
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
