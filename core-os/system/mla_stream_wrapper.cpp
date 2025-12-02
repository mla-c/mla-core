//
// Created by christian on 11/5/25.
//

#include "mla_stream.h"
#include "../log/mla_logging.h"


struct mla_stream_input_timeout_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_int32_t timeout_ms;
};

mla_size_t mla_stream_input_read_with_timeout(const mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer, mla_int32_t timeout_ms) {

    mla_int32_t remaining_timeout = timeout_ms;
    mla_size_t result = 0;

    while (true) {
        result = result + input.read(input, offset + result, length - result, buffer);

        if (result == length) {
            break; // Read complete
        }

        if (remaining_timeout <= 0) {
            break; // Timeout reached
        }

        // Wait and retry
        mla_sleep(10);
        remaining_timeout -= 10;
    }

    return result;
}

mla_size_t __mla_stream_input_timeout_wrapper_read(const mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {
    mla_stream_input_timeout_wrapper_data_t *data = reinterpret_cast<mla_stream_input_timeout_wrapper_data_t *>(input.userdata);

    if (data == nullptr) {
        return 0; // No data, return 0
    }

    return mla_stream_input_read_with_timeout(data->base_input, offset, length, buffer, data->timeout_ms);
}

mla_size_t __mla_stream_input_timeout_wrapper_remaining_bytes(const mla_stream_input_t &input) {
    mla_stream_input_timeout_wrapper_data_t *data = reinterpret_cast<mla_stream_input_timeout_wrapper_data_t *>(input.userdata);

    if (data == nullptr)
        return 0;

    mla_size_t remaining_bytes = data->base_input.remaining_bytes(data->base_input);

    if (remaining_bytes > 0) {
        return remaining_bytes;
    }

    mla_sleep(1);

    remaining_bytes = data->base_input.remaining_bytes(data->base_input);

    if (remaining_bytes > 0) {
        return remaining_bytes;
    }

    mla_sleep(10);

    return data->base_input.remaining_bytes(data->base_input);
}

mla_buffer_cleanup_mode __mla_stream_input_timeout_wrapper_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;
    mla_stream_input_timeout_wrapper_data_t *wrapper_data = reinterpret_cast<mla_stream_input_timeout_wrapper_data_t *>(data);
    wrapper_data->base_input = mla_stream_noop_input(); // Clear base input to avoid dangling references
    return CLEAN_UP_NEEDED;
}

