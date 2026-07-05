//
// Created by chris on 9/12/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_TASK_MANAGER_ESP32_H
#define MLA_GLOBAL_PLATFORM_TASK_MANAGER_ESP32_H


#include "../../core/task/mla_task_manager.h"
#include "../../core/mla_native_resource.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/////////////////////////////////////////////////////////////
/// ESP32 Native Task Manager
/////////////////////////////////////////////////////////////

struct mla_task_manager_esp32_native_data_t {
    TaskHandle_t hThread;
    mla_user_data_t userData;
    mla_task_worker_t worker;
    mla_pointer_t sharedStates;

    static mla_task_manager_esp32_native_data_t init() {
        return {
            nullptr,               // hThread
            mla_user_data_empty(),
            nullptr,               // worker
            mla_pointer_null()     // sharedStates
        };
    }

    static void clean_up_resource(mla_task_manager_esp32_native_data_t& self) {

        TaskHandle_t currentHandle = self.hThread;

        if (currentHandle != nullptr) {
            self.hThread = nullptr;
            vTaskDelete(currentHandle);
        }

        mla_task_shared_states* shared_states = mla_pointer_get_data<mla_task_shared_states>(self.sharedStates);
        if (shared_states != nullptr) {
            mla_task_manager_esp32_atomic_exchange(shared_states->processingState, TASK_STATE_ABORTED);
        }

        // Release the reference to shared states
        self.sharedStates = mla_pointer_null();
    }
};

configSTACK_DEPTH_TYPE mla_task_manager_esp32_native_get_stack_size(const mla_task_stack_size stackSize) {

    switch (stackSize) {
        case TASK_STACK_SIZE_TINY:
            return 2048; // 2 KB  (ESP32 minimum stack size is 2KB)
        case TASK_STACK_SIZE_SMALL:
            return 4096; // 4 KB
        case TASK_STACK_SIZE_MEDIUM:
            return 8192; // 8 KB
        case TASK_STACK_SIZE_LARGE:
            return 12288; // 12 KB
        case TASK_STACK_SIZE_XLARGE:
            return 16384; // 16 KB
        case TASK_STACK_SIZE_XXLARGE:
            return 24576; // 24 KB
        default:
            return 8192;
    }
}

UBaseType_t mla_task_manager_esp32_native_get_priority(const mla_task_priority priority) {

    switch (priority) {
        case TASK_PRIO_LOW:
            return tskIDLE_PRIORITY + 1; // Low priority
        case TASK_PRIO_NORMAL:
            return tskIDLE_PRIORITY + 2; // Normal priority
        case TASK_PRIO_HIGH:
            return tskIDLE_PRIORITY + 3; // High priority
        default:
            return tskIDLE_PRIORITY + 2; // Default to normal priority
    }
}

////////////////////////////////////////////////////////////////////////////
/// Atomic Implementation
////////////////////////////////////////////////////////////////////////////

#define mla_atomic_memory_order __ATOMIC_SEQ_CST

inline mla_int32_t mla_task_manager_esp32_atomic_increment(mla_atomic_int32_t& value) {
    return __atomic_add_fetch(&value.value, 1, mla_atomic_memory_order);
}

inline mla_int32_t mla_task_manager_esp32_atomic_decrement(mla_atomic_int32_t& value) {
    return __atomic_sub_fetch(&value.value, 1, mla_atomic_memory_order);
}

inline mla_int32_t mla_task_manager_esp32_atomic_add(mla_atomic_int32_t& value, mla_int32_t addend) {
    return __atomic_add_fetch(&value.value, addend, mla_atomic_memory_order);
}

inline mla_int32_t mla_task_manager_esp32_atomic_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend) {
    return __atomic_sub_fetch(&value.value, subtrahend, mla_atomic_memory_order);
}

inline mla_int32_t mla_task_manager_esp32_atomic_exchange(mla_atomic_int32_t& value, mla_int32_t newValue) {
    return __atomic_exchange_n(&value.value, newValue, mla_atomic_memory_order);
}

inline mla_bool_t mla_task_manager_esp32_atomic_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue) {
    return __atomic_compare_exchange_n(&value.value, &expectedValue, newValue, false, mla_atomic_memory_order, __ATOMIC_SEQ_CST);
}

