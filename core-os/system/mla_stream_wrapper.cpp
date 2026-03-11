//
// Created by christian on 11/5/25.
//

#include "mla_stream.h"
#include "../log/mla_logging.h"

mla_user_data_id_init(mla_stream_input_timeout_wrapper_data_userdata_name)

struct mla_stream_input_timeout_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_int32_t timeout_ms;
};

struct mla_stream_input_timeout_wrapper_data_initializer {

    static mla_stream_input_timeout_wrapper_data_t init() {
        return {
            mla_stream_noop_input(),
            0
        };
    }
};

mla_stream_input_t mla_stream_input_from_string(const mla_string_t &string) {
    if (mla_string_is_empty(string)) {
        return mla_stream_noop_input();
    }

    return mla_stream_input_from_buffer(reinterpret_cast<mla_byte_t*>(const_cast<mla_char_t*>(mla_string_data(string))), mla_string_length(string));
}

mla_size_t mla_stream_input_read_with_timeout(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer, mla_int32_t timeout_ms) {

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

mla_size_t __mla_stream_input_timeout_wrapper_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {

    mla_stream_input_timeout_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_timeout_wrapper_data_t>(input.userdata, mla_stream_input_timeout_wrapper_data_userdata_name);

    if (data == nullptr) {
        return 0; // No data, return 0
    }

    return mla_stream_input_read_with_timeout(data->base_input, offset, length, buffer, data->timeout_ms);
}

mla_size_t __mla_stream_input_timeout_wrapper_remaining_bytes(mla_stream_input_t &input) {

    mla_stream_input_timeout_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_timeout_wrapper_data_t>(input.userdata, mla_stream_input_timeout_wrapper_data_userdata_name);

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


mla_stream_input_t mla_stream_input_timeout_wrapper(mla_stream_input_t &input, mla_int32_t timeout_ms) {
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

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_input_timeout_wrapper_data_t, mla_stream_input_timeout_wrapper_data_initializer>(user_data, mla_stream_input_timeout_wrapper_data_userdata_name, data);

    if (input.remaining_bytes != nullptr) {
        return {
            user_data,
            __mla_stream_input_timeout_wrapper_read,
            __mla_stream_input_timeout_wrapper_remaining_bytes
        };
    } else {
        return {
            user_data,
            __mla_stream_input_timeout_wrapper_read,
            nullptr
        };
    }
}


mla_user_data_id_init(mla_stream_input_limited_wrapper_data_name)

struct mla_stream_input_limited_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_size_t size;
    mla_size_t readed;
};

struct mla_stream_input_limited_wrapper_data_initializer {

    static mla_stream_input_limited_wrapper_data_t init() {
        return {
            mla_stream_noop_input(),
            0,
            0
        };
    }
};


mla_size_t __mla_stream_input_limited_wrapper_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {

    mla_stream_input_limited_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_limited_wrapper_data_t>(input.userdata, mla_stream_input_limited_wrapper_data_name);

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

mla_size_t __mla_stream_input_limited_wrapper_remaining_bytes(mla_stream_input_t &input) {

    mla_stream_input_limited_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_limited_wrapper_data_t>(input.userdata, mla_stream_input_limited_wrapper_data_name);

    if (data == nullptr)
        return 0;

    return data->size - data->readed;
}

mla_stream_input_t mla_stream_input_limited_wrapper(mla_stream_input_t &input, mla_size_t size) {
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

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_input_limited_wrapper_data_t, mla_stream_input_limited_wrapper_data_initializer>(user_data, mla_stream_input_limited_wrapper_data_name, data);

    return {
        user_data,
        __mla_stream_input_limited_wrapper_read,
        __mla_stream_input_limited_wrapper_remaining_bytes
    };
}

//////////////////////////////////////////////////////////////////////////////
/// Input Buffered Wrapper
//////////////////////////////////////////////////////////////////////////////

mla_user_data_id_init(mla_stream_input_buffered_wrapper_data_name)

struct mla_stream_input_buffered_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_bytes_t buffer;
    mla_size_t buffer_offset;
    mla_size_t buffer_filled;
};

struct mla_stream_input_buffered_wrapper_data_initializer {

    static mla_stream_input_buffered_wrapper_data_t init() {
        return {
            mla_stream_noop_input(),
            mla_bytes_empty(),
            0,
            0
        };
    }
};

mla_size_t __mla_stream_input_buffered_wrapper_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {
    mla_stream_input_buffered_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_buffered_wrapper_data_t>(input.userdata, mla_stream_input_buffered_wrapper_data_name);

    if (data == nullptr || buffer == nullptr) {
        return 0;
    }

    mla_size_t bytes_copied = 0;

    while (bytes_copied < length) {

        if (data->buffer_offset < data->buffer_filled) {
            mla_size_t available = data->buffer_filled - data->buffer_offset;
            mla_size_t to_copy = mla_min(available, length - bytes_copied);
            mla_memcpy(buffer + offset + bytes_copied, mla_bytes_get_data_for_writing(data->buffer) + data->buffer_offset, to_copy);
            data->buffer_offset += to_copy;
            bytes_copied += to_copy;
        } else {
            data->buffer_offset = 0;
            data->buffer_filled = data->base_input.read(data->base_input, 0, mla_bytes_length(data->buffer), mla_bytes_get_data_for_writing(data->buffer));

            if (data->buffer_filled == 0) {
                break;
            }
        }
    }

    return bytes_copied;
}

mla_size_t __mla_stream_input_buffered_wrapper_remaining_bytes(mla_stream_input_t &input) {
    mla_stream_input_buffered_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_buffered_wrapper_data_t>(input.userdata, mla_stream_input_buffered_wrapper_data_name);

    if (data == nullptr || data->base_input.remaining_bytes == nullptr) {
        return 0;
    }

    mla_size_t buffered = data->buffer_filled - data->buffer_offset;

    if (buffered > 0) {
        return mla_size_max;
    }

    return  data->base_input.remaining_bytes(data->base_input);
}

mla_stream_input_t mla_stream_input_buffered_wrapper(mla_stream_input_t &input, mla_size_t buffer_size) {
    if (input.read == nullptr || buffer_size == 0) {
        return input;
    }

    mla_stream_input_buffered_wrapper_data_t *data = static_cast<mla_stream_input_buffered_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_input_buffered_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_input();
    }

    mla_memset(data, 0, sizeof(mla_stream_input_buffered_wrapper_data_t));
    data->buffer = mla_bytes(buffer_size);

    if (mla_bytes_is_empty(data->buffer)) {
        mla_free(data);
        return input;
    }

    data->base_input = input;
    data->buffer_offset = 0;
    data->buffer_filled = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_input_buffered_wrapper_data_t, mla_stream_input_buffered_wrapper_data_initializer>(user_data, mla_stream_input_buffered_wrapper_data_name, data);

    return {
        user_data,
        __mla_stream_input_buffered_wrapper_read,
        input.remaining_bytes != nullptr ? __mla_stream_input_buffered_wrapper_remaining_bytes : nullptr
    };
}

