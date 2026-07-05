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

#if (mla_global_feature_flag_logging_use_native == 0)

#include "mla_logger.h"

#define mla_log_msg(level, msg) \
    if(mla_log_is_active(level)) \
        { \
        mla_filename_and_method()\
        mla_log_message(level, msg, temp); } \
    else \
        {}

#define mla_verbose(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_VERBOSE)) \
        { \
            mla_filename_and_method()\
            mla_log_verbose(msg, temp); } \
    else \
        {}

#define mla_debug(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_DEBUG)) \
        { \
            mla_filename_and_method()\
            mla_log_debug(msg, temp); } \
    else \
        {}

#define mla_info(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_INFO)) \
        { \
            mla_filename_and_method()\
            mla_log_info(msg, temp); } \
    else \
        {}

#define mla_warning(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_WARNING)) \
        { \
            mla_filename_and_method()\
            mla_log_warning(msg, temp); } \
    else \
        {}

#define mla_error(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_ERROR)) \
    { \
        mla_filename_and_method()\
        mla_log_error(msg, temp); } \
    else \
    {}


#else

#include "mla_logger.h"

inline void mla_private_mla_logging_native(const mla_log_level level, const mla_char_t* message) {
    mla_filename_and_method()\
    mla_print("[", 1);
    mla_print(mla_log_level_to_string(level), mla_log_level_to_string_length(level));
    mla_print("] ", 2);

    mla_print(temp, mla_strlen(temp));
    mla_print(" - ", 3);
    mla_print(message, mla_strlen(temp));
    mla_print("\n", 1);
}

inline void mla_private_mla_logging_native(const mla_log_level level, const mla_string_t& message) {
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

#define mla_log_msg(level, msg) mla_private_mla_logging_native(level, msg)
#define mla_verbose(msg) mla_private_mla_logging_native(MLA_LOG_LEVEL_VERBOSE, msg)
#define mla_debug(msg) mla_private_mla_logging_native(MLA_LOG_LEVEL_DEBUG, msg)
#define mla_info(msg) mla_private_mla_logging_native(MLA_LOG_LEVEL_INFO, msg)
#define mla_warning(msg) mla_private_mla_logging_native(MLA_LOG_LEVEL_WARNING, msg)
#define mla_error(msg) mla_private_mla_logging_native(MLA_LOG_LEVEL_ERROR, msg)


#endif

#endif