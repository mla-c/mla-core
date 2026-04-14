//
// Created by christian on 9/10/25.
//

#include "mla_rw_lock.h"

mla_rw_lock_t mla_rw_lock_invalid() {
    return {
        mla_mutex_invalid(),
        mla_mutex_invalid(),
        nullptr,
        mla_buffer_reference_noOwner()
    };
}

mla_rw_lock_t mla_rw_lock(const mla_string_t &name, mla_bool_t support_recursive) {

    mla_rw_lock_state_t* state_info = static_cast<mla_rw_lock_state_t*>(mla_platform_malloc(sizeof(mla_rw_lock_state_t)));

    if (state_info == nullptr)
        return mla_rw_lock_invalid();

    mla_memset(state_info, 0, sizeof(mla_rw_lock_state_t));

    mla_rw_lock_t lock = {
        mla_mutex(mla_string_concat(name, " writer lock"), support_recursive),
        mla_mutex(mla_string_concat(name, " reader lock"), support_recursive),
        state_info,
        mla_buffer_reference(state_info)
    };
    return lock;
}

mla_bool_t mla_rw_lock_try_read(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line) {
    // check if the is an writer lock active
    if (!mla_mutex_try_lock(lock.writerLock, timeout, false, source, line)) {
        return false;
    }

    mla_bool_t successfull = false;

    // Lock the readers
    if (mla_mutex_try_lock(lock.readerLock, timeout, false, source, line)) {

        if (lock.state == nullptr) {
            mla_mutex_try_unlock(lock.readerLock, source, line);
            mla_mutex_try_unlock(lock.writerLock, source, line); // Unlock the mutex after incrementing the reader count
            return false;
        }

        lock.state->readerCount++;
        mla_mutex_try_unlock(lock.readerLock, source, line);
        successfull = true; // Successfully acquired the read lock
    }

    mla_mutex_try_unlock(lock.writerLock, source, line); // Unlock the mutex after incrementing the reader count

    return successfull;
}

mla_bool_t mla_rw_lock_try_unlock_read(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line) {

    mla_bool_t successfull = false;

    if (mla_mutex_try_lock(lock.readerLock, timeout, false, source, line)) {

        if (lock.state == nullptr) {
            mla_mutex_try_unlock(lock.readerLock, source, line); // Unlock the reader lock
            // Was never locked
            return true;
        }

        if (lock.state->readerCount > 0) {
            lock.state->readerCount--;
            successfull = true; // Successfully unlocked the read lock
        } else {
            mla_error(mla_string_concat("Try to unlock reader by no read active ", lock.readerLock.name));
        }

        mla_mutex_try_unlock(lock.readerLock, source, line); // Unlock the reader lock
    }

    return successfull;
}

mla_bool_t mla_rw_lock_try_write(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line) {
    // Try to lock the writer lock
    if (!mla_mutex_try_lock(lock.writerLock, timeout, false, source, line)) {
        return false; // Failed to acquire the write lock
    }

    mla_int32_t readCheckInterval = timeout / 10; // Check every 10% of the timeout

    for (mla_uint8_t i = 0; i < 10; ++i) {

        if (mla_mutex_try_lock(lock.readerLock, timeout, false, source, line)) {

            if (lock.state == nullptr) {
                mla_mutex_try_unlock(lock.readerLock, source, line); // Unlock the reader lock
                mla_mutex_try_unlock(lock.writerLock, source, line); // Unlock the reader lock
                // Was never locked
                return false;
            }

            if (lock.state->readerCount > 0) {
                // If there are active readers, we need to wait
                mla_mutex_try_unlock(lock.readerLock, source, line); // Unlock the reader lock

                if (i < 3) {
                    mla_sleep(0); // Yield for a short period to allow readers to finish
                } else {
                    mla_sleep(readCheckInterval); // Wait for a short period before checking again
                }

            } else {
                // We are the only and only reader so we keep both locks
                return true;
            }

        }

    }

    mla_mutex_try_unlock(lock.writerLock, source, line); // Unlock the reader lock

    return false;

}

mla_bool_t mla_rw_lock_try_unlock_write(mla_rw_lock_t &lock, const char *source, mla_uint32_t line) {

    mla_mutex_try_unlock(lock.readerLock, source, line);
    mla_mutex_try_unlock(lock.writerLock, source, line);
    return true;
}