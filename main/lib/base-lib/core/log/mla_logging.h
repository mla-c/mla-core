//
// Created by christian on 8/8/25.
//

#ifndef MLA_LOGGING_H
#define MLA_LOGGING_H


#include "../mla_data_types.h"

void mla_private_logging_concat(mla_char_t* targetBuffer, const mla_char_t* filename, const mla_char_t* function);

// Macro to get the filename and the method
#define mla_filename_and_method()\
mla_char_t temp[64]; \
mla_private_logging_concat(temp, mla_filename_only, __func__); \

#define mla_private_logging_create_stack_trace_buffer() \
    mla_char_t stack_trace_temp[mla_global_config_stack_trace_max_size]; \
    mla_size_t stack_trace_length = g_low_level_access.get_stack_trace( \
        stack_trace_temp, mla_global_config_stack_trace_max_size); \
    if (stack_trace_length >= mla_global_config_stack_trace_max_size) { \
        stack_trace_length = mla_global_config_stack_trace_max_size - 1; \
    } \
    stack_trace_temp[stack_trace_length] = '\0'

#define mla_private_logging_stack_trace_not_supported "Stacktrace not supported"

#if (mla_global_feature_flag_logging_use_native == 0)

#include "mla_logger.h"

#define mla_log_msg(level, msg) \
    do { \
        const mla_log_level mla_private_log_level = (level); \
        if (mla_log_is_active(mla_private_log_level)) { \
            mla_filename_and_method() \
            mla_log_message(mla_private_log_level, (msg), temp); \
        } \
    } while (false)

#define mla_verbose(msg) \
    do { \
        if (mla_log_is_active(MLA_LOG_LEVEL_VERBOSE)) { \
            mla_filename_and_method() \
            mla_log_verbose((msg), temp); \
        } \
    } while (false)

#define mla_debug(msg) \
    do { \
        if (mla_log_is_active(MLA_LOG_LEVEL_DEBUG)) { \
            mla_filename_and_method() \
            mla_log_debug((msg), temp); \
        } \
    } while (false)

#define mla_info(msg) \
    do { \
        if (mla_log_is_active(MLA_LOG_LEVEL_INFO)) { \
            mla_filename_and_method() \
            mla_log_info((msg), temp); \
        } \
    } while (false)

#define mla_warning(msg) \
    do { \
        if (mla_log_is_active(MLA_LOG_LEVEL_WARNING)) { \
            mla_filename_and_method() \
            mla_log_warning((msg), temp); \
        } \
    } while (false)

#define mla_error(msg) \
    do { \
        if (mla_log_is_active(MLA_LOG_LEVEL_ERROR)) { \
            mla_filename_and_method() \
            mla_log_error((msg), temp); \
        } \
    } while (false)

#define mla_log_stack_trace(level, msg) \
    do { \
        const mla_log_level mla_private_stack_trace_level = (level); \
        if (mla_log_is_active(mla_private_stack_trace_level)) { \
            mla_filename_and_method() \
            mla_log_message(mla_private_stack_trace_level, (msg), temp); \
            if (g_low_level_access.get_stack_trace != nullptr) { \
                mla_private_logging_create_stack_trace_buffer(); \
                if (stack_trace_length > 0) { \
                    mla_log_message(mla_private_stack_trace_level, stack_trace_temp, temp); \
                } \
            } else { \
                mla_log_message(mla_private_stack_trace_level, mla_private_logging_stack_trace_not_supported, temp); \
            } \
        } \
    } while (false)

#else

#include "mla_logger.h"

inline void mla_private_logging_native(const mla_log_level level, const mla_char_t* message) {
    mla_filename_and_method()\
    mla_print("[", 1);
    mla_print(mla_log_level_to_string(level), mla_log_level_to_string_length(level));
    mla_print("] ", 2);

    mla_print(temp, mla_strlen(temp));
    mla_print(" - ", 3);
    mla_print(message, mla_strlen(message));
    mla_print("\n", 1);
}

inline void mla_private_logging_native(const mla_log_level level, const mla_string_t& message) {
    mla_filename_and_method()\
    mla_print("[", 1);
    mla_print(mla_log_level_to_string(level), mla_log_level_to_string_length(level));
    mla_print("] ", 2);

    mla_print(temp, mla_strlen(temp));
    mla_print(" - ", 3);

    const mla_char_t* messageData = mla_string_data(message);
    mla_size_t messageLength = mla_string_length(message);

    mla_print(messageData, messageLength);
    mla_print("\n", 1);
}

#define mla_log_msg(level, msg) \
    do { mla_private_logging_native((level), (msg)); } while (false)
#define mla_verbose(msg) \
    do { mla_private_logging_native(MLA_LOG_LEVEL_VERBOSE, (msg)); } while (false)
#define mla_debug(msg) \
    do { mla_private_logging_native(MLA_LOG_LEVEL_DEBUG, (msg)); } while (false)
#define mla_info(msg) \
    do { mla_private_logging_native(MLA_LOG_LEVEL_INFO, (msg)); } while (false)
#define mla_warning(msg) \
    do { mla_private_logging_native(MLA_LOG_LEVEL_WARNING, (msg)); } while (false)
#define mla_error(msg) \
    do { mla_private_logging_native(MLA_LOG_LEVEL_ERROR, (msg)); } while (false)

#define mla_log_stack_trace(level, msg) \
    do { \
        const mla_log_level mla_private_stack_trace_level = (level); \
        mla_private_logging_native(mla_private_stack_trace_level, (msg)); \
        if (g_low_level_access.get_stack_trace != nullptr) { \
            mla_private_logging_create_stack_trace_buffer(); \
            if (stack_trace_length > 0) { \
                mla_private_logging_native(mla_private_stack_trace_level, stack_trace_temp); \
            } \
        } else { \
            mla_private_logging_native(mla_private_stack_trace_level, mla_private_logging_stack_trace_not_supported); \
        } \
    } while (false)

#endif

#define mla_verbose_stack_trace(msg) \
    do { mla_log_stack_trace(MLA_LOG_LEVEL_VERBOSE, (msg)); } while (false)
#define mla_debug_stack_trace(msg) \
    do { mla_log_stack_trace(MLA_LOG_LEVEL_DEBUG, (msg)); } while (false)
#define mla_info_stack_trace(msg) \
    do { mla_log_stack_trace(MLA_LOG_LEVEL_INFO, (msg)); } while (false)
#define mla_warning_stack_trace(msg) \
    do { mla_log_stack_trace(MLA_LOG_LEVEL_WARNING, (msg)); } while (false)
#define mla_error_stack_trace(msg) \
    do { mla_log_stack_trace(MLA_LOG_LEVEL_ERROR, (msg)); } while (false)

#endif
