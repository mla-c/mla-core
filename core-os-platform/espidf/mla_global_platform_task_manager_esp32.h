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
    mla_callback_userdata userData;
    mla_callback_userdata userData2;
    mla_task_worker_t worker;
    mla_task_shared_states* sharedStates;
};

configSTACK_DEPTH_TYPE mla_task_manager_esp32_native_get_stack_size(const mla_task_stack_size stackSize) {

    switch (stackSize) {
        case TASK_STACK_SIZE_TINY:
            return 512; // 512 Bytes
        case TASK_STACK_SIZE_SMALL:
            return 1024; // 1 KB
        case TASK_STACK_SIZE_MEDIUM:
            return 2048; // 2 KB (ESP32 minimum stack size is 2KB)
        case TASK_STACK_SIZE_LARGE:
            return 4096; // 4 KB
        case TASK_STACK_SIZE_XLARGE:
            return 8192; // 8 KB
        case TASK_STACK_SIZE_XXLARGE:
            return 16384; // 16 KB
        default:
            return 2048;
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

mla_buffer_cleanup_mode __mla_task_manager_esp32_native_cleanup(mla_pointer_t data, mla_callback_userdata userData) {

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
            vTaskDelete(nullptr);
            return; // Exit if no worker function is provided
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

            vTaskDelay(1); // Yield to other tasks

        }

    }
    // Task completed, delete itself
    thread_data->hThread = nullptr;
    vTaskDelete(nullptr);
}

mla_bool_t mla_task_manager_esp32_native_create_task(
        const mla_task_worker_t worker,
        mla_callback_userdata userData,
        mla_callback_userdata userData2,
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
    thread_data->userData = userData;
    thread_data->userData2 = userData2;
    thread_data->sharedStates = shared_states;

    configSTACK_DEPTH_TYPE stackSizeInBytes = mla_task_manager_esp32_native_get_stack_size(stackSize);
    UBaseType_t prio = mla_task_manager_esp32_native_get_priority(priority);
    mla_bool_t success = xTaskCreate(__mla_task_manager_esp32_native_worker, "MLATask", stackSizeInBytes, thread_data, prio, &thread_data->hThread) == pdTRUE;

    if (!success) {
        mla_free(thread_data);
        return false; // Failed to create thread
    }

    // Return the thread handle through outTaskResourceOwner
    *outTaskResourceOwner = mla_buffer_reference(thread_data, __mla_task_manager_esp32_native_cleanup);
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
};

mla_bool_t mla_task_manager_esp32_native_create_mutex(mla_pointer_t* outMutex) {

    mla_task_manager_esp32_native_mutex_t* mutex = static_cast<mla_task_manager_esp32_native_mutex_t*>(mla_malloc(sizeof(mla_task_manager_esp32_native_mutex_t)));

    if (mutex == nullptr) {
        return false; // Failed to allocate memory for mutex
    }

    mutex->semaphore = xSemaphoreCreateMutex();

    if (mutex->semaphore == nullptr) {
        mla_free(mutex);
        return false; // Failed to create the mutex
    }

    *outMutex = static_cast<mla_pointer_t>(mutex);
    return true; // Successfully created the mutex

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
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    return xSemaphoreTake(mutex->semaphore, timeoutms / portTICK_PERIOD_MS) == pdTRUE;
}

mla_bool_t mla_task_manager_esp32_native_unlock_mutex(mla_pointer_t mutexResource) {

    mla_task_manager_esp32_native_mutex_t* mutex = static_cast<mla_task_manager_esp32_native_mutex_t*>(mutexResource);
    if (mutex == nullptr) {
        return false; // Mutex resource is null
    }

    return xSemaphoreGive(mutex->semaphore) == pdTRUE;
}


mla_task_manager_low_level_access g_task_low_level_access = {
    mla_task_manager_esp32_native_create_task,
    mla_task_manager_esp32_native_run,
    mla_task_manager_esp32_native_create_mutex,
    mla_task_manager_esp32_native_lock_mutex,
    mla_task_manager_esp32_native_unlock_mutex,
    mla_task_manager_esp32_native_destroy_mutex
};


#endif