//////////////////////////////////////////////////////////////////////////////
/// Output Buffered Wrapper
//////////////////////////////////////////////////////////////////////////////

mla_user_data_id_init(mla_stream_output_buffered_wrapper_data_name)

struct mla_stream_output_buffered_wrapper_data_t {
    mla_stream_output_t base_output;
    mla_bytes_t buffer;
    mla_size_t buffer_used;
};

struct mla_stream_output_buffered_wrapper_data_initializer {

    static mla_stream_output_buffered_wrapper_data_t init() {
        return {
            mla_stream_noop_output(),
            mla_bytes_empty(),
            0
        };
    }
};

mla_size_t __mla_stream_output_buffered_wrapper_write(mla_stream_output_t &output, mla_size_t offset, mla_size_t length, const mla_byte_t *buffer) {
    mla_stream_output_buffered_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_output_buffered_wrapper_data_t>(output.userdata, mla_stream_output_buffered_wrapper_data_name);

    if (data == nullptr || buffer == nullptr) {
        return 0;
    }

    mla_size_t bytes_written = 0;

    mla_size_t buffer_size = mla_bytes_length(data->buffer);
    mla_byte_t* buffer_data = mla_bytes_get_data_for_writing(data->buffer);

    while (bytes_written < length) {
        mla_size_t available_in_buffer = buffer_size - data->buffer_used;
        mla_size_t to_copy = mla_min(available_in_buffer, length - bytes_written);

        mla_memcpy(buffer_data + data->buffer_used, buffer + offset + bytes_written, to_copy);
        data->buffer_used += to_copy;
        bytes_written += to_copy;

        if (data->buffer_used >= buffer_size) {
            mla_size_t written = data->base_output.write(data->base_output, 0, data->buffer_used, buffer_data);

            if (written != data->buffer_used) {
                mla_debug(mla_string_const("Failed to write all buffered data to base output stream"));
                return bytes_written - (data->buffer_used - written); // Return the actual bytes written before the error
            }
            data->buffer_used = 0;
        }
    }

    return bytes_written;
}

