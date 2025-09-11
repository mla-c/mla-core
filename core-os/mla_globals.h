//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_GLOBALS_H
#define COREOS_GLOBALS_H

#include "mla_data_types.h"
#include "memory/mla_memory_hook.h"
#include "log/mla_logger.h"
#include "system/mla_array_list.h"
#include "task/mla_task_manager.h"

// Global memory hook for custom memory management
mla_global_memory_hook_list_t g_memory_hook = {
    nullptr,
    nullptr,
    0,
    {}
};


// Global Log Manager

mla_logger_manager_t g_logger_manager = {
    mla_array_list_empty<mla_logger_t, mla_logger_initializer>()
};

// Global Task Manager
mla_task_manager_t g_TaskManager = {
    mla_array_list_empty<mla_task_t, mla_task_initializer_t>(),
    mla_rw_lock_create("TaskManager")
};


#endif
