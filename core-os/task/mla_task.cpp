//
// Created by christian on 9/10/25.
//

#include "mla_task.h"

mla_user_data_id_init(mla_user_data_name_task_rep_worker)
mla_user_data_id_init(mla_user_data_name_task_one_worker)

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