mla_stream_input_t mla_stream_input_timeout_wrapper(const mla_stream_input_t &input, mla_int32_t timeout_ms) {
    if (input.read == nullptr)
        return input;

    mla_stream_input_timeout_wrapper_data_t *data = static_cast<mla_stream_input_timeout_wrapper_data_t *>(mla_malloc(
        sizeof(mla_stream_input_timeout_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_input(); // Allocation failed, return noop
    }

    mla_memset(data, 0, sizeof(mla_stream_input_timeout_wrapper_data_t));
    data->base_input = input;
    data->timeout_ms = timeout_ms;

    if (input.remaining_bytes != nullptr) {
        return {
            (mla_callback_userdata) data,
            __mla_stream_input_timeout_wrapper_read,
            __mla_stream_input_timeout_wrapper_remaining_bytes,
            mla_buffer_reference(data, true, __mla_stream_input_timeout_wrapper_cleanup)
        };
    } else {
        return {
            (mla_callback_userdata) data,
            __mla_stream_input_timeout_wrapper_read,
            nullptr,
            mla_buffer_reference(data, true, __mla_stream_input_timeout_wrapper_cleanup)
        };
    }
}


struct mla_stream_input_limited_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_size_t size;
    mla_size_t readed;
};


mla_size_t __mla_stream_input_limited_wrapper_read(const mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {
    mla_stream_input_limited_wrapper_data_t *data = reinterpret_cast<mla_stream_input_limited_wrapper_data_t *>(input.userdata);

    if (data == nullptr) {
        return 0; // No data, return 0
    }

    mla_size_t remaining = data->size - data->readed;
    if (remaining == 0) {
        return 0; // No more data to read
    }

    remaining = mla_min(remaining, length);

    mla_size_t readed = data->base_input.read(data->base_input, offset, remaining, buffer);
    data->readed += readed;
    return readed;
}

mla_size_t __mla_stream_input_limited_wrapper_remaining_bytes(const mla_stream_input_t &input) {

    mla_stream_input_limited_wrapper_data_t *data = reinterpret_cast<mla_stream_input_limited_wrapper_data_t *>(input.userdata);

    if (data == nullptr)
        return 0;

    return data->size - data->readed;
}

mla_buffer_cleanup_mode __mla_stream_input_limited_wrapper_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;
    mla_stream_input_limited_wrapper_data_t *wrapper_data = reinterpret_cast<mla_stream_input_limited_wrapper_data_t *>(data);
    wrapper_data->base_input = mla_stream_noop_input(); // Clear base input to avoid dangling references
    return CLEAN_UP_NEEDED;
}

mla_stream_input_t mla_stream_input_limited_wrapper(const mla_stream_input_t &input, mla_size_t size) {
    if (size == 0 || input.read == nullptr) {
        return mla_stream_noop_input();
    }

    if (input.read == nullptr)
        return input;

    mla_stream_input_limited_wrapper_data_t *data = static_cast<mla_stream_input_limited_wrapper_data_t *>(mla_malloc(
        sizeof(mla_stream_input_limited_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_input(); // Allocation failed, return noop
    }

    mla_memset(data, 0, sizeof(mla_stream_input_timeout_wrapper_data_t));
    data->base_input = input;
    data->readed = 0;
    data->size = size;

    return {
        (mla_callback_userdata) data,
        __mla_stream_input_limited_wrapper_read,
        __mla_stream_input_limited_wrapper_remaining_bytes,
        mla_buffer_reference(data, true, __mla_stream_input_limited_wrapper_cleanup)
    };
}

//////////////////////////////////////////////////////////////////////////////
/// Input Buffered Wrapper
//////////////////////////////////////////////////////////////////////////////


struct mla_stream_input_buffered_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_byte_t *buffer;
    mla_size_t buffer_size;
    mla_size_t buffer_offset;
    mla_size_t buffer_filled;
};

mla_size_t __mla_stream_input_buffered_wrapper_read(const mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {
    mla_stream_input_buffered_wrapper_data_t *data = reinterpret_cast<mla_stream_input_buffered_wrapper_data_t *>(input.userdata);

    if (data == nullptr || buffer == nullptr) {
        return 0;
    }

    mla_size_t bytes_copied = 0;

    while (bytes_copied < length) {

        if (data->buffer_offset < data->buffer_filled) {
            mla_size_t available = data->buffer_filled - data->buffer_offset;
            mla_size_t to_copy = mla_min(available, length - bytes_copied);
            mla_memcpy(buffer + offset + bytes_copied, data->buffer + data->buffer_offset, to_copy);
            data->buffer_offset += to_copy;
            bytes_copied += to_copy;
        } else {
            data->buffer_offset = 0;
            data->buffer_filled = data->base_input.read(data->base_input, 0, data->buffer_size, data->buffer);

            if (data->buffer_filled == 0) {
                break;
            }
        }
    }

    return bytes_copied;
}

mla_size_t __mla_stream_input_buffered_wrapper_remaining_bytes(const mla_stream_input_t &input) {
    mla_stream_input_buffered_wrapper_data_t *data = reinterpret_cast<mla_stream_input_buffered_wrapper_data_t *>(input.userdata);

    if (data == nullptr || data->base_input.remaining_bytes == nullptr) {
        return 0;
    }

    mla_size_t buffered = data->buffer_filled - data->buffer_offset;

    if (buffered > 0) {
        return mla_size_max;
    }

    return  data->base_input.remaining_bytes(data->base_input);
}

mla_buffer_cleanup_mode __mla_stream_input_buffered_wrapper_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;
    mla_stream_input_buffered_wrapper_data_t *wrapper_data = reinterpret_cast<mla_stream_input_buffered_wrapper_data_t *>(data);

    if (wrapper_data->buffer != nullptr) {
        mla_free(wrapper_data->buffer);
    }

    wrapper_data->base_input = mla_stream_noop_input();
    return CLEAN_UP_NEEDED;
}

mla_stream_input_t mla_stream_input_buffered_wrapper(const mla_stream_input_t &input, mla_size_t buffer_size) {
    if (input.read == nullptr || buffer_size == 0) {
        return input;
    }

    mla_stream_input_buffered_wrapper_data_t *data = static_cast<mla_stream_input_buffered_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_input_buffered_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_input();
    }

    mla_memset(data, 0, sizeof(mla_stream_input_buffered_wrapper_data_t));
    data->buffer = static_cast<mla_byte_t *>(mla_malloc(buffer_size));

    if (data->buffer == nullptr) {
        mla_free(data);
        return mla_stream_noop_input();
    }

    data->base_input = input;
    data->buffer_size = buffer_size;
    data->buffer_offset = 0;
    data->buffer_filled = 0;

    return {
        (mla_callback_userdata) data,
        __mla_stream_input_buffered_wrapper_read,
        input.remaining_bytes != nullptr ? __mla_stream_input_buffered_wrapper_remaining_bytes : nullptr,
        mla_buffer_reference(data, true, __mla_stream_input_buffered_wrapper_cleanup)
    };
}

//////////////////////////////////////////////////////////////////////////////
/// Output Buffered Wrapper
//////////////////////////////////////////////////////////////////////////////

struct mla_stream_output_buffered_wrapper_data_t {
    mla_stream_output_t base_output;
    mla_byte_t *buffer;
    mla_size_t buffer_size;
    mla_size_t buffer_used;
};

mla_size_t __mla_stream_output_buffered_wrapper_write(const mla_stream_output_t &output, mla_size_t offset, mla_size_t length, const mla_byte_t *buffer) {
    mla_stream_output_buffered_wrapper_data_t *data = reinterpret_cast<mla_stream_output_buffered_wrapper_data_t *>(output.userdata);

    if (data == nullptr || buffer == nullptr) {
        return 0;
    }

    mla_size_t bytes_written = 0;

    while (bytes_written < length) {
        mla_size_t available_in_buffer = data->buffer_size - data->buffer_used;
        mla_size_t to_copy = mla_min(available_in_buffer, length - bytes_written);

        mla_memcpy(data->buffer + data->buffer_used, buffer + offset + bytes_written, to_copy);
        data->buffer_used += to_copy;
        bytes_written += to_copy;

        if (data->buffer_used >= data->buffer_size) {
            mla_size_t written = data->base_output.write(data->base_output, 0, data->buffer_used, data->buffer);

            if (written != data->buffer_used) {
                mla_error(mla_string_const("Failed to write all buffered data to base output stream"));
            }
            data->buffer_used = 0;
        }
    }

    return bytes_written;
}

mla_buffer_cleanup_mode __mla_stream_output_buffered_wrapper_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;
    mla_stream_output_buffered_wrapper_data_t *wrapper_data = reinterpret_cast<mla_stream_output_buffered_wrapper_data_t *>(data);

    if (wrapper_data->buffer != nullptr) {
        mla_free(wrapper_data->buffer);
    }

    wrapper_data->base_output = mla_stream_noop_output();
    return CLEAN_UP_NEEDED;
}

