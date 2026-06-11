//
// Created by christian on 12/2/25.
//

#ifndef MLA_TEST_MUTEX_STD_H
#define MLA_TEST_MUTEX_STD_H

#include "../mla_test_data_types.h"
#include <mutex>

// Create a mutex and return its pointer value as an ID
mla_test_pointer_t mla_internal_sdt_create_mutex() {
    auto* mtx = new std::mutex();
    return mtx;
}

// Lock the mutex identified by the pointer value
mla_test_bool_t mla_internal_sdt_lock_mutex(mla_test_pointer_t mutex_id) {

    if (mutex_id == nullptr)
        return false;

    std::mutex* mtx = reinterpret_cast<std::mutex*>(mutex_id);
    mtx->lock();
    return true;
}

// Unlock the mutex identified by the pointer value
void mla_internal_sdt_unlock_mutex(mla_test_pointer_t mutex_id) {
    if (mutex_id == nullptr)
        return;

    std::mutex* mtx = reinterpret_cast<std::mutex*>(mutex_id);
    mtx->unlock();
}

// Destroy the mutex identified by the pointer value
void mla_internal_sdt_destroy_mutex(mla_test_pointer_t mutex_id) {

    if (mutex_id == nullptr)
        return;

    std::mutex* mtx = reinterpret_cast<std::mutex*>(mutex_id);
    delete mtx;
}


mla_test_mutex_t g_test_mutex = {
    // Create Mutex
    mla_internal_sdt_create_mutex,
    // Lock Mutex
    mla_internal_sdt_lock_mutex,
    // Unlock Mutex
    mla_internal_sdt_unlock_mutex,
    // Destroy Mutex
    mla_internal_sdt_destroy_mutex
};

#endif