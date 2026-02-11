//
// Created by christian on 9/10/25.
//

#include "mla_task.h"

#define mla_user_data_name_task_rep_worker "tskRep"
#define mla_user_data_name_task_one_worker "tskOne"

mla_task_process_result_state __mla_task_worker_repeating(mla_user_data_t& userData) {

    mla_task_worker_repeating_t worker = mla_user_data_get_callback<mla_task_worker_repeating_t>(userData, mla_user_data_name_task_rep_worker);

    if (worker != nullptr) {
        return worker(userData);
    }
    return TASK_PROCESS_RESULT_DONE;
}

mla_task_t mla_task_repeating(const mla_string_t& name, mla_task_worker_repeating_t worker, mla_user_data_t& userData) {

    mla_user_data_set_callback(userData, mla_user_data_name_task_rep_worker, worker);

    return {
        name,
        __mla_task_worker_repeating,
        userData,
        TASK_PRIO_NORMAL,
        TASK_STACK_SIZE_DEFAULT,
        mla_buffer_reference_noOwner(),
        nullptr, // No shared states by default
        mla_buffer_reference_noOwner(),
    };
}

mla_task_process_result_state __mla_task_worker_one_time(mla_user_data_t& userData) {

    mla_task_worker_one_time_t worker = mla_user_data_get_callback<mla_task_worker_one_time_t>(userData, mla_user_data_name_task_one_worker);

    if (worker != nullptr) {
        worker(userData);
    }
    return TASK_PROCESS_RESULT_DONE;
}

mla_task_t mla_task_one_time(const mla_string_t& name, mla_task_worker_one_time_t worker, mla_user_data_t& userData) {

    mla_user_data_set_callback(userData, mla_user_data_name_task_one_worker, worker);

    return {
        name,
        __mla_task_worker_one_time,
        userData,
        TASK_PRIO_NORMAL,
        TASK_STACK_SIZE_DEFAULT,
        mla_buffer_reference_noOwner(),
        nullptr, // No shared states by default
        mla_buffer_reference_noOwner(),
    };
}

mla_task_t mla_task_native(const mla_string_t& name, mla_task_worker_t worker, mla_user_data_t& userData) {
    return {
        name,
        worker,
        userData,
        TASK_PRIO_NORMAL,
        TASK_STACK_SIZE_DEFAULT,
        mla_buffer_reference_noOwner(),
        nullptr, // No shared states by default
        mla_buffer_reference_noOwner(),
    };
}

// Utils
const mla_char_t* mla_task_priority_to_string(mla_task_priority priority) {

    switch (priority) {
        case TASK_PRIO_LOW:
            return "LOW";
        case TASK_PRIO_NORMAL:
            return "NORMAL";
        case TASK_PRIO_HIGH:
            return "HIGH";
        default:
            return "UNKNOWN";
    }

}

const mla_char_t* mla_task_state_to_string(mla_task_state state) {

    switch (state) {
        case TASK_STATE_STARTING:
            return "STARTING";
        case TASK_STATE_RUNNING:
            return "RUNNING";
        case TASK_STATE_COMPLETED:
            return "COMPLETED";
        case TASK_STATE_ABORTING:
            return "ABORTING";
        case TASK_STATE_ABORTED:
            return "ABORTED";
        case TASK_STATE_UNKNOWN:
            return "UNKNOWN";
        default:
            return "INVALID";
    }

}

const mla_char_t* mla_task_stack_size_to_string(mla_task_stack_size stack_size) {

    switch (stack_size) {
        case TASK_STACK_SIZE_TINY:
            return "TINY";
        case TASK_STACK_SIZE_SMALL:
            return "SMALL";
        case TASK_STACK_SIZE_MEDIUM:
            return "MEDIUM";
        case TASK_STACK_SIZE_LARGE:
            return "LARGE";
        case TASK_STACK_SIZE_XLARGE:
            return "XLARGE";
        case TASK_STACK_SIZE_XXLARGE:
            return "XXLARGE";
        case TASK_STACK_SIZE_DEFAULT:
            return "DEFAULT";
        default:
            return "UNKNOWN";
    }
}
