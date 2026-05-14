//
// Created by chris on 3/18/2026.
//

#ifndef MLA_MAIN_APP_BACKGROUND_TASK_H
#define MLA_MAIN_APP_BACKGROUND_TASK_H

#include "../core/task/mla_task_manager.h"

#define mla_log_interval 500

mla_user_data_id_init(mla_last_log_time_user_data_id)

mla_task_process_result_state __main_app_background_log_task(mla_user_data_t& userData) {

    mla_uint64_t current_time = mla_system_time_ms();

    mla_uint64_t last_time = mla_user_data_get_uint64(userData, mla_last_log_time_user_data_id, 0);

    if (current_time - last_time >= mla_log_interval) {
        mla_info(mla_string_concat(mla_string_const("Background log at time: "), mla_string_from_uint64(current_time)));
        mla_user_data_set_uint64(userData, mla_last_log_time_user_data_id, current_time);
    }

    return TASK_PROCESS_RESULT_CONTINUE;

}


inline void main_app_background_tasks_init() {

    mla_uint64_t current_time = mla_system_time_ms();

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_uint64(user_data, mla_last_log_time_user_data_id, current_time);

    mla_task_t log_task = mla_task_repeating(mla_string_const("background_log_task"), __main_app_background_log_task, user_data);
    mla_task_manager_register_task(log_task);






}

#endif