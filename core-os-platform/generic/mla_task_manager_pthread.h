//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_TASK_MANAGER_PTHREAD_H
#define COREOS_MLA_TASK_MANAGER_PTHREAD_H

#include "../../core-os/task/mla_task_manager.h"
#include <pthread.h>

struct mla_task_manager_pthread_data_t {
    pthread_t thread;
    mla_callback_userdata userData;
    mla_callback_userdata userData2;
    mla_task_worker_t worker;
    mla_task_shared_states* sharedStates;
};

mla_buffer_cleanup_mode __mla_task_manager_pthread_cleanup(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData; // Unused parameter

    mla_task_manager_pthread_data_t* thread_data = static_cast<mla_task_manager_pthread_data_t*>(data);

    if (thread_data) {
        // Stop the thread if it is still running
        // Note: pthread_cancel is not a clean way to stop a thread. This is a forced stop.
        pthread_cancel(thread_data->thread);

        // Dont set the sharedState this is already gone
        //thread_data->sharedStates->processingState = TASK_STATE_ABORTED;
    }

    return CLEAN_UP_NEEDED;
}

mla_pointer_t __mla_task_manager_pthread_worker(mla_pointer_t payload) {

    mla_task_manager_pthread_data_t* thread_data = static_cast<mla_task_manager_pthread_data_t*>(payload);

    if (thread_data) {

        mla_task_shared_states* shared_states = thread_data->sharedStates;

        if (thread_data->worker == nullptr) {
            shared_states->processingState = TASK_STATE_COMPLETED;
            return nullptr;
        }

        while (true) {

            if (shared_states->processingState == TASK_STATE_ABORTING) {
                shared_states->processingState = TASK_STATE_ABORTED;
                break; // Exit the loop if the task is aborted
            }

            shared_states->processingState = TASK_STATE_RUNNING; // Set the state to running
            mla_task_process_result_state result_state = thread_data->worker(thread_data->userData, thread_data->userData2); // Call the worker function

            if (result_state != TASK_PROCESS_RESULT_CONTINUE) {
                shared_states->processingState = TASK_STATE_COMPLETED; // Set the state to completed if the worker function returns DONE
                break; // Exit the loop after completion
            }

        }

    }

    return nullptr;
}

mla_size_t mla_task_manager_pthread_get_stack_size(mla_task_stack_size stackSize) {

    switch (stackSize) {
        case TASK_STACK_SIZE_TINY:
            return 1024 * 254; // 254 KB
        case TASK_STACK_SIZE_SMALL:
            return 1024 * 512; // 512 KB
        case TASK_STACK_SIZE_MEDIUM:
            return 1024 * 1024; // 1 MB
        case TASK_STACK_SIZE_LARGE:
            return 1024 * 1024 * 2; // 2 MB
        case TASK_STACK_SIZE_XLARGE:
            return 1024 * 1024 * 3; // 3 MB
        case TASK_STACK_SIZE_XXLARGE:
            return 1024 * 1024 * 4; // 4 MB
        default:
            return PTHREAD_STACK_MIN; // Default stack size
    }
}

