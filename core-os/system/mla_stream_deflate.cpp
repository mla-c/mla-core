//
// Created by copilot on 3/15/26.
//

#include "mla_stream.h"
#include <zlib.h>

#define MLA_DEFLATE_BUFFER_SIZE 4096

//////////////////////////////////////////////////////////////////////////////
/// Custom zlib allocator using mla_malloc / mla_free
//////////////////////////////////////////////////////////////////////////////

static voidpf mla_zlib_alloc(voidpf opaque, uInt items, uInt size) {
    (void)opaque;
    mla_size_t total = (mla_size_t)items * (mla_size_t)size;
    return (voidpf)mla_malloc(total);
}

static void mla_zlib_free(voidpf opaque, voidpf address) {
    (void)opaque;
    mla_free(address);
}

//////////////////////////////////////////////////////////////////////////////
/// Deflate (Compression) Output Wrapper
//////////////////////////////////////////////////////////////////////////////

mla_user_data_id_init(mla_stream_output_deflate_wrapper_data_name)

struct mla_stream_output_deflate_wrapper_data_t {
    mla_stream_output_t base_output;
    z_stream zstream;
    mla_byte_t output_buffer[MLA_DEFLATE_BUFFER_SIZE];
    mla_bool_t initialized;
};

struct mla_stream_output_deflate_wrapper_data_initializer {

    static mla_stream_output_deflate_wrapper_data_t init() {
        mla_stream_output_deflate_wrapper_data_t result = {
            mla_stream_noop_output(),
            {},
            {},
            false
        };
        return result;
    }
};

static mla_bool_t __mla_stream_output_deflate_flush_compressed(mla_stream_output_deflate_wrapper_data_t *data) {
    mla_size_t have = MLA_DEFLATE_BUFFER_SIZE - (mla_size_t)data->zstream.avail_out;
    if (have > 0) {
        mla_size_t written = data->base_output.write(data->base_output, 0, have, data->output_buffer);
        if (written != have) {
            return false;
        }
    }
    return true;
}

mla_size_t __mla_stream_output_deflate_wrapper_write(mla_stream_output_t &output, mla_size_t offset, mla_size_t length, const mla_byte_t *buffer) {

    mla_stream_output_deflate_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_output_deflate_wrapper_data_t>(output.userdata, mla_stream_output_deflate_wrapper_data_name);

    if (data == nullptr || buffer == nullptr || length == 0) {
        return 0;
    }

    if (!data->initialized) {
        return 0;
    }

    data->zstream.avail_in = (uInt)length;
    data->zstream.next_in = (Bytef*)(buffer + offset);

    mla_size_t total_consumed = 0;

    do {
        data->zstream.avail_out = (uInt)MLA_DEFLATE_BUFFER_SIZE;
        data->zstream.next_out = (Bytef*)data->output_buffer;

        int ret = deflate(&data->zstream, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR) {
            return total_consumed;
        }

        if (!__mla_stream_output_deflate_flush_compressed(data)) {
            return total_consumed;
        }

        total_consumed = length - (mla_size_t)data->zstream.avail_in;

    } while (data->zstream.avail_out == 0);

    return total_consumed;
}

mla_size_t __mla_stream_output_deflate_wrapper_available_bytes(mla_stream_output_t &output) {

    mla_stream_output_deflate_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_output_deflate_wrapper_data_t>(output.userdata, mla_stream_output_deflate_wrapper_data_name);

    if (data == nullptr || data->base_output.available_bytes == nullptr) {
        return mla_size_max;
    }

    return data->base_output.available_bytes(data->base_output);
}

mla_stream_output_t mla_stream_output_deflate_wrapper(mla_stream_output_t &output) {
    if (output.write == nullptr) {
        return mla_stream_noop_output();
    }

    mla_stream_output_deflate_wrapper_data_t *data = static_cast<mla_stream_output_deflate_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_output_deflate_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_output();
    }

    mla_memset(data, 0, sizeof(mla_stream_output_deflate_wrapper_data_t));
    data->base_output = output;

    data->zstream.zalloc = mla_zlib_alloc;
    data->zstream.zfree = mla_zlib_free;
    data->zstream.opaque = Z_NULL;

    int ret = deflateInit(&data->zstream, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        mla_free(data);
        return mla_stream_noop_output();
    }

    data->initialized = true;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_output_deflate_wrapper_data_t, mla_stream_output_deflate_wrapper_data_initializer>(user_data, mla_stream_output_deflate_wrapper_data_name, data);

    return {
        user_data,
        __mla_stream_output_deflate_wrapper_write,
        output.available_bytes != nullptr ? __mla_stream_output_deflate_wrapper_available_bytes : nullptr
    };
}

