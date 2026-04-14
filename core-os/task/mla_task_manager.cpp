//
// Created by christian on 9/10/25.
//

#include "mla_task_manager.h"
#include "mla_atomic.h"

// Global Task Manager
mla_task_manager_t g_TaskManager = {
    mla_array_list_empty<mla_task_t, mla_task_initializer_t>(),
    mla_rw_lock_create("TaskManager")
};

mla_int32_t __mla_task_manager_find_task_by_name_no_lock(mla_string_t name) {


    for (mla_size_t i = 0; i < mla_array_list_size(g_TaskManager.tasks); ++i) {
        const mla_task_t task = mla_array_list_get_unsafe(g_TaskManager.tasks, i);
        if (mla_string_equals(task.name, name)) {
            return i; // Return the index of the task
        }
    }
    return -1;
}

void __mla_task_manager_cleanup_tasks_no_lock() {

    // Cleanup tasks which are completed
    for (mla_int32_t i = mla_array_list_size(g_TaskManager.tasks) - 1; i >= 0; --i) {

        mla_task_t task = mla_array_list_get_unsafe(g_TaskManager.tasks, i);
        if (mla_task_is_done(task.sharedStates->processingState.value)) {
            mla_array_list_remove(g_TaskManager.tasks, i);
        }

    }

}

void mla_task_manager_cleanup() {

    if (!mla_rw_lock_write(g_TaskManager.taskLock))
        return;

    __mla_task_manager_cleanup_tasks_no_lock();

    mla_rw_unlock_write(g_TaskManager.taskLock);

}

mla_bool_t mla_task_manager_register_task(mla_task_t task) {

    if (!mla_rw_lock_write(g_TaskManager.taskLock))
        return false;

    if (__mla_task_manager_find_task_by_name_no_lock(task.name) >= 0) {
        mla_rw_unlock_write(g_TaskManager.taskLock);
        mla_error(mla_string_concat("Task with name ", task.name , " already exists."));
        return false; // Task with the same name already exists
    }

    // Cleanup tasks which are completed or aborted before adding a new task
    __mla_task_manager_cleanup_tasks_no_lock();

    mla_task_shared_states* sharedStates = static_cast<mla_task_shared_states*>(mla_platform_malloc(sizeof(mla_task_shared_states)));

    if (sharedStates == nullptr) {
        mla_rw_unlock_write(g_TaskManager.taskLock);
        return false; // Memory allocation failed
    }

    mla_memset(sharedStates, 0, sizeof(mla_task_shared_states)); // Initialize shared states to zero
    // Add the starting task to the task manager
    sharedStates->processingState.value = TASK_STATE_STARTING; // Set the initial state of the task
    task.sharedStates = sharedStates; // Assign the shared states to the task
    task.sharedStatesResource = mla_buffer_reference(sharedStates);

    mla_array_list_add(g_TaskManager.tasks, task);

    mla_rw_unlock_write(g_TaskManager.taskLock);

    // Create the Task Resource
    /////////////////////////////

    mla_buffer_reference_t outTaskResourceOwner = mla_buffer_reference_noOwner();
    mla_bool_t success = g_task_low_level_access.create_task(task.worker, task.name, task.userData, task.stack_size, task.priority, &outTaskResourceOwner, sharedStates);


    // Update the task resource in the task manager if the task was created successfully or remove the task if it failed to create
    /////////////////////////////

    if (!mla_rw_lock_write(g_TaskManager.taskLock))
        return false;

    // Remove the task from the task manager if it could not be created
    mla_int32_t taskIndex = __mla_task_manager_find_task_by_name_no_lock(task.name);

    if (taskIndex < 0) {
        mla_rw_unlock_write(g_TaskManager.taskLock);
        mla_error(mla_string_concat("Task with name ", task.name, " not found in task manager after start."));
        return false; // Task not found in task manager
    }

    if (success) {
        mla_array_list_get_ref(g_TaskManager.tasks, taskIndex)->taskResource = outTaskResourceOwner; // Update the task resource in the task manager
    } else {
        mla_array_list_remove(g_TaskManager.tasks, taskIndex);
    }

    mla_rw_unlock_write(g_TaskManager.taskLock);

    if (success) {
        mla_debug(mla_string_concat("Task ", task.name, " registered. StackSize:" , mla_task_stack_size_to_string(task.stack_size), " Priority:", mla_task_priority_to_string(task.priority)));
        return true;
    } else {
        mla_error(mla_string_concat("Task ", task.name, " could not be created."));
        return false;
    }

}

