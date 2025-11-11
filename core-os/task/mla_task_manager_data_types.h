//
// Created by christian on 8/9/25.
//

#ifndef COREOS_MLA_TASK_MANAGER_DATA_TYPES_H
#define COREOS_MLA_TASK_MANAGER_DATA_TYPES_H

#include "../mla_data_types.h"
#include "../system/mla_buffer.h"
#include "mla_atomic.h"

enum mla_task_process_result_state: mla_uint8_t {
    TASK_PROCESS_RESULT_DONE,
    TASK_PROCESS_RESULT_CONTINUE,
};

typedef mla_task_process_result_state (*mla_task_worker_t)(mla_callback_userdata userData, mla_callback_userdata userData2);

enum mla_task_priority: mla_uint8_t {
    TASK_PRIO_LOW,
    TASK_PRIO_NORMAL,
    TASK_PRIO_HIGH,
};

enum mla_task_stack_size: mla_uint8_t {
    TASK_STACK_SIZE_DEFAULT,
    TASK_STACK_SIZE_TINY,
    TASK_STACK_SIZE_SMALL,
    TASK_STACK_SIZE_MEDIUM,
    TASK_STACK_SIZE_LARGE,
    TASK_STACK_SIZE_XLARGE,
    TASK_STACK_SIZE_XXLARGE,
};

enum mla_task_state: mla_uint8_t {
    TASK_STATE_STARTING, // Task is pending execution
    TASK_STATE_RUNNING, // Task is currently running
    TASK_STATE_COMPLETED, // Task has completed execution
    TASK_STATE_ABORTING, // Task is in the process of being aborted
    TASK_STATE_ABORTED, // Task has been aborted
    TASK_STATE_UNKNOWN // Task state is unknown or not set
};

enum mla_multi_task_mode: mla_uint8_t {
    MULTI_TASK_MODE_SIMULATED, // Not real Multi Tasking, but simulates it in single thread
    MULTI_TASK_MODE_NATIVE, // Uses the native OS Multi Tasking capabilities
};

// Represents shared states for a task, and the plaform logices
struct mla_task_shared_states {
    mla_task_state processingState;
};

#define mla_task_is_done(state) (state == TASK_STATE_COMPLETED || state == TASK_STATE_ABORTED)

struct mla_task_manager_low_level_access {

    mla_bool_t (*create_task)(const mla_task_worker_t worker, mla_callback_userdata userData, mla_callback_userdata userData2, const mla_task_stack_size stackSize, const mla_task_priority priority, mla_buffer_reference_t* outTaskResourceOwner, mla_task_shared_states* shared_states);
    void (*run_tasks)();
    mla_bool_t (*create_mutex)(mla_pointer_t* outMutex);
    mla_bool_t (*lock_mutex)(mla_pointer_t mutex, mla_int32_t timeout);
    mla_bool_t (*unlock_mutex)(mla_pointer_t mutex);
    mla_bool_t (*destroy_mutex)(mla_pointer_t mutex);
    mla_multi_task_mode (*get_multi_task_mode)();

    // atomic operations
    mla_int32_t (*atomic_int32_increment)(mla_atomic_int32_t& value);
    mla_int32_t (*atomic_int32_decrement)(mla_atomic_int32_t& value);
    mla_int32_t (*atomic_int32_add)(mla_atomic_int32_t& value, mla_int32_t addend);
    mla_int32_t (*atomic_int32_subtract)(mla_atomic_int32_t& value, mla_int32_t subtrahend);
    mla_int32_t (*atomic_int32_exchange)(mla_atomic_int32_t& value, mla_int32_t newValue);
    mla_bool_t (*atomic_int32_compare_exchange)(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue);
};

mla_global mla_task_manager_low_level_access g_task_low_level_access;

#define mla_is_native_multi_tasking g_task_low_level_access.get_multi_task_mode() == MULTI_TASK_MODE_NATIVE

#endif
