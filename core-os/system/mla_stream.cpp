//
// Created by christian on 9/13/25.
//

#include "mla_stream.h"

struct __mla_stream_buffer_manager {
    mla_byte_t* buffer;
    mla_size_t size;
    mla_size_t position;
};


mla_size_t __mla_stream_noop_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void) output;
    (void) offset;
    (void) buffer;
    return length;
}

mla_size_t __mla_steam_noop_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void) input;
    (void) offset;
    (void) buffer;
    (void) length;
    return 0;
}

mla_stream_output_t mla_stream_noop_output() {
    return {
        0,
        __mla_stream_noop_write,
        nullptr,
        mla_buffer_reference_noOwner()
    };
}

mla_stream_input_t mla_stream_noop_input() {
    return {
        0,
        __mla_steam_noop_read,
        nullptr,
        mla_buffer_reference_noOwner()
    };
}

mla_size_t __mla_stream_input_read_from_buffer(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    __mla_stream_buffer_manager* manager = reinterpret_cast<__mla_stream_buffer_manager*>(input.userdata);

    if (manager->position + length > manager->size) {
        length = manager->size - manager->position;
    }

    if (length == 0) {
        return 0;
    }

    mla_memcpy(buffer + offset, static_cast<mla_byte_t*>(manager->buffer) + manager->position, length);
    manager->position += length;
    return length;
}

mla_size_t __mla_stream_input_buffer_remaining_bytes(const mla_stream_input_t& input) {

    __mla_stream_buffer_manager* manager = reinterpret_cast<__mla_stream_buffer_manager*>(input.userdata);
    return manager->size - manager->position;
}

mla_stream_input_t mla_stream_input_from_buffer(mla_byte_t* buffer, mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager)));

    if (bufferManager == nullptr) {
        return mla_stream_noop_input(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_input_read_from_buffer,
        __mla_stream_input_buffer_remaining_bytes,
        mla_buffer_reference(bufferManager)
    };
}

mla_size_t __mla_stream_output_write_to_buffer(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    __mla_stream_buffer_manager* manager = reinterpret_cast<__mla_stream_buffer_manager*>(output.userdata);

    if (manager->position + length > manager->size) {
        length = manager->size - manager->position;
    }

    if (length == 0) {
        return 0;
    }

    mla_memcpy(static_cast<mla_byte_t*>(manager->buffer) + manager->position, buffer + offset, length);
    manager->position += length;
    return length;
}

mla_size_t __mla_stream_output_buffer_available_bytes(const mla_stream_output_t& output) {

    __mla_stream_buffer_manager* manager = reinterpret_cast<__mla_stream_buffer_manager*>(output.userdata);
    return manager->size - manager->position;
}

mla_stream_output_t mla_stream_output_to_buffer(mla_byte_t* buffer, mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager)));

    if (bufferManager == nullptr) {
        return mla_stream_noop_output(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_output_write_to_buffer,
        __mla_stream_output_buffer_available_bytes,
        mla_buffer_reference(bufferManager)
    };
}

mla_stream_input_t mla_stream_input_from_buffer(mla_size_t size) {

    // Allocate the buffer manager and the buffer in one allocation
    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager) + size));

    if (bufferManager == nullptr) {
        return mla_stream_noop_input(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_input_read_from_buffer,
        __mla_stream_input_buffer_remaining_bytes,
        mla_buffer_reference(bufferManager)
    };

}

mla_stream_output_t mla_stream_output_to_buffer(mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager) + size));

    if (bufferManager == nullptr) {
        return mla_stream_noop_output(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_output_write_to_buffer,
        __mla_stream_output_buffer_available_bytes,
        mla_buffer_reference(bufferManager)
    };

}

mla_size_t __mla_stream_std_input_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    (void)input;
    mla_char_t tempBuffer[mla_stream_fast_read_buffer_size] = {0};

    mla_size_t char_read = mla_std_read(tempBuffer, sizeof(tempBuffer));

    if (char_read == 0) {
        return 0; // No data read
    }

    // Copy to the provided buffer
    mla_size_t toCopy = mla_min(char_read, length);
    mla_memcpy(buffer + offset, tempBuffer, toCopy);
    return toCopy;
}


mla_stream_input_t mla_stream_input_stdin() {

    return {
        0,
        __mla_stream_std_input_read,
        nullptr,
        mla_buffer_reference_noOwner()
    };

}

mla_size_t __mla_stream_std_output_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    (void)output;
    mla_char_t* tempBuffer = reinterpret_cast<mla_char_t*>(mla_malloc(sizeof(mla_char_t) * (length + 1)));

    if (tempBuffer == nullptr) {
        return 0; // Memory allocation failed
    }

    mla_memcpy(tempBuffer, buffer + offset, length);
    tempBuffer[length] = '\0'; // Null-terminate the string
    mla_printf("%s", tempBuffer);

    mla_free(tempBuffer);
    return length;

}

mla_stream_output_t mla_stream_output_stdout() {

    return {
        0,
        __mla_stream_std_output_write,
        nullptr,
        mla_buffer_reference_noOwner()
    };

}

struct mla_memory_stream_buffer_t {
    mla_byte_t* buffer;
    mla_size_t size;
    mla_size_t capacity;
    mla_size_t position;
};

