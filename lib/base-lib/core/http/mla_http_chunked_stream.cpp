//
// Created by chris on 11/27/2025.
//

#include "mla_http_chunked_stream.h"
#include "mla_http_utils.h"
#include "../system/mla_number.h"

struct mla_http_chunked_stream_input_userdata_t {
    mla_stream_input_t baseStream;
    mla_int32_t timeout_ms;
    mla_bool_t endOfStream;
    mla_size_t chunkSizeRemaining;

    static mla_http_chunked_stream_input_userdata_t init() {
        return {
            mla_stream_noop_input(),
            0,
            false,
            0
        };
    }
};


mla_user_data_id_init(mla_http_chunked_stream_input_user_data_name)


mla_size_t mla_internal_http_chunked_stream_input_available_bytes(mla_stream_input_t& input) {

    mla_http_chunked_stream_input_userdata_t* userdata = mla_user_data_get_pointer_data<mla_http_chunked_stream_input_userdata_t>(input.userdata, mla_http_chunked_stream_input_user_data_name);

    if (userdata == nullptr || userdata->endOfStream) {
        return 0;
    }

    return mla_size_max; // Unknown size
}

mla_bool_t mla_internal_http_chunked_stream_input_read_chunk_size(mla_http_chunked_stream_input_userdata_t* userdata) {

    // Read the chunk size line
    mla_string_t chunkSizeLine = mla_string_empty();

    if (!mla_http_utils_read_line(userdata->baseStream, chunkSizeLine, userdata->timeout_ms)) {
        userdata->endOfStream = true;
        return false;
    }

    // if there is are line endings from the previous chunk, ignore them
    if (mla_string_is_empty(chunkSizeLine)) {
        if (!mla_http_utils_read_line(userdata->baseStream, chunkSizeLine, userdata->timeout_ms)) {
            userdata->endOfStream = true;
            return false;
        }
    }

    // Parse chunk size
    mla_uint64_t chunkSize = 0;
    if (!mla_parse_uint64_hex(chunkSizeLine, chunkSize)) {
        userdata->endOfStream = true;
        return false;
    }

    if (chunkSize == 0) {
        userdata->endOfStream = true;
        return false;
    }

    userdata->chunkSizeRemaining = (mla_size_t)chunkSize;
    return true;
}

mla_size_t mla_internal_http_chunked_stream_input_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_http_chunked_stream_input_userdata_t* userdata = mla_user_data_get_pointer_data<mla_http_chunked_stream_input_userdata_t>(input.userdata, mla_http_chunked_stream_input_user_data_name);

    if (userdata == nullptr || userdata->endOfStream) {
        return 0;
    }

    // Read data from the base stream up to requested length. If the chunk ends, read the trailing CRLF and read the next chunk size.
    mla_size_t bytesRead = length;

    while (bytesRead != 0) {

        if (userdata->chunkSizeRemaining == 0) {

            if (!mla_internal_http_chunked_stream_input_read_chunk_size(userdata)) {
                return length - bytesRead;
            }
        }

        mla_size_t toRead = mla_min(userdata->chunkSizeRemaining, bytesRead);
        mla_size_t result = mla_stream_input_read_with_timeout(userdata->baseStream, offset, toRead, buffer, userdata->timeout_ms);
        bytesRead -= result;

        if (result != toRead) {
            userdata->endOfStream = true;
            return length - bytesRead;
        }

        offset = offset + result;
        userdata->chunkSizeRemaining = userdata->chunkSizeRemaining - result;
    }

    return length;
}

mla_stream_input_t mla_http_chunked_stream_input(const mla_stream_input_t &baseStream, mla_int32_t timeout_ms) {

    mla_pointer_t userdata_ptr = mla_malloc_struct(mla_http_chunked_stream_input_userdata_t);

    mla_http_chunked_stream_input_userdata_t* userdata = mla_pointer_get_data<mla_http_chunked_stream_input_userdata_t>(userdata_ptr);

    if (userdata == nullptr) {
        return mla_stream_noop_input();
    }

    userdata->baseStream = baseStream;
    userdata->timeout_ms = timeout_ms;
    userdata->endOfStream = false;
    userdata->chunkSizeRemaining = 0;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_http_chunked_stream_input_user_data_name, userdata_ptr);

    return {
        user_data,
        mla_internal_http_chunked_stream_input_read,
        mla_internal_http_chunked_stream_input_available_bytes,
    };

}

mla_stream_input_t mla_http_chunked_stream_input_deflate(const mla_stream_input_t &baseStream, mla_int32_t timeout_ms) {

    mla_stream_input_t chunkedStream = mla_http_chunked_stream_input(baseStream, timeout_ms);
    return mla_stream_input_deflate_decompress_wrapper(chunkedStream);
}


mla_http_chunked_stream_output_t mla_http_chunked_stream_output_invalid() {
    return {
        mla_stream_noop_output(),
        mla_stream_noop_output(),
        mla_stream_noop_output(),
    };
}

struct mla_http_chunked_stream_output_userdata_t {
    mla_stream_output_t baseStream;
    mla_bytes_t buffer;
    mla_size_t currentSize;

    static mla_http_chunked_stream_output_userdata_t init() {
        return {
            mla_stream_noop_output(),
            mla_bytes_empty(),
            0
        };
    }
};


mla_user_data_id_init(mla_http_chunked_stream_output_user_data_name)