mla_bool_t mla_task_manager_pthread_create_task(
    const mla_task_worker_t worker,
    mla_callback_userdata userData,
    mla_callback_userdata userData2,
    const mla_task_stack_size stackSize,
    const mla_task_priority priority,
    mla_buffer_reference_t* outTaskResourceOwner,
    mla_task_shared_states* shared_states) {

    mla_task_manager_pthread_data_t* thread_data = static_cast<mla_task_manager_pthread_data_t*>(mla_malloc(sizeof(mla_task_manager_pthread_data_t)));

    if (thread_data == nullptr) {
        return false;
    }

    mla_memset(thread_data, 0, sizeof(mla_task_manager_pthread_data_t));
    thread_data->worker = worker;
    thread_data->userData = userData;
    thread_data->userData2 = userData2;
    thread_data->sharedStates = shared_states;

    // Create thread attributes if needed for stack size
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    if (stackSize != TASK_STACK_SIZE_DEFAULT) {
        pthread_attr_setstacksize(&attr, mla_task_manager_pthread_get_stack_size(stackSize));
    }


    // Set thread scheduling policy to SCHED_RR (round-robin) for priority
    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    // Set thread priority
    struct sched_param param;
    int min_prio = sched_get_priority_min(SCHED_RR);
    int max_prio = sched_get_priority_max(SCHED_RR);
    int prio_range = max_prio - min_prio;

    // Map our priority values to the system's priority range
    switch (priority) {
        case TASK_PRIO_LOW:
            param.sched_priority = min_prio;
            break;
        case TASK_PRIO_NORMAL:
            param.sched_priority = min_prio + (prio_range / 2);
            break;
        case TASK_PRIO_HIGH:
            param.sched_priority = max_prio;
            break;
        default:
            param.sched_priority = min_prio + (prio_range / 2); // Default to normal
    }

    pthread_attr_setschedparam(&attr, &param);

    // Set inheritance - explicitly state that thread takes scheduling attributes from attr
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // TODO The attributes object, should be passed here but its not working
    // But i think on current machines the Stacksize and priority is not that important


    // Create the thread
    int result = pthread_create(
        &thread_data->thread,                       // Pointer to pthread_t structure
        nullptr,
        __mla_task_manager_pthread_worker, // Thread function
        thread_data
    );

    // Clean up attributes
    pthread_attr_destroy(&attr);

    // No scressfull
    if (result != 0) {
        mla_free(thread_data);
        return false;
    }

    // Return the thread handle through outTaskResourceOwner
    *outTaskResourceOwner = mla_buffer_reference(thread_data, true, __mla_task_manager_pthread_cleanup);

    return true;

}

mla_bool_t mla_task_manager_pthread_create_mutex(mla_pointer_t* outMutex) {

    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(mla_malloc(sizeof(pthread_mutex_t)));
    if (mutex == nullptr) {
        return false;
    }

    // Initialize the mutex
    int result = pthread_mutex_init(mutex, nullptr);
    if (result != 0) {
        mla_free(mutex);
        return false;
    }

    *outMutex = static_cast<mla_pointer_t>(mutex);
    return true;

}

mla_bool_t mla_task_manager_pthread_lock_mutex(mla_pointer_t mutexResource, mla_int32_t timeoutms) {

    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false;
    }

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);


    // Add full seconds
    timeout.tv_sec += timeoutms / 1000;

    // Add remaining milliseconds converted to nanoseconds
    timeout.tv_nsec += (timeoutms % 1000) * 1000000;

    // Handle nanosecond overflow
    if (timeout.tv_nsec >= 1000000000) {
        timeout.tv_nsec -= 1000000000;
        timeout.tv_sec += 1;
    }


    return pthread_mutex_timedlock(mutex, &timeout) == 0; // Lock the mutex with timeout
}

mla_bool_t mla_task_manager_pthread_unlock_mutex(mla_pointer_t mutexResource) {

    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false;
    }

    return pthread_mutex_unlock(mutex) == 0;
}

mla_bool_t mla_task_manager_pthread_destroy_mutex(mla_pointer_t mutexResource) {

    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(mutexResource);

    if (mutex == nullptr) {
        return false;
    }

    return pthread_mutex_destroy(mutex) == 0;
}

void mla_task_manager_pthread_run() {

    while (true) {

        mla_task_manager_state state = mla_task_manager_get_state();

        if (state == TASK_MANAGER_STATE_WAITING_FOR_WORK) {
            return;
        }

        mla_sleep(250); // Sleep for 250 milliseconds
    }

}

mla_multi_task_mode mla_task_manager_pthread_multi_task_mode() {
    return MULTI_TASK_MODE_NATIVE;
}

mla_task_manager_low_level_access g_task_low_level_access = {
    mla_task_manager_pthread_create_task,
    mla_task_manager_pthread_run,
    mla_task_manager_pthread_create_mutex,
    mla_task_manager_pthread_lock_mutex,
    mla_task_manager_pthread_unlock_mutex,
    mla_task_manager_pthread_destroy_mutex,
    mla_task_manager_pthread_multi_task_mode
};

#endif
