//
// Created by christian on 9/13/25.
//

#include "mla_stream.h"

struct mla_internal_stream_buffer_manager {
    mla_byte_t* buffer;
    mla_size_t size;
    mla_size_t position;

    static mla_internal_stream_buffer_manager init() {
        return {
            nullptr,
                0,
                0
            };
    }

};

mla_size_t mla_internal_stream_noop_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void) output;
    (void) offset;
    (void) buffer;
    return length;
}

mla_size_t mla_internal_stream_noop_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void) input;
    (void) offset;
    (void) buffer;
    (void) length;
    return 0;
}

mla_stream_output_t mla_stream_noop_output() {
    return {
        mla_user_data_empty(),
        mla_internal_stream_noop_write,
        nullptr
    };
}

mla_stream_input_t mla_stream_noop_input() {
    return {
        mla_user_data_empty(),
        mla_internal_stream_noop_read,
        nullptr
    };
}

mla_user_data_id_init(mla_stream_buffer_manager_userdata_name)

mla_size_t mla_internal_stream_input_read_from_buffer(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_internal_stream_buffer_manager* manager = mla_user_data_get_pointer_data<mla_internal_stream_buffer_manager>(input.userdata, mla_stream_buffer_manager_userdata_name);

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

mla_size_t mla_internal_stream_input_buffer_remaining_bytes(mla_stream_input_t& input) {

    mla_internal_stream_buffer_manager* manager = mla_user_data_get_pointer_data<mla_internal_stream_buffer_manager>(input.userdata, mla_stream_buffer_manager_userdata_name);

    if (manager == nullptr) {
        return 0; // No manager, no data
    }

    return manager->size - manager->position;
}

mla_stream_input_t mla_stream_input_from_buffer(mla_byte_t* buffer, mla_size_t size) {

    mla_pointer_t bufferManager_ptr = mla_malloc_struct(mla_internal_stream_buffer_manager);

    mla_internal_stream_buffer_manager* bufferManager = mla_pointer_get_data<mla_internal_stream_buffer_manager>(bufferManager_ptr);

    if (bufferManager == nullptr) {
        return mla_stream_noop_input(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_stream_buffer_manager_userdata_name, bufferManager_ptr);

    return {
        user_data,
        mla_internal_stream_input_read_from_buffer,
        mla_internal_stream_input_buffer_remaining_bytes
    };
}

mla_size_t mla_internal_stream_output_write_to_buffer(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_internal_stream_buffer_manager* manager = mla_user_data_get_pointer_data<mla_internal_stream_buffer_manager>(output.userdata, mla_stream_buffer_manager_userdata_name);

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

mla_size_t mla_internal_stream_output_buffer_available_bytes(mla_stream_output_t& output) {

    mla_internal_stream_buffer_manager* manager = mla_user_data_get_pointer_data<mla_internal_stream_buffer_manager>(output.userdata, mla_stream_buffer_manager_userdata_name);

    if (manager == nullptr) {
        return 0; // No manager, no space
    }

    return manager->size - manager->position;
}

mla_stream_output_t mla_stream_output_to_buffer(mla_byte_t* buffer, mla_size_t size) {

    mla_pointer_t bufferManager_ptr = mla_malloc_struct(mla_internal_stream_buffer_manager);

    mla_internal_stream_buffer_manager* bufferManager = mla_pointer_get_data<mla_internal_stream_buffer_manager>(bufferManager_ptr);

    if (bufferManager == nullptr) {
        return mla_stream_noop_output(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_stream_buffer_manager_userdata_name, bufferManager_ptr);

    return {
        user_data,
        mla_internal_stream_output_write_to_buffer,
        mla_internal_stream_output_buffer_available_bytes
    };
}

mla_stream_input_t mla_stream_input_from_buffer(mla_size_t size) {

    mla_pointer_t bufferManager_ptr = mla_malloc_buffer(sizeof(mla_internal_stream_buffer_manager) + size);
    mla_internal_stream_buffer_manager* bufferManager = mla_pointer_get_data<mla_internal_stream_buffer_manager>(bufferManager_ptr);

    if (bufferManager == nullptr) {
        return mla_stream_noop_input(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_stream_buffer_manager_userdata_name, bufferManager_ptr);

    return {
        user_data,
        mla_internal_stream_input_read_from_buffer,
        mla_internal_stream_input_buffer_remaining_bytes
    };

}

mla_stream_output_t mla_stream_output_to_buffer(mla_size_t size) {

    mla_pointer_t bufferManager_ptr = mla_malloc_buffer(sizeof(mla_internal_stream_buffer_manager) + size);
    mla_internal_stream_buffer_manager* bufferManager = mla_pointer_get_data<mla_internal_stream_buffer_manager>(bufferManager_ptr);

    if (bufferManager == nullptr) {
        return mla_stream_noop_output(); // Return noop stream on allocation failure
    }

    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_stream_buffer_manager_userdata_name, bufferManager_ptr);

    return {
        user_data,
        mla_internal_stream_output_write_to_buffer,
        mla_internal_stream_output_buffer_available_bytes
    };

}

mla_size_t mla_internal_stream_std_input_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

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
        mla_internal_stream_std_input_read,
        nullptr
    };

}

mla_size_t mla_internal_stream_std_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    (void)output;

    const mla_char_t* tempBuffer = reinterpret_cast<const mla_char_t*>(buffer);
    return mla_print(tempBuffer + offset, length);
}

mla_stream_output_t mla_stream_output_stdout() {

    return {
        mla_user_data_empty(),
        mla_internal_stream_std_output_write,
        nullptr
    };

}

struct mla_memory_stream_buffer_t {

    mla_bytes_t buffer;
    mla_size_t size;
    mla_size_t position;
    mla_bool_t can_grow;

    static mla_memory_stream_buffer_t init() {
        return {
            mla_bytes_empty(),
                0,
                0,
                false
            };
    }

};


mla_user_data_id_init(mla_memory_stream_buffer_userdata_name)

mla_size_t mla_internal_memory_stream_input_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0; // No buffer, no data
    }

    const mla_byte_t* data_buffer = mla_bytes_get_data_readonly(memBuffer->buffer);

    if (data_buffer == nullptr) {
        return 0; // No data, cannot read
    }

    if (memBuffer->position + length > memBuffer->size) {
        length = memBuffer->size - memBuffer->position;
    }

    // Copy data to the buffer
    mla_memcpy(buffer + offset, data_buffer + memBuffer->position, length);
    memBuffer->position += length;
    return length;
}