void mla_private_task_manager_esp32_native_worker(void * param) {

    mla_task_manager_esp32_native_data_t* thread_data = mla_s_cast<mla_task_manager_esp32_native_data_t*>(param);

    if (thread_data) {

        mla_task_shared_states* shared_states = mla_pointer_get_data<mla_task_shared_states>(thread_data->sharedStates);

        if (shared_states == nullptr) {
            thread_data->hThread = nullptr;
            vTaskDelete(nullptr);
            return; // No shared states, nothing to do
        }

        if (thread_data->worker == nullptr) {
            mla_task_manager_esp32_atomic_exchange(shared_states->processingState, TASK_STATE_COMPLETED);
            thread_data->hThread = nullptr;
            mla_error(mla_string_const("Unable to start task, no worker function provided"));
            vTaskDelete(nullptr);
            return; // Exit if no worker function is provided
        }

        mla_debug("Starting task worker");

        while (true) {

            if (mla_task_manager_esp32_atomic_compare_exchange(shared_states->processingState, TASK_STATE_ABORTING, TASK_STATE_ABORTED)) {
                mla_error(mla_string_const("Task is aborting, stopping execution"));
                break; // Exit the loop if the task is aborted
            }

            // Only switch to RUNNING if it was previously RUNNING or STARTING.
            // If it is ABORTING, we must not overwrite it.
            mla_task_manager_esp32_atomic_compare_exchange(shared_states->processingState, TASK_STATE_STARTING, TASK_STATE_RUNNING);

            mla_task_process_result_state result_state = thread_data->worker(thread_data->userData); // Call the worker function

            if (result_state != TASK_PROCESS_RESULT_CONTINUE) {
                mla_task_manager_esp32_atomic_exchange(shared_states->processingState, TASK_STATE_COMPLETED); // Set the state to completed if the worker function returns DONE
                mla_debug("Task processing complete");
                break; // Exit the loop after completion
            }

            vTaskDelay(1); // Yield to other tasks

        }

    }
    // Task completed, delete itself
    thread_data->hThread = nullptr;
    vTaskDelete(nullptr);
}

mla_bool_t mla_task_manager_esp32_native_create_task(
        const mla_task_worker_t worker,
        const mla_string_t& task_name,
        mla_user_data_t& user_data,
        const mla_task_stack_size stackSize,
        const mla_task_priority priority,
        mla_pointer_t& outTaskResourceOwner,
        const mla_pointer_t& shared_states) {

    mla_pointer_t thread_data_ptr = mla_malloc_struct_cleanup_extension(mla_task_manager_esp32_native_data_t);
    mla_task_manager_esp32_native_data_t* thread_data = mla_pointer_get_data<mla_task_manager_esp32_native_data_t>(thread_data_ptr);

    if (thread_data == nullptr) {
        return false;
    }

    mla_memset(thread_data, 0, sizeof(mla_task_manager_esp32_native_data_t));
    thread_data->worker = worker;
    thread_data->userData = user_data;
    thread_data->sharedStates = shared_states;

    configSTACK_DEPTH_TYPE stackSizeInBytes = mla_task_manager_esp32_native_get_stack_size(stackSize);
    UBaseType_t prio = mla_task_manager_esp32_native_get_priority(priority);

    mla_char_t taskNameBuffer[configMAX_TASK_NAME_LEN] = {0};
    mla_memcpy(taskNameBuffer, mla_string_data(task_name), mla_min(mla_string_length(task_name), configMAX_TASK_NAME_LEN - 1));

    mla_bool_t success = xTaskCreate(mla_private_task_manager_esp32_native_worker, taskNameBuffer, stackSizeInBytes, thread_data, prio, &thread_data->hThread) == pdTRUE;

    if (!success) {
        outTaskResourceOwner = mla_pointer_null();
        return false; // Failed to create thread
    }

    // Return the thread handle through outTaskResourceOwner
    outTaskResourceOwner = thread_data_ptr;
    return true; // Successfully created the task
}

void mla_task_manager_esp32_native_run() {

    while (true) {

        mla_task_manager_state state = mla_task_manager_get_state();

        if (state == TASK_MANAGER_STATE_WAITING_FOR_WORK) {
            return;
        }

        mla_sleep(250); // Sleep for 250 milliseconds
    }

}


////////////////////////////////////////////////////////////////////////////
/// Mutex Implementation
////////////////////////////////////////////////////////////////////////////

struct mla_task_manager_esp32_native_mutex_t {
    SemaphoreHandle_t semaphore;
    mla_bool_t recursive;

    static mla_task_manager_esp32_native_mutex_t init() {
        return {
            nullptr, // semaphore
            false    // recursive
        };
    }

    static void clean_up_resource(mla_task_manager_esp32_native_mutex_t& self) {
        if (self.semaphore != nullptr) {
            vSemaphoreDelete(self.semaphore);
            self.semaphore = nullptr;
        }
    }
};

mla_bool_t mla_task_manager_esp32_native_create_mutex(mla_pointer_t& outMutex, mla_bool_t supports_recursive_locking) {

    mla_pointer_t mutex_ptr = mla_malloc_struct_cleanup_extension(mla_task_manager_esp32_native_mutex_t);
    mla_task_manager_esp32_native_mutex_t* mutex = mla_pointer_get_data<mla_task_manager_esp32_native_mutex_t>(mutex_ptr);

    if (mutex == nullptr) {
        return false;
    }

    mutex->recursive = supports_recursive_locking;

    if (supports_recursive_locking) {
        mutex->semaphore = xSemaphoreCreateRecursiveMutex();
    } else {
        mutex->semaphore = xSemaphoreCreateMutex();
    }

    if (mutex->semaphore == nullptr) {
        outMutex = mla_pointer_null();
        return false;
    }

    outMutex = mutex_ptr;
    return true;
}

