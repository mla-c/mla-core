//
// Created by chris on 8/10/2025.
//

#ifndef MLA_TASK_MANAGER_WINDOWS_NATIVE_H
#define MLA_TASK_MANAGER_WINDOWS_NATIVE_H

#include "../../core/mla_native_resource.h"
#include "../../core/task/mla_task_manager.h"
#include <windows.h>
#include <assert.h>

static_assert(sizeof(mla_int32_t) == sizeof(LONG),
              "mla_int32_t must be 32-bit for Interlocked APIs");

inline mla_int32_t mla_task_manager_windows_atomic_int32_increment(mla_atomic_int32_t& value) {
    return InterlockedIncrement(reinterpret_cast<volatile LONG*>(&value.value));
}

inline mla_int32_t mla_task_manager_windows_atomic_int32_decrement(mla_atomic_int32_t& value) {
    return InterlockedDecrement(reinterpret_cast<volatile LONG*>(&value.value));
}

inline mla_int32_t mla_task_manager_windows_atomic_int32_add(mla_atomic_int32_t& value, mla_int32_t addend) {
    return InterlockedAdd(reinterpret_cast<volatile LONG*>(&value.value), static_cast<LONG>(addend));
}

inline mla_int32_t mla_task_manager_windows_atomic_int32_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend) {
    return InterlockedAdd(reinterpret_cast<volatile LONG*>(&value.value), static_cast<LONG>(-subtrahend));
}

inline mla_int32_t mla_task_manager_windows_atomic_int32_exchange(mla_atomic_int32_t& value, mla_int32_t newValue) {
    return InterlockedExchange(reinterpret_cast<volatile LONG*>(&value.value), static_cast<LONG>(newValue));
}


struct mla_task_manager_windows_native_data_t {
    HANDLE hThread;
    mla_user_data_t userData;
    mla_task_worker_t worker;
    mla_pointer_t sharedStates; //mla_task_shared_states*;

    static void clean_up_resource(mla_platform_pointer_t data) {

        mla_task_manager_windows_native_data_t* thread_data = static_cast<mla_task_manager_windows_native_data_t*>(data);

        if (thread_data) {
            // Stop the thread if it is still running
            if (thread_data->hThread != nullptr) {
                TerminateThread(thread_data->hThread, 0);
                CloseHandle(thread_data->hThread);
            }

            mla_task_shared_states* shared_states = mla_pointer_get_data<mla_task_shared_states>(thread_data->sharedStates);

            if (shared_states != nullptr) {
                // Set the shared state to aborted if the thread is still running
                mla_task_manager_windows_atomic_int32_exchange(shared_states->processingState, TASK_STATE_ABORTED);
            }

            // Reset the ref counter
            thread_data->sharedStates = mla_pointer_null();

        }

     }
};

inline mla_bool_t mla_task_manager_windows_atomic_int32_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue) {

    LONG prev = InterlockedCompareExchange(
        reinterpret_cast<volatile LONG*>(&value.value),
        static_cast<LONG>(newValue),
        static_cast<LONG>(expectedValue));
    return prev == static_cast<LONG>(expectedValue);

}