mla_size_t mla_internal_memory_stream_input_remaining_bytes(mla_stream_input_t& input) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0; // No buffer, no data
    }

    return memBuffer->size - memBuffer->position;
}

mla_size_t mla_internal_memory_stream_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(output.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0; // No buffer, cannot write
    }

    if (memBuffer->position + length > mla_bytes_length(memBuffer->buffer)) {

        if (memBuffer->can_grow) {

            // Need to resize the buffer
            mla_size_t newSize = mla_max(mla_max(mla_bytes_length(memBuffer->buffer) * 2, memBuffer->position + length), mla_global_config_stream_fast_read_buffer_size);
            // We can not use realloc so we need to use malloc and memcpy
            mla_bytes_t newBuffer = mla_bytes(newSize);

            mla_byte_t* new_data = mla_bytes_get_data_for_writing(newBuffer);
            const mla_byte_t* old_data = mla_bytes_get_data_readonly(memBuffer->buffer);

            if (new_data != nullptr) {

                if (old_data != nullptr) {

                    mla_memcpy(new_data, old_data, memBuffer->size);
                    memBuffer->buffer = mla_bytes_empty();
                    // Zero the new allocated part
                    if (newSize > memBuffer->size) {
                        mla_memset(new_data + memBuffer->size, 0, newSize - memBuffer->size);
                    }
                } else {
                    mla_memset(new_data, 0, newSize);
                }

                memBuffer->buffer = newBuffer;

            } else {

                memBuffer->buffer = mla_bytes_empty();
                memBuffer->position = 0;
                memBuffer->size = 0;
                return 0; // Reallocation failed

            }

        } else {

            // Cannot grow, limit the length
            length = mla_bytes_length(memBuffer->buffer) - memBuffer->position;
            if (length == 0) {
                return 0; // No space available
            }
        }
    }

    mla_byte_t* data_buffer = mla_bytes_get_data_for_writing(memBuffer->buffer);

    // Copy the data
    mla_memcpy(data_buffer + memBuffer->position, buffer + offset, length);
    memBuffer->position += length;
    if (memBuffer->position > memBuffer->size) {
        memBuffer->size = memBuffer->position;
    }
    return length;

}

