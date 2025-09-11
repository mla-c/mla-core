//
// Created by christian on 8/8/25.
//

#ifndef COREOS_MLA_LOGGING_H
#define COREOS_MLA_LOGGING_H


#include "../mla_data_types.h"

#ifndef USE_NAVTIVE_LOGGING_MANAGER


// Macro to get the filename and the method
#define __FILENAME_AND_METHOD__()\
char temp[256]; \
mla_snprintf(temp, sizeof(temp), "%s::%s", __FILENAME_ONLY__, __func__); \

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

#define mla_log_msg(level, msg) mla_printf("%s[%s] - %s\n", level, msg, __FILENAME_AND_METHOD__)
#define mla_verbose(msg) mla_printf("v[%s] - %s\n", __FILENAME_AND_METHOD__, msg)
#define mla_debug(msg) mla_printf("d[%s] - %s\n", __FILENAME_AND_METHOD__, msg)
#define mla_info(msg) mla_printf("i[%s] - %s\n", __FILENAME_AND_METHOD__, msg)
#define mla_warning(msg) mla_printf("w[%s] - %s\n", __FILENAME_AND_METHOD__, msg)
#define mla_error(msg) mla_printf("e[%s] - %s\n", __FILENAME_AND_METHOD__, msg)

#endif

#endif