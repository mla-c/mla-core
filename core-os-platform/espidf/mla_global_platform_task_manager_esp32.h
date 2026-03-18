//
// Created by chris on 9/12/2025.
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_TASK_MANAGER_ESP32_H
#define COREOS_MLA_GLOBAL_PLATFORM_TASK_MANAGER_ESP32_H


#include "../../core-os/task/mla_task_manager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/////////////////////////////////////////////////////////////
/// ESP32 Native Task Manager
/////////////////////////////////////////////////////////////

struct mla_task_manager_esp32_native_data_t {
    TaskHandle_t hThread;
    mla_user_data_t userData;
    mla_task_worker_t worker;
    mla_task_shared_states* sharedStates;
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

mla_buffer_cleanup_mode __mla_task_manager_esp32_native_cleanup(mla_pointer_t data, const mla_dynamic_data_t& userData) {

    mla_task_manager_esp32_native_data_t* thread_data = static_cast<mla_task_manager_esp32_native_data_t*>(data);

    if (thread_data) {
        // Stop the thread if it is still running
        TaskHandle_t currentHandle = thread_data->hThread;

        if (currentHandle != nullptr) {
            thread_data->hThread = nullptr;
            vTaskDelete(currentHandle);
        }

        // Dont set the sharedState this is already gone
        //thread_data->sharedStates->processingState = TASK_STATE_ABORTED;
    }

    return CLEAN_UP_NEEDED;
}

void __mla_task_manager_esp32_native_worker(void * param) {


    mla_task_manager_esp32_native_data_t* thread_data = static_cast<mla_task_manager_esp32_native_data_t*>(param);

    if (thread_data) {

        mla_task_shared_states* shared_states = thread_data->sharedStates;

        if (thread_data->worker == nullptr) {
            shared_states->processingState = TASK_STATE_COMPLETED;
            thread_data->hThread = nullptr;
            mla_error(mla_string_const("Unable to start task, no worker function provided"));
            vTaskDelete(nullptr);
            return; // Exit if no worker function is provided
        }

        mla_debug("Starting task worker");

        while (true) {

            if (shared_states->processingState == TASK_STATE_ABORTING) {
                mla_error(mla_string_const("Task is aborting, stopping execution"));
                shared_states->processingState = TASK_STATE_ABORTED;
                break; // Exit the loop if the task is aborted
            }

            shared_states->processingState = TASK_STATE_RUNNING; // Set the state to running
            mla_task_process_result_state result_state = thread_data->worker(thread_data->userData); // Call the worker function

            if (result_state != TASK_PROCESS_RESULT_CONTINUE) {
                shared_states->processingState = TASK_STATE_COMPLETED; // Set the state to completed if the worker function returns DONE
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
        mla_buffer_reference_t* outTaskResourceOwner,
        mla_task_shared_states* shared_states) {

    mla_task_manager_esp32_native_data_t* thread_data = static_cast<mla_task_manager_esp32_native_data_t*>(mla_malloc(sizeof(mla_task_manager_esp32_native_data_t)));

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

    mla_bool_t success = xTaskCreate(__mla_task_manager_esp32_native_worker, taskNameBuffer, stackSizeInBytes, thread_data, prio, &thread_data->hThread) == pdTRUE;

    if (!success) {
        mla_free(thread_data);
        return false; // Failed to create thread
    }

    // Return the thread handle through outTaskResourceOwner
    *outTaskResourceOwner = mla_buffer_reference_create(thread_data, true, __mla_task_manager_esp32_native_cleanup, mla_dynamic_data_empty());
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
};

mla_bool_t mla_task_manager_esp32_native_create_mutex(mla_pointer_t* outMutex, mla_bool_t supports_recursive_locking) {
    if (outMutex == nullptr) {
        return false;
    }

    mla_task_manager_esp32_native_mutex_t* mutex =
    static_cast<mla_task_manager_esp32_native_mutex_t*>(mla_malloc(sizeof(mla_task_manager_esp32_native_mutex_t)));

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
        mla_free(mutex);
        return false;
    }

    *outMutex = static_cast<mla_pointer_t>(mutex);
    return true;
}

mla_bool_t mla_task_manager_esp32_native_destroy_mutex(mla_pointer_t mutexResource) {

    mla_task_manager_esp32_native_mutex_t* mutex = static_cast<mla_task_manager_esp32_native_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    if (mutex->semaphore == nullptr) {
        return false; // Mutex was never created
    }

    vSemaphoreDelete(mutex->semaphore); // Delete the semaphore
    mla_free(mutex); // Free the mutex memory
    return true;
}

mla_bool_t mla_task_manager_esp32_native_lock_mutex(mla_pointer_t mutexResource, mla_int32_t timeoutms) {

    mla_task_manager_esp32_native_mutex_t* mutex = static_cast<mla_task_manager_esp32_native_mutex_t*>(mutexResource);

    if (mutex == nullptr || mutex->semaphore == nullptr) {
        return false;
    }

    if (mutex->recursive) {
        return xSemaphoreTakeRecursive(mutex->semaphore, timeoutms / portTICK_PERIOD_MS) == pdTRUE;
    } else {
        return xSemaphoreTake(mutex->semaphore, timeoutms / portTICK_PERIOD_MS) == pdTRUE;
    }
}

mla_bool_t mla_task_manager_esp32_native_unlock_mutex(mla_pointer_t mutexResource) {

    mla_task_manager_esp32_native_mutex_t* mutex = static_cast<mla_task_manager_esp32_native_mutex_t*>(mutexResource);
    if (mutex == nullptr || mutex->semaphore == nullptr) {
        return false; // Mutex resource is null
    }

    if (mutex->recursive) {
        return xSemaphoreGiveRecursive(mutex->semaphore) == pdTRUE;
    } else {
        return xSemaphoreGive(mutex->semaphore) == pdTRUE;
    }
}

////////////////////////////////////////////////////////////////////////////
/// Atomic Implementation
////////////////////////////////////////////////////////////////////////////

#define mla_atomic_memory_order __ATOMIC_SEQ_CST

mla_multi_task_mode mla_task_manager_esp32_native_get_multi_task_mode() {
    return MULTI_TASK_MODE_NATIVE;
}

mla_int32_t mla_task_manager_esp32_atomic_increment(mla_atomic_int32_t& value) {
    return __atomic_add_fetch(&value.value, 1, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_esp32_atomic_decrement(mla_atomic_int32_t& value) {
    return __atomic_sub_fetch(&value.value, 1, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_esp32_atomic_add(mla_atomic_int32_t& value, mla_int32_t addend) {
    return __atomic_add_fetch(&value.value, addend, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_esp32_atomic_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend) {
    return __atomic_sub_fetch(&value.value, subtrahend, mla_atomic_memory_order);
}

mla_int32_t mla_task_manager_esp32_atomic_exchange(mla_atomic_int32_t& value, mla_int32_t newValue) {
    return __atomic_exchange_n(&value.value, newValue, mla_atomic_memory_order);
}

mla_bool_t mla_task_manager_esp32_atomic_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue) {
    return __atomic_compare_exchange_n(&value.value, &expectedValue, newValue, false, mla_atomic_memory_order, __ATOMIC_SEQ_CST);
}


////////////////////////////////////////////////////////////////////////////
/// Task Local Storage Implementation
////////////////////////////////////////////////////////////////////////////

// Uses FreeRTOS Thread Local Storage Pointers
// Each task local allocates a unique index into the per-task TLS array.

static mla_bool_t g_esp32_task_local_slots_used[configNUM_THREAD_LOCAL_STORAGE_POINTERS] = {};

struct mla_task_manager_esp32_task_local {
    BaseType_t index;
};

mla_bool_t mla_task_manager_esp32_create_task_local(mla_pointer_t* outTaskLocal) {

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

    mla_task_manager_esp32_task_local* local = static_cast<mla_task_manager_esp32_task_local*>(mla_malloc(sizeof(mla_task_manager_esp32_task_local)));

    if (local == nullptr) {
        return false;
    }

    g_esp32_task_local_slots_used[freeIndex] = true;
    local->index = freeIndex;
    *outTaskLocal = static_cast<mla_pointer_t>(local);

    return true;
}

mla_bool_t mla_task_manager_esp32_destroy_task_local(mla_pointer_t taskLocal) {

    mla_task_manager_esp32_task_local* local = static_cast<mla_task_manager_esp32_task_local*>(taskLocal);

    if (local == nullptr) {
        return true;
    }

    g_esp32_task_local_slots_used[local->index] = false;
    vTaskSetThreadLocalStoragePointer(nullptr, local->index, nullptr);
    mla_free(local);
    return true;
}

mla_bool_t mla_task_manager_esp32_set_task_local(mla_pointer_t taskLocal, mla_pointer_t value) {

    mla_task_manager_esp32_task_local* local = static_cast<mla_task_manager_esp32_task_local*>(taskLocal);

    if (local == nullptr) {
        return false;
    }

    vTaskSetThreadLocalStoragePointer(nullptr, local->index, value);
    return true;
}

mla_pointer_t mla_task_manager_esp32_get_task_local(mla_pointer_t taskLocal) {

    mla_task_manager_esp32_task_local* local = static_cast<mla_task_manager_esp32_task_local*>(taskLocal);

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
    mla_task_manager_esp32_native_destroy_mutex,
    mla_task_manager_esp32_native_get_multi_task_mode,
    mla_task_manager_esp32_create_task_local,
    mla_task_manager_esp32_destroy_task_local,
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