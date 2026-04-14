//
// Created by christian on 9/13/25.
//

#include "mla_stream.h"

struct __mla_stream_buffer_manager {
    mla_byte_t* buffer;
    mla_size_t size;
    mla_size_t position;
};

struct __mla_stream_buffer_manager_initializer {

    static __mla_stream_buffer_manager init() {
        return {
        nullptr,
            0,
            0
        };
    }

};


mla_size_t __mla_stream_noop_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void) output;
    (void) offset;
    (void) buffer;
    return length;
}

mla_size_t __mla_steam_noop_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void) input;
    (void) offset;
    (void) buffer;
    (void) length;
    return 0;
}

mla_stream_output_t mla_stream_noop_output() {
    return {
        mla_user_data_empty(),
        __mla_stream_noop_write,
        nullptr
    };
}

mla_stream_input_t mla_stream_noop_input() {
    return {
        mla_user_data_empty(),
        __mla_steam_noop_read,
        nullptr
    };
}

mla_user_data_id_init(mla_stream_buffer_manager_userdata_name)

mla_size_t __mla_stream_input_read_from_buffer(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    __mla_stream_buffer_manager* manager = mla_user_data_get_pointer<__mla_stream_buffer_manager>(input.userdata, mla_stream_buffer_manager_userdata_name);

    if (manager == nullptr) {
        return 0; // No manager, no data
    }

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

mla_size_t __mla_stream_input_buffer_remaining_bytes(mla_stream_input_t& input) {

    __mla_stream_buffer_manager* manager = mla_user_data_get_pointer<__mla_stream_buffer_manager>(input.userdata, mla_stream_buffer_manager_userdata_name);

    if (manager == nullptr) {
        return 0; // No manager, no data
    }

    return manager->size - manager->position;
}

mla_stream_input_t mla_stream_input_from_buffer(mla_byte_t* buffer, mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_platform_malloc(sizeof(__mla_stream_buffer_manager)));

    if (bufferManager == nullptr) {
        return mla_stream_noop_input(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<__mla_stream_buffer_manager, __mla_stream_buffer_manager_initializer>(user_data, mla_stream_buffer_manager_userdata_name, bufferManager);

    return {
        user_data,
        __mla_stream_input_read_from_buffer,
        __mla_stream_input_buffer_remaining_bytes
    };
}

mla_size_t __mla_stream_output_write_to_buffer(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    __mla_stream_buffer_manager* manager = mla_user_data_get_pointer<__mla_stream_buffer_manager>(output.userdata, mla_stream_buffer_manager_userdata_name);

    if (manager == nullptr) {
        return 0; // No manager, cannot write
    }

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

mla_size_t __mla_stream_output_buffer_available_bytes(mla_stream_output_t& output) {

    __mla_stream_buffer_manager* manager = mla_user_data_get_pointer<__mla_stream_buffer_manager>(output.userdata, mla_stream_buffer_manager_userdata_name);

    if (manager == nullptr) {
        return 0; // No manager, no space
    }

    return manager->size - manager->position;
}

mla_stream_output_t mla_stream_output_to_buffer(mla_byte_t* buffer, mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_platform_malloc(sizeof(__mla_stream_buffer_manager)));

    if (bufferManager == nullptr) {
        return mla_stream_noop_output(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<__mla_stream_buffer_manager, __mla_stream_buffer_manager_initializer>(user_data, mla_stream_buffer_manager_userdata_name, bufferManager);

    return {
        user_data,
        __mla_stream_output_write_to_buffer,
        __mla_stream_output_buffer_available_bytes
    };
}

mla_stream_input_t mla_stream_input_from_buffer(mla_size_t size) {

    // Allocate the buffer manager and the buffer in one allocation
    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_platform_malloc(sizeof(__mla_stream_buffer_manager) + size));

    if (bufferManager == nullptr) {
        return mla_stream_noop_input(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<__mla_stream_buffer_manager, __mla_stream_buffer_manager_initializer>(user_data, mla_stream_buffer_manager_userdata_name, bufferManager);

    return {
        user_data,
        __mla_stream_input_read_from_buffer,
        __mla_stream_input_buffer_remaining_bytes
    };

}

mla_stream_output_t mla_stream_output_to_buffer(mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_platform_malloc(sizeof(__mla_stream_buffer_manager) + size));

    if (bufferManager == nullptr) {
        return mla_stream_noop_output(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<__mla_stream_buffer_manager, __mla_stream_buffer_manager_initializer>(user_data, mla_stream_buffer_manager_userdata_name, bufferManager);

    return {
        user_data,
        __mla_stream_output_write_to_buffer,
        __mla_stream_output_buffer_available_bytes
    };

}

mla_size_t __mla_stream_std_input_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    (void)input;
    mla_char_t tempBuffer[mla_global_config_stream_fast_read_buffer_size] = {0};

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
        mla_user_data_empty(),
        __mla_stream_std_input_read,
        nullptr
    };

}

mla_size_t __mla_stream_std_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    (void)output;

    const mla_char_t* tempBuffer = reinterpret_cast<const mla_char_t*>(buffer);
    return mla_print(tempBuffer + offset, length);
}

mla_stream_output_t mla_stream_output_stdout() {

    return {
        mla_user_data_empty(),
        __mla_stream_std_output_write,
        nullptr
    };

}

struct mla_memory_stream_buffer_t {
    mla_byte_t* buffer;
    mla_size_t size;
    mla_size_t capacity;
    mla_size_t position;
    mla_bool_t can_grow;
};

struct mla_memory_stream_buffer_initializer {

    static mla_memory_stream_buffer_t init() {
        return {
        nullptr,
            0,
            0,
            0,
            false
        };
    }

};

mla_user_data_id_init(mla_memory_stream_buffer_userdata_name)

mla_size_t __mla_memory_stream_input_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0; // No buffer, no data
    }

    if (memBuffer->position + length > memBuffer->size) {
        length = memBuffer->size - memBuffer->position;
    }

    // Copy data to the buffer
    mla_memcpy(buffer + offset, memBuffer->buffer + memBuffer->position, length);
    memBuffer->position += length;
    return length;
}

mla_size_t __mla_memory_stream_input_remaining_bytes(mla_stream_input_t& input) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0; // No buffer, no data
    }

    return memBuffer->size - memBuffer->position;
}