mla_size_t __mla_memory_stream_input_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(input.userdata);
    if (memBuffer->position + length > memBuffer->size) {
        length = memBuffer->size - memBuffer->position;
    }

    // Copy data to the buffer
    mla_memcpy(buffer + offset, memBuffer->buffer + memBuffer->position, length);
    memBuffer->position += length;
    return length;
}

mla_size_t __mla_memory_stream_input_remaining_bytes(const mla_stream_input_t& input) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(input.userdata);
    return memBuffer->size - memBuffer->position;
}

mla_size_t __mla_memory_stream_output_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(output.userdata);

    if (memBuffer->position + length > memBuffer->capacity) {
        // Need to resize the buffer
        mla_size_t newSize = mla_max(mla_max(memBuffer->capacity * 2, memBuffer->position + length), mla_stream_fast_read_buffer_size);
        // We can not use realloc so we need to use malloc and memcpy
        mla_byte_t* newBuffer = reinterpret_cast<mla_byte_t*>(mla_malloc(newSize));
        if (newBuffer != nullptr) {

            if (memBuffer->buffer != nullptr) {
                mla_memcpy(newBuffer, memBuffer->buffer, memBuffer->size);
                mla_free(memBuffer->buffer);
                // Zero the new allocated part
                if (newSize > memBuffer->size) {
                    mla_memset(newBuffer + memBuffer->size, 0, newSize - memBuffer->size);
                }
            } else {
                mla_memset(newBuffer, 0, newSize);
            }

        }

        if (newBuffer == nullptr) {
            memBuffer->buffer = nullptr;
            memBuffer->capacity = 0;
            memBuffer->position = 0;
            memBuffer->size = 0;
            return 0; // Reallocation failed
        } else {
            memBuffer->buffer = newBuffer;
            memBuffer->capacity = newSize;
        }
    }

    // Copy the data
    mla_memcpy(memBuffer->buffer + memBuffer->position, buffer + offset, length);
    memBuffer->position += length;
    if (memBuffer->position > memBuffer->size) {
        memBuffer->size = memBuffer->position;
    }
    return length;

}

mla_size_t __mla_memory_stream_output_available_bytes(const mla_stream_output_t& output) {
    (void)output;
    // The only limit is the heap
    return mla_size_max;
}

mla_memory_stream_t mla_memory_stream_invalid() {
    return {
        mla_stream_noop_input(),
        mla_stream_noop_output()
    };
}

mla_memory_stream_t mla_memory_stream_empty() {
    return mla_memory_stream(0);
}

mla_buffer_cleanup_mode __mla_memory_stream_cleanup_hook(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData;
    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(data);

    if (memBuffer == nullptr) {
        return CLEAN_UP_SKIP;
    }

    if (memBuffer->buffer != nullptr) {
        mla_free(memBuffer->buffer);
    }

    return CLEAN_UP_NEEDED;
}

mla_memory_stream_t mla_memory_stream(mla_size_t initial_size) {

    mla_memory_stream_buffer_t* memBuffer = static_cast<mla_memory_stream_buffer_t*>(mla_malloc(sizeof(mla_memory_stream_buffer_t)));

    if (memBuffer == nullptr) {
        return mla_memory_stream_invalid();
    }

    mla_memset(memBuffer, 0, sizeof(mla_memory_stream_buffer_t));

    if (initial_size > 0) {

        memBuffer->buffer = static_cast<mla_byte_t*>(mla_malloc(initial_size));

        if (memBuffer->buffer == nullptr) {
            mla_free(memBuffer);
            return mla_memory_stream_invalid();
        }

        mla_memset(memBuffer->buffer, 0, initial_size);
    }

    memBuffer->size = 0;
    memBuffer->capacity = initial_size;
    memBuffer->position = 0;

    mla_buffer_reference_t bufferOwner = mla_buffer_reference(memBuffer, false, __mla_memory_stream_cleanup_hook);

    return {
        {
            reinterpret_cast<mla_callback_userdata>(memBuffer),
            __mla_memory_stream_input_read,
            __mla_memory_stream_input_remaining_bytes,
            bufferOwner
        },
        {
            reinterpret_cast<mla_callback_userdata>(memBuffer),
            __mla_memory_stream_output_write,
            __mla_memory_stream_output_available_bytes,
            bufferOwner
        }
    };
}

mla_size_t mla_memory_stream_get_size(const mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(memoryStream.input.userdata);

    if (memBuffer == nullptr) {
        return 0;
    }

    return memBuffer->size;
}

mla_size_t mla_memory_stream_get_position(const mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(memoryStream.input.userdata);

    if (memBuffer == nullptr) {
        return 0;
    }

    return memBuffer->position;
}

mla_bool_t mla_memory_stream_set_position(mla_memory_stream_t &memoryStream, mla_size_t position) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(memoryStream.input.userdata);

    if (memBuffer == nullptr) {
        return false;
    }

    if (position > memBuffer->size) {
        return false; // Position out of bounds
    }

    memBuffer->position = position;
    return true;
}

void mla_memory_stream_reset(mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(memoryStream.input.userdata);

    if (memBuffer == nullptr) {
        return;
    }

    memBuffer->position = 0;
    memBuffer->size = 0;

    if (memBuffer->buffer == nullptr) {
        return;
    }

    mla_memset(memBuffer->buffer, 0, memBuffer->size);

}
