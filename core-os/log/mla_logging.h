//
// Created by christian on 8/8/25.
//

#ifndef COREOS_MLA_LOGGING_H
#define COREOS_MLA_LOGGING_H


#include "../mla_data_types.h"

// Macro to get the filename and the method
#define __FILENAME_AND_METHOD__()\
mla_char_t temp[64]; \
mla_snprintf(temp, sizeof(temp), "%s::%s", __FILENAME_ONLY__, __func__); \

#if (!defined(mla_logging_use_native) || (mla_logging_use_native == 0))

#include "mla_logger.h"

#define mla_log_msg(level, msg) \
    if(mla_log_is_active(level)) \
        { \
        __FILENAME_AND_METHOD__()\
        mla_log_message(level, msg, temp); } \
    else \
        {}

#define mla_verbose(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_VERBOSE)) \
        { \
            __FILENAME_AND_METHOD__()\
            mla_log_verbose(msg, temp); } \
    else \
        {}

#define mla_debug(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_DEBUG)) \
        { \
            __FILENAME_AND_METHOD__()\
            mla_log_debug(msg, temp); } \
    else \
        {}

#define mla_info(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_INFO)) \
        { \
            __FILENAME_AND_METHOD__()\
            mla_log_info(msg, temp); } \
    else \
        {}

#define mla_warning(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_WARNING)) \
        { \
            __FILENAME_AND_METHOD__()\
            mla_log_warning(msg, temp); } \
    else \
        {}

#define mla_error(msg) \
    if(mla_log_is_active(MLA_LOG_LEVEL_ERROR)) \
    { \
        __FILENAME_AND_METHOD__()\
        mla_log_error(msg, temp); } \
    else \
    {}


#else

#include "mla_logger.h"

inline void __mla_logging_native(const mla_log_level level, const mla_char_t* message) {
    __FILENAME_AND_METHOD__()\
    mla_printf("[%s] %s - %s\n", mla_log_level_to_string(level), temp, message);\
}

inline void __mla_logging_native(const mla_log_level level, const mla_string_t& message) {
    __FILENAME_AND_METHOD__()\
    mla_string_t copy = message; \
    const mla_c_string_t c_message = mla_string_to_cString(copy, false);
    mla_printf("[%s] %s - %s\n", mla_log_level_to_string(level), temp, c_message);\
    if (c_message.isOwner) {\
        mla_free(const_cast<mla_char_t*>(c_message.c_str));\
    }\
}

#define mla_log_msg(level, msg) __mla_logging_native(level, msg)
#define mla_verbose(msg) __mla_logging_native(MLA_LOG_LEVEL_VERBOSE, msg)
#define mla_debug(msg) __mla_logging_native(MLA_LOG_LEVEL_DEBUG, msg)
#define mla_info(msg) __mla_logging_native(MLA_LOG_LEVEL_INFO, msg)
#define mla_warning(msg) __mla_logging_native(MLA_LOG_LEVEL_WARNING, msg)
#define mla_error(msg) __mla_logging_native(MLA_LOG_LEVEL_ERROR, msg)


#endif

#endif