mla_bool_t mla_task_manager_abort_task(const mla_string_t& name) {

    if (!mla_rw_lock_write(g_TaskManager.taskLock)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(g_TaskManager.tasks); ++i) {

        mla_task_t* task = mla_array_list_get_ref(g_TaskManager.tasks, i);

        if (!mla_string_equals(task->name, name)) {
            continue;
        }

        if (!mla_task_is_done(task->sharedStates->processingState.value)) {

            mla_atomic_exchange(task->sharedStates->processingState, TASK_STATE_ABORTING); // Set the task state to aborting

            for (mla_uint16_t count = 0; count < 1000; ++count) {

                mla_sleep(1);
                // Check if the task is done or the state is commited correct
                // There are some race conditions possible here
                if (mla_task_is_done(task->sharedStates->processingState.value)) {
                    break; // Task has been aborted
                }
            }


            mla_debug(mla_string_concat("Task ", task->name, " is being aborted."));
        } else {
            mla_debug(mla_string_concat("Task ", task->name, " is already completed or aborted."));
        }

        break;

    }

    mla_rw_unlock_write(g_TaskManager.taskLock);

    return true;
}

mla_array_list_t<mla_task_info_t, mla_task_info_initializer> mla_task_manager_get_task_infos() {

    mla_array_list_t<mla_task_info_t, mla_task_info_initializer> result = mla_array_list_empty<mla_task_info_t, mla_task_info_initializer>();

    if (!mla_rw_lock_read(g_TaskManager.taskLock)) {
        return result;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(g_TaskManager.tasks); ++i) {

        mla_task_t task = mla_array_list_get_unsafe(g_TaskManager.tasks, i);

        mla_task_info_t info = {
            task.name,
            task.priority,
            task.stack_size,
            (mla_task_state)task.sharedStates->processingState.value
        };

        mla_array_list_add(result, info);

    }

    mla_rw_unlock_read(g_TaskManager.taskLock);

    return result;

}

mla_task_info_t mla_task_manager_get_task_info(const mla_string_t& name) {

    mla_task_info_t result = {
        name,
        TASK_PRIO_LOW, // Default priority
        TASK_STACK_SIZE_DEFAULT, // Default stack size of 1MB
        TASK_STATE_UNKNOWN
    };

    if (!mla_rw_lock_read(g_TaskManager.taskLock)) {
        return result;
    }

    mla_bool_t found = false;

    for (mla_size_t i = 0; i < mla_array_list_size(g_TaskManager.tasks); ++i) {

        mla_task_t task = mla_array_list_get_unsafe(g_TaskManager.tasks, i);

        if (mla_string_equals(task.name, name)) {
            result.priority = task.priority;
            result.stack_size = task.stack_size;
            result.state = (mla_task_state)task.sharedStates->processingState.value;
            found = true;
            break;
        }

    }

    mla_rw_unlock_read(g_TaskManager.taskLock);

    if (!found) {
        mla_debug(mla_string_concat("Task ", name, " not found"));
    }

    return result;
}

mla_bool_t mla_task_manager_task_exists(const mla_string_t& name) {

    if (mla_string_length(name) == 0) {
        return false; // Empty name is not valid
    }

    if (!mla_rw_lock_read(g_TaskManager.taskLock)) {
        return false;
    }

    mla_bool_t found = false;

    for (mla_size_t i = 0; i < mla_array_list_size(g_TaskManager.tasks); ++i) {

        mla_task_t task = mla_array_list_get_unsafe(g_TaskManager.tasks, i);

        if (mla_string_equals(task.name, name)) {
            found = true;
            break;
        }

    }

    mla_rw_unlock_read(g_TaskManager.taskLock);

    return found; // Name is valid

}

mla_task_manager_state mla_task_manager_get_state() {

    if (!mla_rw_lock_read(g_TaskManager.taskLock)) {
        return TASK_MANAGER_STATE_UNKNOWN;
    }

    mla_task_manager_state state = TASK_MANAGER_STATE_WAITING_FOR_WORK;

    for (mla_size_t i = 0; i < mla_array_list_size(g_TaskManager.tasks); ++i) {

        mla_task_t task = mla_array_list_get_unsafe(g_TaskManager.tasks, i);

        if (task.sharedStates != nullptr) {

            if (!mla_task_is_done(task.sharedStates->processingState.value)) {
                state = TASK_MANAGER_STATE_PROCESSING;
                break;
            }

        }

    }

    mla_rw_unlock_read(g_TaskManager.taskLock);
    return state;
}


void mla_task_manager_process_all_tasks() {
    g_task_low_level_access.run_tasks();
}