mla_bool_t mla_internal_http_chunked_stream_output_flush(mla_http_chunked_stream_output_userdata_t* userdata) {
    if (userdata->currentSize == 0) {
        return true;
    }

    // Write the chunk size in hexadecimal followed by CRLF
    mla_string_t chunkSizeStr = mla_string_from_uint64_hex_short(userdata->currentSize);
    mla_stream_output_t& baseStream = userdata->baseStream;

    if (!mla_stream_output_write_string(baseStream, chunkSizeStr)) {
        return false;
    }

    if (!mla_stream_output_write_string(baseStream, mla_string_const("\r\n"))) {
        return false;
    }

    mla_byte_t* buffer_data = mla_bytes_get_data_for_writing(userdata->buffer);

    // Write the actual chunk data
    mla_size_t written = baseStream.write(baseStream, 0, userdata->currentSize, buffer_data);

    if (written != userdata->currentSize) {
        return false;
    }

    // Write the trailing CRLF
    if (!mla_stream_output_write_string(baseStream, mla_string_const("\r\n"))) {
        return false;
    }

    userdata->currentSize = 0;
    return true;
}

mla_size_t mla_internal_http_chunked_stream_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    // Get the base stream
    mla_http_chunked_stream_output_userdata_t* userdata = mla_user_data_get_pointer_data<mla_http_chunked_stream_output_userdata_t>(output.userdata, mla_http_chunked_stream_output_user_data_name);

    if (userdata == nullptr) {
        return 0;
    }

    mla_size_t bytesWritten = 0;

    mla_size_t bufferSize = mla_bytes_length(userdata->buffer);
    mla_byte_t* buffer_data = mla_bytes_get_data_for_writing(userdata->buffer);

    while (bytesWritten < length) {
        mla_size_t spaceLeft = bufferSize - userdata->currentSize;
        mla_size_t toCopy = mla_min(spaceLeft, length - bytesWritten);

        if (toCopy > 0) {
            mla_memcpy(buffer_data + userdata->currentSize, buffer + offset + bytesWritten, toCopy);
            userdata->currentSize += toCopy;
            bytesWritten += toCopy;
        }

        if (userdata->currentSize == bufferSize) {
            if (!mla_internal_http_chunked_stream_output_flush(userdata)) {
                return bytesWritten; // Return what we managed to process
            }
        }
    }

    return bytesWritten;
}

mla_bool_t mla_http_chunked_stream_output_finished(mla_http_chunked_stream_output_t &chunkedOutput) {

    // Flush deflate stream
    mla_stream_output_deflate_finish(chunkedOutput.output);

    // Flush the chunked stream buffer
    mla_http_chunked_stream_output_userdata_t* userdata = mla_user_data_get_pointer_data<mla_http_chunked_stream_output_userdata_t>(chunkedOutput.chunkedStream.userdata, mla_http_chunked_stream_output_user_data_name);
    if (userdata != nullptr) {
        if (!mla_internal_http_chunked_stream_output_flush(userdata)) {
            return false;
        }
    }

    // Write the terminating chunk
    if (!mla_stream_output_write_string(chunkedOutput.baseStream, mla_string_const("0\r\n\r\n"))) {
        return false;
    }

    chunkedOutput = mla_http_chunked_stream_output_invalid();
    return true;
}

#define mla_http_chunked_stream_output_chunk_size 1024

mla_stream_output_t mla_internal_create_chunked_stream_output(const mla_stream_output_t &baseStream, mla_size_t bufferSize) {

    mla_pointer_t user_data_ptr = mla_malloc_struct(mla_http_chunked_stream_output_userdata_t);

    mla_http_chunked_stream_output_userdata_t* userdata = mla_pointer_get_data<mla_http_chunked_stream_output_userdata_t>(user_data_ptr);

    if (userdata == nullptr) {
        return mla_stream_noop_output();
    }
    mla_memset(userdata, 0, sizeof(mla_http_chunked_stream_output_userdata_t));

    userdata->baseStream = baseStream;
    userdata->buffer = mla_bytes(bufferSize);
    userdata->currentSize = 0;

    if (mla_bytes_is_empty(userdata->buffer)) {
        mla_platform_free(userdata);
        return mla_stream_noop_output();
    }

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_http_chunked_stream_output_user_data_name, user_data_ptr);

    return {
        user_data,
        mla_internal_http_chunked_stream_output_write,
        baseStream.available_bytes,
    };
}

mla_http_chunked_stream_output_t mla_http_chunked_stream_output(const mla_stream_output_t &baseStream) {
    mla_stream_output_t chunkedStream = mla_internal_create_chunked_stream_output(baseStream, mla_http_chunked_stream_output_chunk_size);

    return {
        chunkedStream,
        chunkedStream,
        baseStream
    };
}

#define mla_http_chunked_stream_output_deflate_chunk_size (mla_global_config_stream_output_deflate_min_compression_data_size * 2)

mla_http_chunked_stream_output_t mla_http_chunked_stream_output_deflate(const mla_stream_output_t &baseStream) {
    mla_stream_output_t chunkedStream = mla_internal_create_chunked_stream_output(baseStream, mla_http_chunked_stream_output_deflate_chunk_size);
    // ContentType: deflate is for the Browser zlib deflate
    mla_stream_output_t deflateStream = mla_stream_output_deflate_compress_wrapper(chunkedStream, mla_deflate_mode_zlib);

    return {
        deflateStream,
        chunkedStream,
        baseStream,
    };
}