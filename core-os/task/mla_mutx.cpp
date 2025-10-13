//
// Created by christian on 9/10/25.
//

#include "mla_mutx.h"


mla_buffer_cleanup_mode __mla_mutex_cleanup_hook(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData; // Silences the unused parameter warning

    // This function is called when the mutex is destroyed
    mla_pointer_t mutexResource = data;
    mla_bool_t success = g_task_low_level_access.destroy_mutex(mutexResource);

    if (!success) {
        mla_error("Unable to destroy mutex resource");
    }

    return CLEAN_UP_SKIP; // Indicate that the resource needs to be cleaned up
}

mla_mutex_t mla_mutex(mla_string_t name) {

    mla_mutex_t mutex = {
        name,
        nullptr,
        mla_buffer_reference_noOwner(),
    };

    // Create the native mutex
    mla_pointer_t outResource = nullptr;
    mla_bool_t success = g_task_low_level_access.create_mutex(&outResource);

    if (!success) {
        mla_string_t message = mla_string_concat("Failed to create mutex: ", name);
        mla_error(message);
    }

    mutex.resource = outResource; // Assign the resource pointer
    mutex.resourceOwner = mla_buffer_reference(outResource, true, __mla_mutex_cleanup_hook);
    return mutex;
}

mla_mutex_t mla_mutex(const mla_char_t* name, mla_size_t size) {

    return  mla_mutex(mla_string(name, size));
}

mla_bool_t mla_mutex_try_lock(const mla_mutex_t& mutex, mla_int32_t timeout, const char* source, mla_uint32_t line) {

    if (!g_task_low_level_access.lock_mutex(mutex.resource, timeout)) {
        mla_string_t line_number = mla_string_from_uint32(line);
        mla_string_t message = mla_string_concat("Failed to lock mutex: ", mutex.name, " ",  source, ":", line_number);
        mla_error(message);
        return false;
    }
    return true;
}

mla_bool_t mla_mutex_try_unlock(const mla_mutex_t& mutex, const char* source, mla_uint32_t line) {

    if (!g_task_low_level_access.unlock_mutex(mutex.resource)) {
        mla_string_t line_number = mla_string_from_uint32(line);
        mla_string_t message = mla_string_concat("Failed to lock mutex: ", mutex.name, " ",  source, ":", line_number);
        mla_error(message);
        return false;
    }

    return true;
}