mla_bool_t mla_task_manager_esp32_native_lock_mutex(const mla_pointer_t& mutexResource, mla_int32_t timeoutms) {

    mla_task_manager_esp32_native_mutex_t* mutex = mla_pointer_get_data<mla_task_manager_esp32_native_mutex_t>(mutexResource);

    if (mutex == nullptr || mutex->semaphore == nullptr) {
        return false;
    }

    if (mutex->recursive) {
        return xSemaphoreTakeRecursive(mutex->semaphore, timeoutms / portTICK_PERIOD_MS) == pdTRUE;
    } else {
        return xSemaphoreTake(mutex->semaphore, timeoutms / portTICK_PERIOD_MS) == pdTRUE;
    }
}

mla_bool_t mla_task_manager_esp32_native_unlock_mutex(const mla_pointer_t& mutexResource) {

    mla_task_manager_esp32_native_mutex_t* mutex = mla_pointer_get_data<mla_task_manager_esp32_native_mutex_t>(mutexResource);
    if (mutex == nullptr || mutex->semaphore == nullptr) {
        return false; // Mutex resource is null
    }

    if (mutex->recursive) {
        return xSemaphoreGiveRecursive(mutex->semaphore) == pdTRUE;
    } else {
        return xSemaphoreGive(mutex->semaphore) == pdTRUE;
    }
}

mla_multi_task_mode mla_task_manager_esp32_native_get_multi_task_mode() {
    return MULTI_TASK_MODE_NATIVE;
}

////////////////////////////////////////////////////////////////////////////
/// Task Local Storage Implementation
////////////////////////////////////////////////////////////////////////////

// Uses FreeRTOS Thread Local Storage Pointers
// Each task local allocates a unique index into the per-task TLS array.

static mla_bool_t g_esp32_task_local_slots_used[configNUM_THREAD_LOCAL_STORAGE_POINTERS] = {};

struct mla_task_manager_esp32_task_local {
    BaseType_t index;

    static mla_task_manager_esp32_task_local init() {
        return {
            -1 // index
        };
    }

    static void clean_up_resource(mla_task_manager_esp32_task_local& self) {
        g_esp32_task_local_slots_used[self.index] = false;
        vTaskSetThreadLocalStoragePointer(nullptr, self.index, nullptr);
    }
};

mla_bool_t mla_task_manager_esp32_create_task_local(mla_pointer_t& outTaskLocal) {

    // Find a free slot
    BaseType_t freeIndex = -1;
    for (BaseType_t i = 0; i < configNUM_THREAD_LOCAL_STORAGE_POINTERS; i++) {
        if (!g_esp32_task_local_slots_used[i]) {
            freeIndex = i;
            break;
        }
    }

    if (freeIndex < 0) {
        return false; // No free slots available
    }

    mla_pointer_t local_ptr = mla_malloc_struct_cleanup_extension(mla_task_manager_esp32_task_local);
    mla_task_manager_esp32_task_local* local = mla_pointer_get_data<mla_task_manager_esp32_task_local>(local_ptr);

    if (local == nullptr) {
        return false;
    }

    g_esp32_task_local_slots_used[freeIndex] = true;
    local->index = freeIndex;
    outTaskLocal = local_ptr;

    return true;
}

mla_bool_t mla_task_manager_esp32_set_task_local(const mla_pointer_t& taskLocal, mla_platform_pointer_t value) {

    mla_task_manager_esp32_task_local* local = mla_pointer_get_data<mla_task_manager_esp32_task_local>(taskLocal);

    if (local == nullptr) {
        return false;
    }

    vTaskSetThreadLocalStoragePointer(nullptr, local->index, value);
    return true;
}

mla_platform_pointer_t mla_task_manager_esp32_get_task_local(const mla_pointer_t& taskLocal) {

    mla_task_manager_esp32_task_local* local = mla_pointer_get_data<mla_task_manager_esp32_task_local>(taskLocal);

    if (local == nullptr) {
        return nullptr;
    }

    return pvTaskGetThreadLocalStoragePointer(nullptr, local->index);
}

mla_task_manager_low_level_access g_task_low_level_access = {
    mla_task_manager_esp32_native_create_task,
    mla_task_manager_esp32_native_run,
    mla_task_manager_esp32_native_create_mutex,
    mla_task_manager_esp32_native_lock_mutex,
    mla_task_manager_esp32_native_unlock_mutex,
    mla_task_manager_esp32_native_get_multi_task_mode,
    mla_task_manager_esp32_create_task_local,
    mla_task_manager_esp32_set_task_local,
    mla_task_manager_esp32_get_task_local,
    mla_task_manager_esp32_atomic_increment,
    mla_task_manager_esp32_atomic_decrement,
    mla_task_manager_esp32_atomic_add,
    mla_task_manager_esp32_atomic_subtract,
    mla_task_manager_esp32_atomic_exchange,
    mla_task_manager_esp32_atomic_compare_exchange
};


#endif