DWORD WINAPI __mla_task_manager_windows_native_worker(LPVOID lpParam) {

    mla_task_manager_windows_native_data_t* thread_data = static_cast<mla_task_manager_windows_native_data_t*>(lpParam);

    if (thread_data) {

        mla_task_shared_states* shared_states = mla_pointer_get_data<mla_task_shared_states>(thread_data->sharedStates);

        if (shared_states == nullptr) {
            return 0; // No shared states, nothing to do
        }

        if (thread_data->worker == nullptr) {
            mla_task_manager_windows_atomic_int32_exchange(shared_states->processingState, TASK_STATE_COMPLETED);
            return 0;
        }

        while (true) {

            if (mla_task_manager_windows_atomic_int32_compare_exchange(shared_states->processingState, TASK_STATE_ABORTING, TASK_STATE_ABORTED)) {
                break; // Exit the loop if the task is aborted
            }

            // Only switch to RUNNING if it was previously RUNNING or STARTING.
            // If it is ABORTING, we must not overwrite it.
            mla_task_manager_windows_atomic_int32_compare_exchange(shared_states->processingState, TASK_STATE_STARTING, TASK_STATE_RUNNING);

            mla_task_process_result_state result_state = thread_data->worker(thread_data->userData); // Call the worker function

            if (result_state != TASK_PROCESS_RESULT_CONTINUE) {
                mla_task_manager_windows_atomic_int32_exchange(shared_states->processingState, TASK_STATE_COMPLETED); // Set the state to completed if the worker function returns DONE
                break; // Exit the loop after completion
            }

            // Sleep for a short duration to prevent busy-waiting
            Sleep(10);

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
        const mla_string_t& task_name,
        mla_user_data_t& user_data,
        const mla_task_stack_size stackSize,
        const mla_task_priority priority,
        mla_pointer_t& outTaskResourceOwner,
        const mla_pointer_t& shared_states) {

    mla_pointer_t thread_data_ptr = mla_malloc_native_resource_struct(mla_task_manager_windows_native_data_t);

    mla_task_manager_windows_native_data_t* thread_data = static_cast<mla_task_manager_windows_native_data_t*>(mla_platform_malloc(sizeof(mla_task_manager_windows_native_data_t)));

    if (thread_data == nullptr) {
        return false;
    }

    mla_memset(thread_data, 0, sizeof(mla_task_manager_windows_native_data_t));
    thread_data->worker = worker;
    thread_data->userData = user_data;
    thread_data->sharedStates = shared_states;

    SIZE_T stackSizeInBytes = mla_task_manager_windows_native_get_stack_size(stackSize);
    thread_data->hThread = CreateThread(nullptr, stackSizeInBytes, __mla_task_manager_windows_native_worker, thread_data, 0, nullptr);

    if (thread_data->hThread == nullptr) {
        mla_platform_free(thread_data);
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


#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0A00)
    if (!mla_string_is_empty(task_name)) {
        mla_string_utf16_buffer_t utf16 = mla_string_to_utf16_buffer(task_name);

        PCWSTR thread_name = mla_pointer_get_data<WCHAR>(utf16.data);

        if (thread_name != nullptr) {
            SetThreadDescription(thread_data->hThread, thread_name);
        }
    }
#else
    (void)task_name; // Unused parameter on older Windows versions
#endif


    // Return the thread handle through outTaskResourceOwner
    outTaskResourceOwner = thread_data_ptr;
    return true; // Successfully created the task
}

// Windows native mutex implementation is to slow and supports recursive locks
// We will use a CRITICAL_SECTION for the mutex implementation
struct mla_task_manager_windows_native_mutex_t {
    CRITICAL_SECTION section;
    mla_bool_t locked; // already locked by the current thread
    mla_bool_t supports_recursive_locking; // whether the mutex supports recursive locking

    static void clean_up_resource(mla_platform_pointer_t data) {

        mla_task_manager_windows_native_mutex_t* mutex = static_cast<mla_task_manager_windows_native_mutex_t*>(data);
        DeleteCriticalSection(&mutex->section); // Delete the critical section
    }
};

mla_bool_t mla_task_manager_windows_native_create_mutex(mla_pointer_t& outMutex, mla_bool_t supports_recursive_locking) {

    mla_pointer_t mutex_ptr = mla_malloc_native_resource_struct(mla_task_manager_windows_native_mutex_t);

    mla_task_manager_windows_native_mutex_t* mutex = mla_pointer_get_data<mla_task_manager_windows_native_mutex_t>(mutex_ptr);

    if (mutex == nullptr) {
        return false; // Failed to allocate memory for mutex
    }

    InitializeCriticalSection(&mutex->section);
    mutex->locked = false; // Initially not locked
    mutex->supports_recursive_locking = supports_recursive_locking;

    outMutex = mutex_ptr;
    return true; // Successfully created the mutex

}

mla_bool_t mla_task_manager_windows_native_lock_mutex(const mla_pointer_t& mutex_resource, mla_int32_t timeoutms) {

    mla_task_manager_windows_native_mutex_t* mutex = mla_pointer_get_data<mla_task_manager_windows_native_mutex_t>(mutex_resource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    // Windows does not support timeouts for critical sections directly,
    // so we will simulate it with a sleep loop.

    for (int i = 0; i < timeoutms / 10; ++i) {

        if (TryEnterCriticalSection(&mutex->section)) {

            if (mutex->supports_recursive_locking) {
                return true; // Successfully locked the mutex
            }

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

mla_bool_t mla_task_manager_windows_native_unlock_mutex(const mla_pointer_t& mutex_resource) {

    mla_task_manager_windows_native_mutex_t* mutex =  mla_pointer_get_data<mla_task_manager_windows_native_mutex_t>(mutex_resource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    if (!mutex->supports_recursive_locking) {

        if (TryEnterCriticalSection(&mutex->section)) {

            if (!mutex->locked) {
                LeaveCriticalSection(&mutex->section); // Leave the critical section if it was not locked
                return false; // Mutex was not locked by this thread
            }

            // Locked by this thread, so we can unlock it
            mutex->locked = false; // Mark as unlocked
            LeaveCriticalSection(&mutex->section); // Leave the critical section
            return true; // Successfully unlocked the mutex
        } else {
            return false; // Mutex was not locked by this thread
        }

    }

    LeaveCriticalSection(&mutex->section); // Leave the critical section
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

void __mla_task_manager_windows_native_destroy_task_local(const mla_native_resource_t& userData) {

    mla_bool_t success = FlsFree(userData.asUint32) != 0;

     if (!success) {
         mla_error("Failed to free FLS index for task local storage");
     }
}

mla_bool_t mla_task_manager_windows_native_create_task_local(mla_pointer_t& outTaskLocal) {

    DWORD flsIndex = FlsAlloc(nullptr);
    if (flsIndex == FLS_OUT_OF_INDEXES) {
        return false;
    }

    mla_native_resource_t resource = mla_dynamic_data_from_uint32(flsIndex);

    outTaskLocal = mla_native_resource_to_managed_pointer(resource, __mla_task_manager_windows_native_destroy_task_local);

    if (mla_pointer_is_null(outTaskLocal)) {
        FlsFree(flsIndex);
        return false;
    }

    return false;

}


mla_bool_t mla_task_manager_windows_native_set_task_local(const mla_pointer_t& resource, mla_platform_pointer_t value) {

    mla_native_resource_t* data = mla_native_resource_from_managed_pointer(resource);

    if (data == nullptr) {
        return false;
    }

    return FlsSetValue(data->asUint32, value) != 0;
}

mla_platform_pointer_t mla_task_manager_windows_native_get_task_local(const mla_pointer_t& resource) {

    mla_native_resource_t* data = mla_native_resource_from_managed_pointer(resource);

    if (data == nullptr) {
        return nullptr;
    }

    return FlsGetValue(data->asUint32);
}

mla_task_manager_low_level_access g_task_low_level_access = {
        mla_task_manager_windows_native_create_task,
        mla_task_manager_windows_native_run,
        mla_task_manager_windows_native_create_mutex,
        mla_task_manager_windows_native_lock_mutex,
        mla_task_manager_windows_native_unlock_mutex,
        mla_task_manager_windows_multi_task_mode,
        mla_task_manager_windows_native_create_task_local,
        mla_task_manager_windows_native_set_task_local,
        mla_task_manager_windows_native_get_task_local,
        mla_task_manager_windows_atomic_int32_increment,
        mla_task_manager_windows_atomic_int32_decrement,
    mla_task_manager_windows_atomic_int32_add,
    mla_task_manager_windows_atomic_int32_subtract,
    mla_task_manager_windows_atomic_int32_exchange,
    mla_task_manager_windows_atomic_int32_compare_exchange

};


#endif