mla_size_t __mla_memory_stream_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(output.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0; // No buffer, cannot write
    }

    if (memBuffer->position + length > memBuffer->capacity) {

        if (memBuffer->can_grow) {

            // Need to resize the buffer
            mla_size_t newSize = mla_max(mla_max(memBuffer->capacity * 2, memBuffer->position + length), mla_global_config_stream_fast_read_buffer_size);
            // We can not use realloc so we need to use malloc and memcpy
            mla_byte_t* newBuffer = reinterpret_cast<mla_byte_t*>(mla_platform_malloc(newSize));
            if (newBuffer != nullptr) {

                if (memBuffer->buffer != nullptr) {
                    mla_memcpy(newBuffer, memBuffer->buffer, memBuffer->size);
                    mla_platform_free(memBuffer->buffer);
                    // Zero the new allocated part
                    if (newSize > memBuffer->size) {
                        mla_memset(newBuffer + memBuffer->size, 0, newSize - memBuffer->size);
                    }
                } else {
                    mla_memset(newBuffer, 0, newSize);
                }

                memBuffer->buffer = newBuffer;
                memBuffer->capacity = newSize;

            } else {

                if (memBuffer->buffer != nullptr) {
                    mla_platform_free(memBuffer->buffer);
                }

                memBuffer->buffer = nullptr;
                memBuffer->capacity = 0;
                memBuffer->position = 0;
                memBuffer->size = 0;
                return 0; // Reallocation failed

            }

        } else {
            // Cannot grow, limit the length
            length = memBuffer->capacity - memBuffer->position;
            if (length == 0) {
                return 0; // No space available
            }
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

mla_size_t __mla_memory_stream_output_available_bytes(mla_stream_output_t& output) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(output.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0;
    }

    if (!memBuffer->can_grow) {
        return memBuffer->capacity - memBuffer->position;
    }

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
    return mla_memory_stream(0, true);
}

mla_buffer_cleanup_mode __mla_memory_stream_cleanup_hook(mla_platform_pointer_t data, const mla_dynamic_data_t& userData) {

    (void)userData;
    mla_memory_stream_buffer_t* memBuffer = reinterpret_cast<mla_memory_stream_buffer_t*>(data);

    if (memBuffer == nullptr) {
        return CLEAN_UP_SKIP;
    }

    if (memBuffer->buffer != nullptr) {
        mla_platform_free(memBuffer->buffer);
    }

    return CLEAN_UP_NEEDED;
}

mla_memory_stream_t mla_memory_stream(mla_size_t initial_size) {
    return mla_memory_stream(initial_size, true);
}

mla_memory_stream_t mla_memory_stream(mla_size_t initial_size, mla_bool_t can_grow) {

    mla_memory_stream_buffer_t* memBuffer = static_cast<mla_memory_stream_buffer_t*>(mla_platform_malloc(sizeof(mla_memory_stream_buffer_t)));

    if (memBuffer == nullptr) {
        return mla_memory_stream_invalid();
    }

    mla_memset(memBuffer, 0, sizeof(mla_memory_stream_buffer_t));

    if (initial_size > 0) {

        memBuffer->buffer = static_cast<mla_byte_t*>(mla_platform_malloc(initial_size));

        if (memBuffer->buffer == nullptr) {
            mla_platform_free(memBuffer);
            return mla_memory_stream_invalid();
        }

        mla_memset(memBuffer->buffer, 0, initial_size);
    }

    memBuffer->can_grow = can_grow;
    memBuffer->size = 0;
    memBuffer->capacity = initial_size;
    memBuffer->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership_ex(user_data, mla_memory_stream_buffer_userdata_name, memBuffer, __mla_memory_stream_cleanup_hook, false);

    return {
        {
            user_data,
            __mla_memory_stream_input_read,
            __mla_memory_stream_input_remaining_bytes,
        },
        {
            user_data,
            __mla_memory_stream_output_write,
            __mla_memory_stream_output_available_bytes,
        }
    };
}

mla_size_t mla_memory_stream_get_size(const mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0;
    }

    return memBuffer->size;
}