mla_size_t __mla_stream_output_buffered_wrapper_available_bytes(const mla_stream_output_t &output) {
    mla_stream_output_buffered_wrapper_data_t *data = reinterpret_cast<mla_stream_output_buffered_wrapper_data_t *>(output.userdata);

    if (data == nullptr || data->base_output.available_bytes == nullptr) {
        return 0;
    }

    mla_size_t base_available = data->base_output.available_bytes(data->base_output);
    mla_size_t buffer_available = data->buffer_size - data->buffer_used;

    return base_available + buffer_available;
}

mla_stream_output_t mla_stream_output_buffered_wrapper(const mla_stream_output_t &output, mla_size_t buffer_size) {
    if (output.write == nullptr || buffer_size == 0) {
        return output;
    }

    mla_stream_output_buffered_wrapper_data_t *data = static_cast<mla_stream_output_buffered_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_output_buffered_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_output();
    }

    mla_memset(data, 0, sizeof(mla_stream_output_buffered_wrapper_data_t));
    data->buffer = static_cast<mla_byte_t *>(mla_malloc(buffer_size));

    if (data->buffer == nullptr) {
        mla_free(data);
        return mla_stream_noop_output();
    }

    data->base_output = output;
    data->buffer_size = buffer_size;
    data->buffer_used = 0;

    return {
        (mla_callback_userdata) data,
        __mla_stream_output_buffered_wrapper_write,
        output.available_bytes != nullptr ? __mla_stream_output_buffered_wrapper_available_bytes : nullptr,
        mla_buffer_reference(data, true, __mla_stream_output_buffered_wrapper_cleanup)
    };
}

void mla_stream_output_flush_buffered_wrapper(const mla_stream_output_t &output) {
    mla_stream_output_buffered_wrapper_data_t *data = reinterpret_cast<mla_stream_output_buffered_wrapper_data_t *>(output.userdata);

    if (data == nullptr || data->buffer_used == 0) {
        return;
    }

    mla_size_t written = data->base_output.write(data->base_output, 0, data->buffer_used, data->buffer);

    if (written != data->buffer_used) {
        mla_error(mla_string_const("Failed to write all buffered data to base output stream during flush"));
    }

    data->buffer_used = 0;
}