mla_size_t mla_internal_memory_stream_output_available_bytes(mla_stream_output_t& output) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(output.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0;
    }

    if (!memBuffer->can_grow) {
        return mla_bytes_length(memBuffer->buffer) - memBuffer->position;
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

mla_memory_stream_t mla_memory_stream(mla_size_t initial_size) {
    return mla_memory_stream(initial_size, true);
}

mla_memory_stream_t mla_memory_stream(mla_size_t initial_size, mla_bool_t can_grow) {

    mla_pointer_t memBuffer_ptr = mla_malloc_struct(mla_memory_stream_buffer_t);

    mla_memory_stream_buffer_t* memBuffer = mla_pointer_get_data<mla_memory_stream_buffer_t>(memBuffer_ptr);

    if (memBuffer == nullptr) {
        return mla_memory_stream_invalid();
    }

    if (initial_size > 0) {

        memBuffer->buffer = mla_bytes(initial_size);

        if (mla_bytes_is_empty(memBuffer->buffer)) {
            return mla_memory_stream_invalid();
        }
    }

    memBuffer->can_grow = can_grow;
    memBuffer->size = 0;
    memBuffer->position = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_memory_stream_buffer_userdata_name, memBuffer_ptr);

    return {
        {
            user_data,
            mla_internal_memory_stream_input_read,
            mla_internal_memory_stream_input_remaining_bytes,
        },
        {
            user_data,
            mla_internal_memory_stream_output_write,
            mla_internal_memory_stream_output_available_bytes,
        }
    };
}

mla_size_t mla_memory_stream_get_size(const mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0;
    }

    return memBuffer->size;
}

mla_size_t mla_memory_stream_get_position(const mla_memory_stream_t &memoryStream) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return 0;
    }

    return memBuffer->position;
}

mla_bool_t mla_memory_stream_set_position(mla_memory_stream_t &memoryStream, mla_size_t position) {

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

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

    mla_memory_stream_buffer_t* memBuffer = mla_user_data_get_pointer_data<mla_memory_stream_buffer_t>(memoryStream.input.userdata, mla_memory_stream_buffer_userdata_name);

    if (memBuffer == nullptr) {
        return;
    }

    mla_size_t oldSize = memBuffer->size;
    memBuffer->size = 0;
    memBuffer->position = 0;

    mla_byte_t* data_buffer = mla_bytes_get_data_for_writing(memBuffer->buffer);

    if (data_buffer == nullptr) {
        return;
    }

    if (oldSize > 0) {
        mla_memset(data_buffer, 0, oldSize);
    }
}

mla_user_data_id_init(mla_stream_output_size_calculation_user_data_name)

mla_size_t mla_internal_stream_output_size_calculation_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
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
        mla_internal_stream_output_size_calculation_write,
        nullptr
    };

}

mla_size_t mla_stream_output_size_calculation_get_size(const mla_stream_output_t &output) {
    return mla_user_data_get_uint32(output.userdata, mla_stream_output_size_calculation_user_data_name, 0);
}
