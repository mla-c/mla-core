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

typedef mla_buffer_cleanup_mode(*mla_buffer_cleanup_hook_t)(mla_platform_pointer_t data, const mla_dynamic_data_t& userData);

struct mla_buffer_t {
    mla_atomic_int32_t refCount;
    const mla_platform_pointer_t data; // Pointer to the buffer data
    mla_buffer_cleanup_hook_t cleanupHook;
    mla_dynamic_data_t cleanupHookUserData;
};

mla_buffer_t* mla_buffer(const mla_platform_pointer_t p_Data, mla_buffer_cleanup_hook_t p_CleanupHook, const mla_dynamic_data_t& cleanupHookUserData);

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


mla_buffer_reference_t mla_buffer_reference_create(const mla_platform_pointer_t data, mla_bool_t mangedExternalResource, mla_buffer_cleanup_hook_t cleanupHook, const mla_dynamic_data_t& cleanupHookUserData);

mla_buffer_reference_t mla_buffer_reference_without_cleanup_hook(const mla_platform_pointer_t data);
mla_buffer_reference_t mla_buffer_reference_noOwner();


template <typename T, typename TInit = mla_default_init(T)>
mla_buffer_cleanup_mode mla_buffer_default_cleanup(mla_platform_pointer_t data, const mla_dynamic_data_t& userData) {
    (void)userData;

    T* l_Data = reinterpret_cast<T*>(data);

    if (l_Data == nullptr) {
        return CLEAN_UP_SKIP; // No data to clean up
    }

    *l_Data = TInit::init();
    return CLEAN_UP_NEEDED;
}

template <typename T, typename TInit = mla_default_init(T)>
mla_buffer_reference_t mla_buffer_reference(const T* data) {
    mla_buffer_cleanup_hook_t cleanupHook = mla_buffer_default_cleanup<T, TInit>;
    return mla_buffer_reference_create(data, false, cleanupHook,  mla_dynamic_data_empty());
}



mla_bool_t mla_buffer_reference_is_noOwner(const mla_buffer_reference_t& p_Reference);

void mla_buffer_reference_destroy(mla_buffer_reference_t& p_Reference);

void mla_buffer_reference_destroy_without_cleanup_unsafe(mla_buffer_reference_t& p_Reference);

#endif