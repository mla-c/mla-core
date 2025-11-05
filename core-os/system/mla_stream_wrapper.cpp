//
// Created by christian on 11/5/25.
//

#include "mla_stream.h"


struct mla_stream_input_timeout_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_int32_t timeout_ms;
};

mla_size_t __mla_stream_input_timeout_wrapper_read(const mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {
    mla_stream_input_timeout_wrapper_data_t *data = reinterpret_cast<mla_stream_input_timeout_wrapper_data_t *>(input.userdata);

    if (data == nullptr) {
        return 0; // No data, return 0
    }

    mla_int32_t remaining_timeout = data->timeout_ms;
    mla_size_t result = 0;

    while (true) {
        result = result + data->base_input.read(data->base_input, offset + result, length - result, buffer);

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
