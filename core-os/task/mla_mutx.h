//
// Created by christian on 8/9/25.
//

#ifndef COREOS_MLA_MUTX_H
#define COREOS_MLA_MUTX_H

#include "../mla_data_types.h"
#include "../log/mla_logging.h"
#include "mla_task_manager_data_types.h"
#include "../system/mla_string_concat.h"

#define mla_mutex_default_wait_timeout 1000 // Default timeout for mutex lock in milliseconds


struct mla_mutex_t {
    mla_string_t name;
    mla_pointer_t resource; // Pointer to the mutex resource
    mla_buffer_reference_t resourceOwner; // Resource owner for the mutex
};

mla_mutex_t mla_mutex_invalid();
mla_mutex_t mla_mutex(const mla_string_t& name);
mla_mutex_t mla_mutex(const mla_char_t* name, mla_size_t size);
mla_bool_t mla_mutex_try_lock(const mla_mutex_t& mutex, mla_int32_t timeout, mla_bool_t silent, const mla_char_t* source, mla_uint32_t line);
mla_bool_t mla_mutex_try_unlock(const mla_mutex_t& mutex, const mla_char_t* source, mla_uint32_t line);

#define mla_mutex_lock(mutex) mla_mutex_try_lock(mutex, mla_mutex_default_wait_timeout, false, __FILE__, __LINE__)
#define mla_mutex_lock_timeout(mutex, timeout) mla_mutex_try_lock(mutex, timeout, false, __FILE__, __LINE__)
#define mla_mutex_trylock(mutex, timeout) mla_mutex_try_lock(mutex, timeout, true, __FILE__, __LINE__)
#define mla_mutex_unlock(mutex) mla_mutex_try_unlock(mutex, __FILE__, __LINE__)

#define mla_mutex_create(name) mla_mutex(name, sizeof(name))


#endif
