//
// Created by christian on 12/2/25.
//

#ifndef COREOS_TEST_MUTEX_SINGLE_THREAD_H
#define COREOS_TEST_MUTEX_SINGLE_THREAD_H

#include "../mla_test_data_types.h"


// Create a mutex and return its pointer value as an ID
mla_test_uint64_t __mla_test_single_thread_create_mutex() {
    return 100; // Dummy ID for single-threaded environment
}

// Lock the mutex identified by the pointer value
mla_test_bool_t __mla_test_single_thread_lock_mutex(mla_test_uint64_t mutex_id) {

    if (mutex_id != 100)
        return false;

    return true; // Always succeeds in single-threaded environment
}

// Unlock the mutex identified by the pointer value
void __mla_test_single_thread_unlock_mutex(mla_test_uint64_t mutex_id) {
    (void)mutex_id;
}

// Destroy the mutex identified by the pointer value
void __mla_test_single_thread_destroy_mutex(mla_test_uint64_t mutex_id) {
    (void)mutex_id;
}


mla_test_mutex_t g_test_mutex = {
    // Create Mutex
    __mla_test_single_thread_create_mutex,
    // Lock Mutex
    __mla_test_single_thread_lock_mutex,
    // Unlock Mutex
    __mla_test_single_thread_unlock_mutex,
    // Destroy Mutex
    __mla_test_single_thread_destroy_mutex
};

#endif