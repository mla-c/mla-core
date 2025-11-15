//
// Created by christian on 9/10/25.
//

#include "mla_rw_lock.h"

mla_rw_lock_t mla_rw_lock_invalid() {
    return {
        mla_mutex_invalid(),
        mla_mutex_invalid(),
        0,
    };
}

mla_rw_lock_t mla_rw_lock(const mla_string_t &name) {
    mla_rw_lock_t lock = {
        mla_mutex(mla_string_concat(name, " writer lock")),
        mla_mutex(mla_string_concat(name, " reader lock")),
        0,
    };
    return lock;
}

mla_bool_t mla_rw_lock_try_read(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line) {
    // check if the is an writer lock active
    if (!mla_mutex_try_lock(lock.writerLock, timeout, source, line)) {
        return false;
    }

    mla_bool_t successfull = false;

    // Lock the readers
    if (mla_mutex_try_lock(lock.readerLock, timeout, source, line)) {
        mla_int32_t *lockCounterRef = const_cast<mla_int32_t *>(&lock.readerCount);
        (*lockCounterRef)++; // Increment the reader count
        mla_mutex_try_unlock(lock.readerLock, source, line);
        successfull = true; // Successfully acquired the read lock
    }

    mla_mutex_try_unlock(lock.writerLock, source, line); // Unlock the mutex after incrementing the reader count

    return successfull;
}

mla_bool_t mla_rw_lock_try_unlock_read(mla_rw_lock_t &lock, mla_int32_t timeout, const char *source, mla_uint32_t line) {

    mla_bool_t successfull = false;

    if (mla_mutex_try_lock(lock.readerLock, timeout, source, line)) {
        mla_int32_t *lockCounterRef = const_cast<mla_int32_t *>(&lock.readerCount);

        if (*lockCounterRef > 0) {
            (*lockCounterRef)--; // Decrement the reader count
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
    if (!mla_mutex_try_lock(lock.writerLock, timeout, source, line)) {
        return false; // Failed to acquire the write lock
    }

    mla_int32_t readCheckInterval = timeout / 10; // Check every 10% of the timeout

    for (mla_uint8_t i = 0; i < 10; ++i) {

        if (mla_mutex_try_lock(lock.readerLock, timeout, source, line)) {

            if (lock.readerCount > 0) {
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