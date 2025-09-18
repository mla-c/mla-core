//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_BUFFER_H
#define COREOS_MLA_BUFFER_H

#include "../mla_data_types.h"

enum mla_buffer_cleanup_mode {
    CLEAN_UP_SKIP,
    CLEAN_UP_NEEDED,
};

typedef mla_buffer_cleanup_mode(*mla_buffer_cleanup_hook_t)(mla_pointer_t data, mla_callback_userdata userData);

struct mla_buffer_t {
    mla_size_t refCount;
    const mla_pointer_t data; // Pointer to the buffer data
    mla_buffer_cleanup_hook_t cleanupHook;
    mla_callback_userdata cleanupHookUserData;
};

mla_buffer_t* mla_buffer(const mla_pointer_t p_Data, mla_buffer_cleanup_hook_t p_CleanupHook = nullptr, mla_callback_userdata cleanupHookUserData = 0);

void __mla_buffer_destroy(mla_buffer_t* p_Buffer, mla_bool_t executeCleanupHook);

void mla_buffer_destroy_without_cleanup_unsafe(mla_buffer_t* p_Buffer);

void mla_buffer_destroy(mla_buffer_t* p_Buffer);

// At this point we are using c++ reference counting to manage the buffer's lifetime.
// The buffer will be destroyed when the reference count reaches zero.


class mla_buffer_reference_t {
public:

    // Copy constructor
    mla_buffer_reference_t(const mla_buffer_reference_t& p_Other);

    // Constructor that takes a buffer pointer
    mla_buffer_reference_t(mla_buffer_t* p_Buffer);

    // Default constructor
    ~mla_buffer_reference_t();
    // Assignment operator
    mla_buffer_reference_t& operator=(const mla_buffer_reference_t& p_Other);
public:
    mla_buffer_t* buffer;  // Pointer to the buffer
};

mla_buffer_reference_t mla_buffer_reference(const mla_pointer_t data, mla_bool_t mangedExternalResource = false, mla_buffer_cleanup_hook_t cleanupHook = nullptr, mla_callback_userdata cleanupHookUserData = 0);

mla_buffer_reference_t mla_buffer_reference_noOwner();

void mla_buffer_reference_destroy(mla_buffer_reference_t& p_Reference);

void mla_buffer_reference_destroy_without_cleanup_unsafe(mla_buffer_reference_t& p_Reference);

#endif