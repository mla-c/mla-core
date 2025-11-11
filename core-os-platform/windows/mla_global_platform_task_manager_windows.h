//
// Created by chris on 8/10/2025.
//

#ifndef COREOS_MLA_TASK_MANAGER_WINDOWS_NATIVE_H
#define COREOS_MLA_TASK_MANAGER_WINDOWS_NATIVE_H

#include "../../core-os/task/mla_task_manager.h"
#include <windows.h>
#include <assert.h>

struct mla_task_manager_windows_native_data_t {
    HANDLE hThread;
    mla_callback_userdata userData;
    mla_callback_userdata userData2;
    mla_task_worker_t worker;
    mla_task_shared_states* sharedStates;
};

mla_buffer_cleanup_mode __mla_task_manager_windows_native_cleanup(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData;
    mla_task_manager_windows_native_data_t* thread_data = static_cast<mla_task_manager_windows_native_data_t*>(data);

    if (thread_data) {
        // Stop the thread if it is still running
        if (thread_data->hThread != nullptr) {
            TerminateThread(thread_data->hThread, 0);
            CloseHandle(thread_data->hThread);
        }

        // Dont set the sharedState this is already gone
        //thread_data->sharedStates->processingState = TASK_STATE_ABORTED;
    }

    return CLEAN_UP_NEEDED;
}

DWORD WINAPI __mla_task_manager_windows_native_worker(LPVOID lpParam) {

    mla_task_manager_windows_native_data_t* thread_data = static_cast<mla_task_manager_windows_native_data_t*>(lpParam);

    if (thread_data) {

        mla_task_shared_states* shared_states = thread_data->sharedStates;

        if (thread_data->worker == nullptr) {
            shared_states->processingState = TASK_STATE_COMPLETED;
            return 0;
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

    return 0;
}


SIZE_T mla_task_manager_windows_native_get_stack_size(const mla_task_stack_size stackSize) {

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
            return 0;
    }
}

mla_bool_t mla_task_manager_windows_native_create_task(
        const mla_task_worker_t worker,
        mla_callback_userdata userData,
        mla_callback_userdata userData2,
        const mla_task_stack_size stackSize,
        const mla_task_priority priority,
        mla_buffer_reference_t* outTaskResourceOwner,
        mla_task_shared_states* shared_states) {

    mla_task_manager_windows_native_data_t* thread_data = static_cast<mla_task_manager_windows_native_data_t*>(mla_malloc(sizeof(mla_task_manager_windows_native_data_t)));

    if (thread_data == nullptr) {
        return false;
    }

    mla_memset(thread_data, 0, sizeof(mla_task_manager_windows_native_data_t));
    thread_data->worker = worker;
    thread_data->userData = userData;
    thread_data->userData2 = userData2;
    thread_data->sharedStates = shared_states;

    SIZE_T stackSizeInBytes = mla_task_manager_windows_native_get_stack_size(stackSize);
    thread_data->hThread = CreateThread(nullptr, stackSizeInBytes, __mla_task_manager_windows_native_worker, thread_data, 0, nullptr);

    if (thread_data->hThread == nullptr) {
        mla_free(thread_data);
        return false; // Failed to create thread
    }

    // Set thread priority
    int winPriority;
    switch (priority) {
        case TASK_PRIO_LOW:
            winPriority = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case TASK_PRIO_NORMAL:
            winPriority = THREAD_PRIORITY_NORMAL;
            break;
        case TASK_PRIO_HIGH:
            winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        default:
            winPriority = THREAD_PRIORITY_NORMAL; // Default to normal priority
    }
    SetThreadPriority(thread_data->hThread, winPriority);

    // Return the thread handle through outTaskResourceOwner
    *outTaskResourceOwner = mla_buffer_reference(thread_data, true, __mla_task_manager_windows_native_cleanup);
    return true; // Successfully created the task
}

// Windows native mutex implementation is to slow and supports recursive locks
// We will use a CRITICAL_SECTION for the mutex implementation
struct mla_task_manager_windows_native_mutex_t {
    CRITICAL_SECTION section;
    mla_bool_t locked; // already locked by the current thread
};

mla_bool_t mla_task_manager_windows_native_create_mutex(mla_pointer_t* outMutex) {

    mla_task_manager_windows_native_mutex_t* mutex = static_cast<mla_task_manager_windows_native_mutex_t*>(mla_malloc(sizeof(mla_task_manager_windows_native_mutex_t)));

    if (mutex == nullptr) {
        return false; // Failed to allocate memory for mutex
    }
    mla_memset(mutex, 0, sizeof(mla_task_manager_windows_native_mutex_t));

    InitializeCriticalSection(&mutex->section);
    mutex->locked = false; // Initially not locked

    *outMutex = static_cast<mla_pointer_t>(mutex);
    return true; // Successfully created the mutex

}

mla_bool_t mla_task_manager_windows_native_lock_mutex(mla_pointer_t mutexResource, mla_int32_t timeoutms) {

    mla_task_manager_windows_native_mutex_t* mutex = static_cast<mla_task_manager_windows_native_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    // Windows does not support timeouts for critical sections directly,
    // so we will simulate it with a sleep loop.

    for (int i = 0; i < timeoutms / 10; ++i) {

        if (TryEnterCriticalSection(&mutex->section)) {

            if (mutex->locked) {
                LeaveCriticalSection(&mutex->section); // Leave the critical section if it was already locked
                return false; // Mutex is already locked by this thread
            }

            mutex->locked = true; // Mark as locked
            return true; // Successfully locked the mutex
        }
        Sleep(10); // Sleep for 10 milliseconds
    }
    return false; // Failed to lock the mutex within the timeout
}

mla_bool_t mla_task_manager_windows_native_unlock_mutex(mla_pointer_t mutexResource) {

    mla_task_manager_windows_native_mutex_t* mutex = static_cast<mla_task_manager_windows_native_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    if (TryEnterCriticalSection(&mutex->section)) {

        if (!mutex->locked) {
            LeaveCriticalSection(&mutex->section); // Leave the critical section if it was not locked
            return false; // Mutex was not locked by this thread
        }

        // Locked by this thread, so we can unlock it
        mutex->locked = false; // Mark as unlocked
        LeaveCriticalSection(&mutex->section); // Leave the critical section
    } else {
        return false; // Mutex was not locked by this thread
    }

    LeaveCriticalSection(&mutex->section); // Leave the critical section
    return true;

}

mla_bool_t mla_task_manager_windows_native_destroy_mutex(mla_pointer_t mutexResource) {

    mla_task_manager_windows_native_mutex_t* mutex = static_cast<mla_task_manager_windows_native_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    DeleteCriticalSection(&mutex->section); // Delete the critical section
    mla_free(mutex); // Free the mutex memory
    return true;
}

void mla_task_manager_windows_native_run() {

    while (true) {

        mla_task_manager_state state = mla_task_manager_get_state();

        if (state == TASK_MANAGER_STATE_WAITING_FOR_WORK) {
            return;
        }

        mla_sleep(250); // Sleep for 250 milliseconds
    }

}

mla_multi_task_mode mla_task_manager_windows_multi_task_mode() {
    return MULTI_TASK_MODE_NATIVE;
}

mla_task_manager_low_level_access g_task_low_level_access = {
        mla_task_manager_windows_native_create_task,
        mla_task_manager_windows_native_run,
        mla_task_manager_windows_native_create_mutex,
        mla_task_manager_windows_native_lock_mutex,
        mla_task_manager_windows_native_unlock_mutex,
        mla_task_manager_windows_native_destroy_mutex,
        mla_task_manager_windows_multi_task_mode
};


#endif
