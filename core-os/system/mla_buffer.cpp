//
// Created by christian on 9/10/25.
//

#include "mla_buffer.h"

#include "mla_user_data.h"
#include "../task/mla_atomic.h"


mla_buffer_t* mla_buffer(const mla_platform_pointer_t p_Data, mla_buffer_cleanup_hook_t p_CleanupHook, const mla_dynamic_data_t& cleanupHookUserData) {

    if (!p_Data) {
        return nullptr; // Return null if data is null
    }

    mla_buffer_t* buffer = static_cast<mla_buffer_t*>(mla_malloc(sizeof(mla_buffer_t)));

    if (buffer == nullptr) {
        return nullptr; // Return null if allocation fails
    }

    buffer->refCount = mla_atomic_int32(0); // Initialize reference count to 0
    buffer->data = p_Data;
    buffer->cleanupHook = p_CleanupHook; // Set the cleanup hook
    buffer->cleanupHookUserData = cleanupHookUserData; // Set the user data for the cleanup hook
    return buffer;
}

void __mla_buffer_destroy(mla_buffer_t* p_Buffer, mla_bool_t executeCleanupHook) {
    if (p_Buffer) {
        if (mla_atomic_decrement(p_Buffer->refCount) == 0) {
            mla_platform_pointer_t l_Data = const_cast<mla_platform_pointer_t>(p_Buffer->data);

            if (l_Data) {

                mla_buffer_cleanup_mode needCleanup = CLEAN_UP_NEEDED;

                // Call the cleanup hook if it is set
                if (executeCleanupHook && p_Buffer->cleanupHook) {
                    needCleanup = p_Buffer->cleanupHook(l_Data, p_Buffer->cleanupHookUserData);
                }

                // Free the data only if it was allocated
                if (needCleanup == CLEAN_UP_NEEDED) {
                    mla_free(l_Data);
                }
            }
            p_Buffer->data = nullptr; // Clear the pointer
            mla_free(p_Buffer);
        }
    }
}

void mla_buffer_destroy_without_cleanup_unsafe(mla_buffer_t* p_Buffer) {
    __mla_buffer_destroy(p_Buffer, false);
}

void mla_buffer_destroy(mla_buffer_t* p_Buffer) {
    __mla_buffer_destroy(p_Buffer, true);
}


mla_buffer_reference_t::mla_buffer_reference_t(const mla_buffer_reference_t& p_Other) : buffer(p_Other.buffer) {
    if (buffer) {
        mla_atomic_increment(buffer->refCount);
    }
}

mla_buffer_reference_t::mla_buffer_reference_t(mla_buffer_t* p_Buffer) : buffer(p_Buffer) {
    if (buffer) {
        mla_atomic_increment(buffer->refCount);
    }
}

mla_buffer_reference_t::~mla_buffer_reference_t() {
    mla_buffer_destroy(buffer);
    buffer = nullptr;

}

mla_buffer_reference_t& mla_buffer_reference_t::operator=(const mla_buffer_reference_t& p_Other) {

    if (this != &p_Other) {
        // Destroy current buffer

        // Because by destroying the current buffer is can happend
        // that the p_Other is Zeroed out.
        // This happends if there are cyclic references. link in a linked list.
        // So we need to backup the buffer pointer before destroying it.
        // It will not destroy the buffer of p_Other, because it is a reference.
        // Only the pointer for holding the reference is destroyed.
        mla_buffer_t* backupBuffer = p_Other.buffer;

        mla_buffer_destroy(buffer); // Destroy current buffer

        // Assign new buffer
        buffer = backupBuffer; // Assign new buffer

        // Increment reference count of the new buffer
        if (buffer) {
            mla_atomic_increment(buffer->refCount);
        }
    }
    return *this;
}

mla_buffer_reference_t mla_buffer_reference_without_cleanup_hook(const mla_platform_pointer_t data) {
    mla_dynamic_data_t empty_user_data = mla_dynamic_data_empty();
    return mla_buffer_reference_create(data, false, nullptr, empty_user_data);
}


mla_buffer_reference_t mla_buffer_reference_create(const mla_platform_pointer_t data, mla_bool_t mangedExternalResource, mla_buffer_cleanup_hook_t cleanupHook, const mla_dynamic_data_t& cleanupHookUserData) {

    if (data != nullptr && !mangedExternalResource && mla_is_gcc_pointer(data)) {
        return mla_buffer_reference_noOwner();
    }

    mla_buffer_t* buffer = mla_buffer(data, cleanupHook, cleanupHookUserData);
    return mla_buffer_reference_t(buffer);
}

mla_buffer_reference_t mla_buffer_reference_noOwner() {
    return mla_buffer_reference_t(nullptr);
}

void mla_buffer_reference_destroy(mla_buffer_reference_t& p_Reference) {

    if (!p_Reference.buffer) {
        return; // Nothing to destroy
    }

    mla_buffer_destroy(p_Reference.buffer);
    p_Reference.buffer = nullptr;
}


void mla_buffer_reference_destroy_without_cleanup_unsafe(mla_buffer_reference_t& p_Reference) {

    if (!p_Reference.buffer) {
        return; // Nothing to destroy
    }

    mla_buffer_destroy_without_cleanup_unsafe(p_Reference.buffer);
    p_Reference.buffer = nullptr;
}

mla_bool_t mla_buffer_reference_is_noOwner(const mla_buffer_reference_t& p_Reference) {
    return p_Reference.buffer == nullptr;
}