mla_size_t mla_memory_stream_get_position(const mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0;
    }

    return memBuffer->position;
}

mla_bool_t mla_memory_stream_set_position(mla_memory_stream_t &memoryStream, mla_size_t position) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

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

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return;
    }

    mla_size_t oldSize = memBuffer->size;
    memBuffer->size = 0;
    memBuffer->position = 0;

    if (memBuffer->buffer == nullptr) {
        return;
    }

    mla_memset(memBuffer->buffer, 0, oldSize);

}

mla_user_data_id_init(mla_stream_output_size_calculation_user_data_name)

mla_size_t __mla_stream_output_size_calculation_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)output;
    (void)buffer;
    (void)offset;

    mla_user_data_inc_uint32(output.userdata, mla_stream_output_size_calculation_user_data_name, length);
    return length;
}

mla_stream_output_t mla_stream_output_size_calculation() {

    mla_user_data_t data = mla_user_data_empty();
    mla_user_data_set_uint32(data, mla_stream_output_size_calculation_user_data_name, 0);

    return {
        data,
        __mla_stream_output_size_calculation_write,
        nullptr
    };

}

mla_size_t mla_stream_output_size_calculation_get_size(const mla_stream_output_t &output) {
    return mla_user_data_get_uint32(output.userdata, mla_stream_output_size_calculation_user_data_name, 0);
}
