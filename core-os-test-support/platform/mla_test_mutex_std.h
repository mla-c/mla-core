//
// Created by christian on 12/2/25.
//

#ifndef COREOS_TEST_MUTEX_STD_H
#define COREOS_TEST_MUTEX_STD_H

#include "../mla_test_data_types.h"
#include <mutex>

// Create a mutex and return its pointer value as an ID
mla_test_uint64_t __mla_test_sdt_create_mutex() {
    auto* mtx = new std::mutex();
    return static_cast<mla_test_uint64_t>(reinterpret_cast<uintptr_t>(mtx));
}

// Lock the mutex identified by the pointer value
mla_test_bool_t __mla_test_sdt_lock_mutex(mla_test_uint64_t mutex_id) {
    auto* mtx = reinterpret_cast<std::mutex*>(static_cast<uintptr_t>(mutex_id));
    if (!mtx) return (mla_test_bool_t)false;
    mtx->lock();
    return (mla_test_bool_t)true;
}

// Unlock the mutex identified by the pointer value
void __mla_test_sdt_unlock_mutex(mla_test_uint64_t mutex_id) {
    auto* mtx = reinterpret_cast<std::mutex*>(static_cast<uintptr_t>(mutex_id));
    if (!mtx) return;
    mtx->unlock();
}

// Destroy the mutex identified by the pointer value
void __mla_test_sdt_destroy_mutex(mla_test_uint64_t mutex_id) {
    auto* mtx = reinterpret_cast<std::mutex*>(static_cast<uintptr_t>(mutex_id));
    delete mtx;
}


mla_test_mutex_t g_test_mutex = {
    // Create Mutex
    __mla_test_sdt_create_mutex,
    // Lock Mutex
    __mla_test_sdt_lock_mutex,
    // Unlock Mutex
    __mla_test_sdt_unlock_mutex,
    // Destroy Mutex
    __mla_test_sdt_destroy_mutex
};

#endif