mla_size_t __mla_stream_output_buffered_wrapper_available_bytes(mla_stream_output_t &output) {

    mla_stream_output_buffered_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_output_buffered_wrapper_data_t>(output.userdata, mla_stream_output_buffered_wrapper_data_name);

    if (data == nullptr || data->base_output.available_bytes == nullptr) {
        return 0;
    }

    mla_size_t base_available = data->base_output.available_bytes(data->base_output);
    mla_size_t buffer_available = mla_bytes_length(data->buffer) - data->buffer_used;

    return base_available + buffer_available;
}

mla_stream_output_t mla_stream_output_buffered_wrapper(mla_stream_output_t &output, mla_size_t buffer_size) {
    if (output.write == nullptr || buffer_size == 0) {
        return output;
    }

    mla_stream_output_buffered_wrapper_data_t *data = static_cast<mla_stream_output_buffered_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_output_buffered_wrapper_data_t)));

    if (data == nullptr) {
        return output;
    }

    mla_memset(data, 0, sizeof(mla_stream_output_buffered_wrapper_data_t));
    data->buffer = mla_bytes(buffer_size);

    if (mla_bytes_is_empty(data->buffer)) {
        mla_free(data);
        return mla_stream_noop_output();
    }

    data->base_output = output;
    data->buffer_used = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_output_buffered_wrapper_data_t, mla_stream_output_buffered_wrapper_data_initializer>(user_data, mla_stream_output_buffered_wrapper_data_name, data);

    return {
        user_data,
        __mla_stream_output_buffered_wrapper_write,
        output.available_bytes != nullptr ? __mla_stream_output_buffered_wrapper_available_bytes : nullptr
    };
}

mla_bool_t mla_stream_output_flush_buffered_wrapper(const mla_stream_output_t &output) {

    mla_stream_output_buffered_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_output_buffered_wrapper_data_t>(output.userdata, mla_stream_output_buffered_wrapper_data_name);

    if (data == nullptr || data->buffer_used == 0) {
        return true; // Nothing to flush
    }

    mla_byte_t* buffer_data = mla_bytes_get_data_for_writing(data->buffer);
    mla_size_t written = data->base_output.write(data->base_output, 0, data->buffer_used, buffer_data);

    if (written != data->buffer_used) {
        mla_error(mla_string_const("Failed to write all buffered data to base output stream during flush"));
        return false;
    }

    data->buffer_used = 0;
    return true;
}

///////////////////////////////////////
/// Input Interceptor Wrappers
///////////////////////////////////////

mla_user_data_id_init(mla_stream_input_interceptor_wrapper_data_name)

struct mla_stream_input_interceptor_wrapper_data_t {
    mla_stream_input_t base_input;
    mla_stream_input_interceptor_read read;
    mla_stream_input_interceptor_remaining_bytes remaining_bytes;
};

struct mla_stream_input_interceptor_wrapper_data_initializer {

    static mla_stream_input_interceptor_wrapper_data_t init() {
        return {
            mla_stream_noop_input(),
            nullptr,
            nullptr
        };
    }
};

mla_size_t __mla_stream_input_interceptor_wrapper_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {

    mla_stream_input_interceptor_wrapper_data_t* data = mla_user_data_get_pointer<mla_stream_input_interceptor_wrapper_data_t>(input.userdata, mla_stream_input_interceptor_wrapper_data_name);

    if (data == nullptr) {
        return 0; // No data or no interceptor, return 0
    }

    if (data->read == nullptr) {

        if (data->base_input.read == nullptr) {
            return 0; // No read function to call
        }

        // No interceptor, pass through to base input
        return data->base_input.read(data->base_input, offset, length, buffer);
    }

    return data->read(input, data->base_input, offset, length, buffer);

}

mla_size_t __mla_stream_input_interceptor_wrapper_remaining_bytes(mla_stream_input_t &input) {

    mla_stream_input_interceptor_wrapper_data_t* data = mla_user_data_get_pointer<mla_stream_input_interceptor_wrapper_data_t>(input.userdata, mla_stream_input_interceptor_wrapper_data_name);

    if (data == nullptr) {
        return 0; // No data or no interceptor, return 0
    }

    if (data->remaining_bytes == nullptr) {

        if (data->base_input.remaining_bytes == nullptr) {
            return mla_size_max; // No remaining bytes function, assume data is there but unknown size
        }

        // No interceptor, pass through to base input
        return data->base_input.remaining_bytes(data->base_input);
    }

    return data->remaining_bytes(input, data->base_input);

}

