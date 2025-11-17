//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_RW_LOCK_H
#define COREOS_MLA_RW_LOCK_H

#include "mla_mutx.h"

struct mla_rw_lock_state_t {
    mla_int32_t readerCount; // Count of active readers
};

struct mla_rw_lock_t {
    mla_mutex_t writerLock; // Mutex for writer access
    mla_mutex_t readerLock; // Mutex for reader access
    mla_rw_lock_state_t* state; // Pointer to the lock state
    mla_buffer_reference_t stateOwner;
};

mla_rw_lock_t mla_rw_lock_invalid();
mla_rw_lock_t mla_rw_lock(const mla_string_t &name);
mla_bool_t mla_rw_lock_try_read(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line);
mla_bool_t mla_rw_lock_try_unlock_read(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line);
mla_bool_t mla_rw_lock_try_write(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line);
mla_bool_t mla_rw_lock_try_unlock_write(mla_rw_lock_t &lock, const char *source, mla_uint32_t line);

#define mla_rw_lock_default_timeout 1000 // Default timeout for read/write locks in milliseconds

#define mla_rw_lock_read(lock) mla_rw_lock_try_read(lock, mla_rw_lock_default_timeout, __FILE__, __LINE__)
#define mla_rw_lock_read_timout(lock, timeout) mla_rw_lock_try_read(lock, timeout, __FILE__, __LINE__)

#define mla_rw_lock_write(lock) mla_rw_lock_try_write(lock, mla_rw_lock_default_timeout, __FILE__, __LINE__)
#define mla_rw_lock_write_timout(lock, timeout) mla_rw_lock_try_write(lock, timeout, __FILE__, __LINE__)

#define mla_rw_unlock_read(lock) mla_rw_lock_try_unlock_read(lock, mla_rw_lock_default_timeout, __FILE__, __LINE__)
#define mla_rw_unlock_read_timout(lock, timeout) mla_rw_lock_try_unlock_read(lock, timeout, __FILE__, __LINE__)

#define mla_rw_unlock_write(lock) mla_rw_lock_try_unlock_write(lock, __FILE__, __LINE__)

#define mla_rw_lock_create(name) mla_rw_lock(mla_string_const(name))

#endif
