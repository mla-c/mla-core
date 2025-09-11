//
// Created by christian on 8/9/25.
//

#ifndef COREOS_MLA_TASK_H
#define COREOS_MLA_TASK_H

#include "../system/mla_string.h"
#include "mla_task_manager_data_types.h"

typedef void (*mla_task_worker_one_time_t)(mla_callback_userdata userData);
typedef mla_task_process_result_state (*mla_task_worker_repeating_t)(mla_callback_userdata userData);


struct mla_task_t {
    mla_string_t name; // Name of the task
    mla_task_worker_t worker;
    mla_callback_userdata workerUserdata; // User data for the task
    mla_callback_userdata workerUserdata2; // User data for the task
    mla_task_priority priority; // Priority of the task
    mla_task_stack_size stack_size; // Stack size for the task

    // OS Resources
    mla_buffer_reference_t taskResource;

    // States
    mla_task_shared_states* sharedStates; // Shared states for the task
    mla_buffer_reference_t sharedStatesResource; // Resource for shared states
};

struct mla_task_initializer_t {
    static mla_task_t init() {
        return {
            mla_string_empty(),
            nullptr,
            0,
            0,
            TASK_PRIO_NORMAL,
            TASK_STACK_SIZE_DEFAULT,
            mla_buffer_reference_noOwner(),
            nullptr, // No shared states by default
            mla_buffer_reference_noOwner(),
        };
    }
};

mla_task_t mla_task_repeating(const mla_string_t& name, mla_task_worker_repeating_t worker, mla_callback_userdata userData = 0);
mla_task_process_result_state __mla_task_worker_one_time(mla_callback_userdata userData, mla_callback_userdata userData2);
mla_task_t mla_task_one_time(const mla_string_t& name, mla_task_worker_one_time_t worker, mla_callback_userdata userData = 0);
mla_task_t mla_task_native(const mla_string_t& name, mla_task_worker_t worker, mla_callback_userdata userData = 0, mla_callback_userdata userData2 = 0);

#endif
