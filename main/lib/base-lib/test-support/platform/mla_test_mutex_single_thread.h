//
// Created by christian on 12/2/25.
//

#ifndef MLA_TEST_MUTEX_SINGLE_THREAD_H
#define MLA_TEST_MUTEX_SINGLE_THREAD_H

#include "../mla_test_data_types.h"

static mla_test_uint32_t g_single_thread_mutex_state = 100;

// Create a mutex and return its pointer value as an ID
mla_test_pointer_t mla_private_mla_test_single_thread_create_mutex() {
    return &g_single_thread_mutex_state; // Dummy ID for single-threaded environment
}

// Lock the mutex identified by the pointer value
mla_test_bool_t mla_private_mla_test_single_thread_lock_mutex(mla_test_pointer_t mutex_id) {

    if (mutex_id == nullptr)
        return false;

    if (*((mla_test_uint32_t*)(mutex_id)) != 100)
        return false;

    return true; // Always succeeds in single-threaded environment
}

// Unlock the mutex identified by the pointer value
void mla_private_mla_test_single_thread_unlock_mutex(mla_test_pointer_t mutex_id) {
    (void)mutex_id;
}

// Destroy the mutex identified by the pointer value
void mla_private_mla_test_single_thread_destroy_mutex(mla_test_pointer_t mutex_id) {
    (void)mutex_id;
}


mla_test_mutex_t g_test_mutex = {
    // Create Mutex
    mla_private_mla_test_single_thread_create_mutex,
    // Lock Mutex
    mla_private_mla_test_single_thread_lock_mutex,
    // Unlock Mutex
    mla_private_mla_test_single_thread_unlock_mutex,
    // Destroy Mutex
    mla_private_mla_test_single_thread_destroy_mutex
};

#endif