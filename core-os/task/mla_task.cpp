//
// Created by christian on 9/10/25.
//

#include "mla_task.h"

mla_task_process_result_state __mla_task_worker_repeating(mla_callback_userdata userData, mla_callback_userdata userData2) {
    mla_task_worker_repeating_t worker = reinterpret_cast<mla_task_worker_repeating_t>(userData);
    if (worker != nullptr) {
        return worker(userData2);
    }
    return TASK_PROCESS_RESULT_DONE;
}

mla_task_t mla_task_repeating(const mla_string_t& name, mla_task_worker_repeating_t worker, mla_callback_userdata userData) {
    return {
        name,
        __mla_task_worker_repeating,
        reinterpret_cast<mla_callback_userdata>(worker),
        userData, // Not used for repeating tasks
        TASK_PRIO_NORMAL,
        TASK_STACK_SIZE_DEFAULT,
        mla_buffer_reference_noOwner(),
        nullptr, // No shared states by default
        mla_buffer_reference_noOwner(),
    };
}

mla_task_process_result_state __mla_task_worker_one_time(mla_callback_userdata userData, mla_callback_userdata userData2) {
    mla_task_worker_one_time_t worker = reinterpret_cast<mla_task_worker_one_time_t>(userData);
    if (worker != nullptr) {
        worker(userData2);
    }
    return TASK_PROCESS_RESULT_DONE;
}

mla_task_t mla_task_one_time(const mla_string_t& name, mla_task_worker_one_time_t worker, mla_callback_userdata userData) {
    return {
        name,
        __mla_task_worker_one_time,
        reinterpret_cast<mla_callback_userdata>(worker),
        userData, // Store the one-time worker as userdata
        TASK_PRIO_NORMAL,
        TASK_STACK_SIZE_DEFAULT,
        mla_buffer_reference_noOwner(),
        nullptr, // No shared states by default
        mla_buffer_reference_noOwner(),
    };
}

mla_task_t mla_task_native(const mla_string_t& name, mla_task_worker_t worker, mla_callback_userdata userData, mla_callback_userdata userData2) {
    return {
        name,
        worker,
        userData,
        userData2,
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
