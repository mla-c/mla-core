//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_TASK_MANAGER_PTHREAD_H
#define COREOS_MLA_TASK_MANAGER_PTHREAD_H

#include "../../core-os/task/mla_task_manager.h"
#include "../../core-os/task/mla_atomic.h"
#include <pthread.h>

#define mla_task_manager_pthread_thread_name_max_length 16

struct mla_task_manager_pthread_data_t {
    pthread_t thread;
    mla_user_data_t userData;
    mla_task_worker_t worker;
    mla_task_shared_states* sharedStates;
};

mla_buffer_cleanup_mode __mla_task_manager_pthread_cleanup(mla_pointer_t data, const mla_dynamic_data_t& userData) {

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
            mla_atomic_exchange(shared_states->processingState, TASK_STATE_COMPLETED);
            return nullptr;
        }

        while (true) {

            if (mla_atomic_compare_exchange(shared_states->processingState, TASK_STATE_ABORTING, TASK_STATE_ABORTED)) {
                break; // Exit the loop if the task is aborted
            }

            // Only switch to RUNNING if it was previously RUNNING or STARTING.
            // If it is ABORTING, we must not overwrite it.
            mla_atomic_compare_exchange(shared_states->processingState, TASK_STATE_STARTING, TASK_STATE_RUNNING);

            mla_task_process_result_state result_state = thread_data->worker(thread_data->userData); // Call the worker function

            if (result_state != TASK_PROCESS_RESULT_CONTINUE) {
                mla_atomic_exchange(shared_states->processingState, TASK_STATE_COMPLETED); // Set the state to completed if the worker function returns DONE
                break; // Exit the loop after completion
            }

            mla_sleep(10);

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
    const mla_string_t& task_name,
    mla_user_data_t& user_data,
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
    thread_data->userData = user_data;
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

    mla_char_t thread_name[mla_task_manager_pthread_thread_name_max_length] = {0};
    mla_memcpy(thread_name, mla_string_data(task_name), mla_min(mla_string_length(task_name), mla_task_manager_pthread_thread_name_max_length - 1));
    pthread_setname_np(thread_data->thread, thread_name);

    // Return the thread handle through outTaskResourceOwner
    *outTaskResourceOwner = mla_buffer_reference_create(thread_data, true, __mla_task_manager_pthread_cleanup, mla_dynamic_data_empty());

    return true;

}

mla_bool_t mla_task_manager_pthread_create_mutex(mla_pointer_t* outMutex, mla_bool_t supports_recursive_locking) {

    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(mla_malloc(sizeof(pthread_mutex_t)));
    if (mutex == nullptr) {
        return false;
    }
    mla_memset(mutex, 0, sizeof(pthread_mutex_t));

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    if (supports_recursive_locking) {
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    }

    // Initialize the mutex
    int result = pthread_mutex_init(mutex, &attr);

    pthread_mutexattr_destroy(&attr);

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

    // 1. FAST PATH: Attempt to lock immediately without expensive time calculations
    if (pthread_mutex_trylock(mutex) == 0) {
        return true;
    }

    // This avoids overhead of clock_gettime and potential math errors for infinite waits.
    if (timeoutms < 0) {
        return pthread_mutex_lock(mutex) == 0;
    }

    // 2. SLOW PATH: Lock is busy, calculate timeout and wait
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

 #define mla_atomic_memory_order __ATOMIC_SEQ_CST

mla_multi_task_mode mla_task_manager_pthread_multi_task_mode() {
    return MULTI_TASK_MODE_NATIVE;
}

mla_int32_t mla_task_manager_pthread_atomic_int32_increment(mla_atomic_int32_t& value) {
    return __atomic_add_fetch(&value.value, 1, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_pthread_atomic_int32_decrement(mla_atomic_int32_t& value) {
    return __atomic_sub_fetch(&value.value, 1, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_pthread_atomic_int32_add(mla_atomic_int32_t& value, mla_int32_t addend) {
    return __atomic_add_fetch(&value.value, addend, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_pthread_atomic_int32_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend) {
    return __atomic_sub_fetch(&value.value, subtrahend, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_pthread_atomic_int32_exchange(mla_atomic_int32_t& value, mla_int32_t newValue) {
    return __atomic_exchange_n(&value.value, newValue, mla_atomic_memory_order);
}

mla_bool_t mla_task_manager_pthread_atomic_int32_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue) {
    return __atomic_compare_exchange_n(&value.value, &expectedValue, newValue, false, mla_atomic_memory_order, __ATOMIC_SEQ_CST);
}

mla_bool_t mla_task_manager_pthread_create_task_local(mla_pointer_t* outTaskLocal) {

    pthread_key_t* key = static_cast<pthread_key_t*>(mla_malloc(sizeof(pthread_key_t)));
    if (key == nullptr) {
        return false;
    }

    int result = pthread_key_create(key, nullptr);
    if (result != 0) {
        mla_free(key);
        return false;
    }

    *outTaskLocal = static_cast<mla_pointer_t>(key);
    return true;
}

mla_bool_t mla_task_manager_pthread_destroy_task_local(mla_pointer_t taskLocal) {

    pthread_key_t* key = static_cast<pthread_key_t*>(taskLocal);
    if (key == nullptr) {
        return false;
    }

    mla_bool_t success = pthread_key_delete(*key) == 0;
    mla_free(key);
    return success;
}

mla_bool_t mla_task_manager_pthread_set_task_local(mla_pointer_t taskLocal, mla_pointer_t value) {

    pthread_key_t* key = static_cast<pthread_key_t*>(taskLocal);
    if (key == nullptr) {
        return false;
    }

    return pthread_setspecific(*key, value) == 0;
}

mla_pointer_t mla_task_manager_pthread_get_task_local(mla_pointer_t taskLocal) {

    pthread_key_t* key = static_cast<pthread_key_t*>(taskLocal);
    if (key == nullptr) {
        return nullptr;
    }

    return pthread_getspecific(*key);
}

mla_task_manager_low_level_access g_task_low_level_access = {
    mla_task_manager_pthread_create_task,
    mla_task_manager_pthread_run,
    mla_task_manager_pthread_create_mutex,
    mla_task_manager_pthread_lock_mutex,
    mla_task_manager_pthread_unlock_mutex,
    mla_task_manager_pthread_destroy_mutex,
    mla_task_manager_pthread_multi_task_mode,
    mla_task_manager_pthread_create_task_local,
    mla_task_manager_pthread_destroy_task_local,
    mla_task_manager_pthread_set_task_local,
    mla_task_manager_pthread_get_task_local,
    mla_task_manager_pthread_atomic_int32_increment,
    mla_task_manager_pthread_atomic_int32_decrement,
    mla_task_manager_pthread_atomic_int32_add,
    mla_task_manager_pthread_atomic_int32_subtract,
    mla_task_manager_pthread_atomic_int32_exchange,
    mla_task_manager_pthread_atomic_int32_compare_exchange

};

#endif