mla_bool_t mla_stream_output_deflate_finish(const mla_stream_output_t &output) {

    mla_stream_output_deflate_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_output_deflate_wrapper_data_t>(output.userdata, mla_stream_output_deflate_wrapper_data_name);

    if (data == nullptr || !data->initialized) {
        return false;
    }

    data->zstream.avail_in = 0;
    data->zstream.next_in = Z_NULL;

    int ret;
    do {
        data->zstream.avail_out = (uInt)MLA_DEFLATE_BUFFER_SIZE;
        data->zstream.next_out = (Bytef*)data->output_buffer;

        ret = deflate(&data->zstream, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            deflateEnd(&data->zstream);
            data->initialized = false;
            return false;
        }

        if (!__mla_stream_output_deflate_flush_compressed(data)) {
            deflateEnd(&data->zstream);
            data->initialized = false;
            return false;
        }

    } while (ret != Z_STREAM_END);

    deflateEnd(&data->zstream);
    data->initialized = false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
/// Inflate (Decompression) Input Wrapper
//////////////////////////////////////////////////////////////////////////////

mla_user_data_id_init(mla_stream_input_inflate_wrapper_data_name)

struct mla_stream_input_inflate_wrapper_data_t {
    mla_stream_input_t base_input;
    z_stream zstream;
    mla_byte_t input_buffer[MLA_DEFLATE_BUFFER_SIZE];
    mla_bool_t initialized;
    mla_bool_t finished;
};

struct mla_stream_input_inflate_wrapper_data_initializer {

    static mla_stream_input_inflate_wrapper_data_t init() {
        mla_stream_input_inflate_wrapper_data_t result = {
            mla_stream_noop_input(),
            {},
            {},
            false,
            false
        };
        return result;
    }
};

mla_size_t __mla_stream_input_inflate_wrapper_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {

    mla_stream_input_inflate_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_inflate_wrapper_data_t>(input.userdata, mla_stream_input_inflate_wrapper_data_name);

    if (data == nullptr || buffer == nullptr || length == 0) {
        return 0;
    }

    if (!data->initialized || data->finished) {
        return 0;
    }

    data->zstream.avail_out = (uInt)length;
    data->zstream.next_out = (Bytef*)(buffer + offset);

    mla_size_t total_produced = 0;

    while (data->zstream.avail_out > 0 && !data->finished) {

        if (data->zstream.avail_in == 0) {
            mla_size_t bytes_read = data->base_input.read(data->base_input, 0, MLA_DEFLATE_BUFFER_SIZE, data->input_buffer);

            if (bytes_read == 0) {
                break;
            }

            data->zstream.avail_in = (uInt)bytes_read;
            data->zstream.next_in = (Bytef*)data->input_buffer;
        }

        int ret = inflate(&data->zstream, Z_NO_FLUSH);

        if (ret == Z_STREAM_END) {
            data->finished = true;
            break;
        }

        if (ret != Z_OK && ret != Z_BUF_ERROR) {
            data->finished = true;
            break;
        }
    }

    total_produced = length - (mla_size_t)data->zstream.avail_out;
    return total_produced;
}

mla_size_t __mla_stream_input_inflate_wrapper_remaining_bytes(mla_stream_input_t &input) {

    mla_stream_input_inflate_wrapper_data_t *data = mla_user_data_get_pointer<mla_stream_input_inflate_wrapper_data_t>(input.userdata, mla_stream_input_inflate_wrapper_data_name);

    if (data == nullptr) {
        return 0;
    }

    if (data->finished) {
        return 0;
    }

    if (data->zstream.avail_in > 0) {
        return mla_size_max;
    }

    if (data->base_input.remaining_bytes != nullptr) {
        mla_size_t base_remaining = data->base_input.remaining_bytes(data->base_input);
        if (base_remaining > 0) {
            return mla_size_max;
        }
        return 0;
    }

    return mla_size_max;
}

mla_stream_input_t mla_stream_input_inflate_wrapper(mla_stream_input_t &input) {
    if (input.read == nullptr) {
        return mla_stream_noop_input();
    }

    mla_stream_input_inflate_wrapper_data_t *data = static_cast<mla_stream_input_inflate_wrapper_data_t *>(mla_malloc(sizeof(mla_stream_input_inflate_wrapper_data_t)));

    if (data == nullptr) {
        return mla_stream_noop_input();
    }

    mla_memset(data, 0, sizeof(mla_stream_input_inflate_wrapper_data_t));
    data->base_input = input;
    data->finished = false;

    data->zstream.zalloc = mla_zlib_alloc;
    data->zstream.zfree = mla_zlib_free;
    data->zstream.opaque = Z_NULL;
    data->zstream.avail_in = 0;
    data->zstream.next_in = Z_NULL;

    int ret = inflateInit(&data->zstream);
    if (ret != Z_OK) {
        mla_free(data);
        return mla_stream_noop_input();
    }

    data->initialized = true;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<mla_stream_input_inflate_wrapper_data_t, mla_stream_input_inflate_wrapper_data_initializer>(user_data, mla_stream_input_inflate_wrapper_data_name, data);

    return {
        user_data,
        __mla_stream_input_inflate_wrapper_read,
        input.remaining_bytes != nullptr ? __mla_stream_input_inflate_wrapper_remaining_bytes : nullptr
    };
}