mla_stream_input_t mla_stream_input_interceptor_wrapper(mla_stream_input_t &input, mla_stream_input_interceptor_read intercept_read_function, mla_stream_input_interceptor_remaining_bytes intercept_remaining_bytes_function) {

    mla_stream_input_interceptor_wrapper_data_t* data = static_cast<mla_stream_input_interceptor_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_input_interceptor_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_input();
    }

    mla_memset(data, 0, sizeof(mla_stream_input_interceptor_wrapper_data_t));
    data->base_input = input;
    data->read = intercept_read_function;
    data->remaining_bytes = intercept_remaining_bytes_function;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_input_interceptor_wrapper_data_t, mla_stream_input_interceptor_wrapper_data_initializer>(user_data, mla_stream_input_interceptor_wrapper_data_name, data);


    return {
        user_data,
        input.read != nullptr ? __mla_stream_input_interceptor_wrapper_read : nullptr,
        input.remaining_bytes != nullptr ? __mla_stream_input_interceptor_wrapper_remaining_bytes : nullptr
    };

}


///////////////////////////////////////
/// Output Interceptor Wrappers
///////////////////////////////////////

mla_user_data_id_init(mla_stream_output_interceptor_data_name)

struct mla_stream_output_interceptor_wrapper_data_t {
    mla_stream_output_t base_output;
    mla_stream_output_interceptor_write write;
    mla_stream_output_interceptor_available_bytes available_bytes;
};

struct mla_stream_output_interceptor_wrapper_data_initializer {

    static mla_stream_output_interceptor_wrapper_data_t init() {
        return {
            mla_stream_noop_output(),
            nullptr,
            nullptr
        };
    }
};

mla_size_t __mla_stream_output_interceptor_wrapper_write(mla_stream_output_t &output, mla_size_t offset, mla_size_t length, const mla_byte_t *buffer) {

    mla_stream_output_interceptor_wrapper_data_t* data = mla_user_data_get_pointer<mla_stream_output_interceptor_wrapper_data_t>(output.userdata, mla_stream_output_interceptor_data_name);

    if (data == nullptr) {
        return 0; // No data or no interceptor, return 0
    }

    if (data->write == nullptr) {

        if (data->base_output.write == nullptr) {
            return 0; // No write function to call
        }

        // No interceptor, pass through to base output
        return data->base_output.write(data->base_output, offset, length, buffer);
    }

    return data->write(output, data->base_output, offset, length, buffer);
}

mla_size_t __mla_stream_output_interceptor_wrapper_available_bytes(mla_stream_output_t &output) {

    mla_stream_output_interceptor_wrapper_data_t* data = mla_user_data_get_pointer<mla_stream_output_interceptor_wrapper_data_t>(output.userdata, mla_stream_output_interceptor_data_name);

    if (data == nullptr) {
        return 0; // No data or no interceptor, return 0
    }

    if (data->available_bytes == nullptr) {

        if (data->base_output.available_bytes == nullptr) {
            return mla_size_max; // No available bytes function, assume space is there but unknown size
        }

        // No interceptor, pass through to base output
        return data->base_output.available_bytes(data->base_output);
    }

    return data->available_bytes(output, data->base_output);

}

mla_stream_output_t mla_stream_output_interceptor_wrapper(mla_stream_output_t &output, mla_stream_output_interceptor_write intercept_write_function, mla_stream_output_interceptor_available_bytes intercept_available_bytes_function) {

    mla_stream_output_interceptor_wrapper_data_t* data = static_cast<mla_stream_output_interceptor_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_output_interceptor_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_output();
    }

    mla_memset(data, 0, sizeof(mla_stream_output_interceptor_wrapper_data_t));
    data->base_output = output;
    data->write = intercept_write_function;
    data->available_bytes = intercept_available_bytes_function;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_output_interceptor_wrapper_data_t, mla_stream_output_interceptor_wrapper_data_initializer>(user_data, mla_stream_output_interceptor_data_name, data);

    return {
        user_data,
        output.write != nullptr ? __mla_stream_output_interceptor_wrapper_write : nullptr,
        output.available_bytes != nullptr ? __mla_stream_output_interceptor_wrapper_available_bytes : nullptr
    };
}

