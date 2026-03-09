//
// Created by chris on 3/9/2026.
//

#include "mla_task_manager_data_types.h"

mla_string_t mla_task_priority_to_string(mla_task_priority priority) {
    switch (priority) {
        case TASK_PRIO_LOW: return mla_string_const("Low");
        case TASK_PRIO_NORMAL: return mla_string_const("Normal");
        case TASK_PRIO_HIGH: return mla_string_const("High");
        default: return mla_string_const("Unknown");
    }
}

mla_string_t mla_task_stack_size_to_string(mla_task_stack_size stack_size) {
    switch (stack_size) {
        case TASK_STACK_SIZE_DEFAULT: return mla_string_const("Default");
        case TASK_STACK_SIZE_TINY: return mla_string_const("Tiny");
        case TASK_STACK_SIZE_SMALL: return mla_string_const("Small");
        case TASK_STACK_SIZE_MEDIUM: return mla_string_const("Medium");
        case TASK_STACK_SIZE_LARGE: return mla_string_const("Large");
        case TASK_STACK_SIZE_XLARGE: return mla_string_const("X-Large");
        case TASK_STACK_SIZE_XXLARGE: return mla_string_const("XX-Large");
        default: return mla_string_const("Unknown");
    }
}

mla_string_t mla_task_state_to_string(mla_task_state state) {
    switch (state) {
        case TASK_STATE_STARTING: return mla_string_const("Starting");
        case TASK_STATE_RUNNING: return mla_string_const("Running");
        case TASK_STATE_COMPLETED: return mla_string_const("Completed");
        case TASK_STATE_ABORTING: return mla_string_const("Aborting");
        case TASK_STATE_ABORTED: return mla_string_const("Aborted");
        default: return mla_string_const("Unknown");
    }
}
