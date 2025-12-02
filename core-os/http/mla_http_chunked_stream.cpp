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
};

mla_buffer_cleanup_mode __mla_http_chunked_stream_input_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;

    mla_http_chunked_stream_input_userdata_t* userdata = reinterpret_cast<mla_http_chunked_stream_input_userdata_t*>(data);

    if (userdata == nullptr)
        return CLEAN_UP_SKIP;

    // Clean up ref counter
    userdata->baseStream = mla_stream_noop_input();
    return CLEAN_UP_NEEDED;
}

mla_size_t __mla_http_chunked_stream_input_available_bytes(const mla_stream_input_t& input) {

    mla_http_chunked_stream_input_userdata_t* userdata = reinterpret_cast<mla_http_chunked_stream_input_userdata_t*>(input.userdata);

    if (userdata == nullptr || userdata->endOfStream) {
        return 0;
    }

    return mla_size_max; // Unknown size
}

mla_bool_t __mla_http_chunked_stream_input_read_chunk_size(mla_http_chunked_stream_input_userdata_t* userdata) {

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

    userdata->chunkSizeRemaining = chunkSize;
    return true;
}

mla_size_t __mla_http_chunked_stream_input_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    mla_http_chunked_stream_input_userdata_t* userdata = reinterpret_cast<mla_http_chunked_stream_input_userdata_t*>(input.userdata);

    if (userdata == nullptr || userdata->endOfStream) {
        return 0;
    }

    // Read data from the base stream up to requested length. If the chunk ends, read the trailing CRLF and read the next chunk size.
    mla_size_t bytesRead = length;

    while (bytesRead != 0) {

        if (userdata->chunkSizeRemaining == 0) {

            if (!__mla_http_chunked_stream_input_read_chunk_size(userdata)) {
                return 0;
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

    mla_http_chunked_stream_input_userdata_t* userdata = reinterpret_cast<mla_http_chunked_stream_input_userdata_t*>(mla_malloc(sizeof(mla_http_chunked_stream_input_userdata_t)));

    if (userdata == nullptr)
        return mla_stream_noop_input();

    mla_memset(userdata, 0, sizeof(mla_http_chunked_stream_input_userdata_t));
    userdata->baseStream = baseStream;
    userdata->timeout_ms = timeout_ms;
    userdata->endOfStream = false;
    userdata->chunkSizeRemaining = 0;

    mla_buffer_reference_t owner = mla_buffer_reference(userdata, true, __mla_http_chunked_stream_input_cleanup);

    return {
        reinterpret_cast<mla_callback_userdata>(userdata),
        __mla_http_chunked_stream_input_read,
        __mla_http_chunked_stream_input_available_bytes,
        owner
    };

}


mla_bool_t mla_http_chunked_stream_output_finished(mla_http_chunked_stream_output_t &chunkedOutput) {

    // Write the terminating chunk
    if (!mla_stream_output_write_string(chunkedOutput.output, mla_string_const("0\r\n\r\n"))) {
        return false;
    }

    // Mark the chunked output as finished by writing the terminating chunk
    chunkedOutput = mla_http_chunked_stream_output_invalid();
    return true;
}

mla_http_chunked_stream_output_t mla_http_chunked_stream_output_invalid() {
    return {
        mla_stream_noop_output(),
        mla_stream_noop_output()
    };
}

mla_size_t __mla_http_chunked_stream_output_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    // Get the base stream
    const mla_stream_output_t* baseStream = reinterpret_cast<const mla_stream_output_t*>(output.userdata);
    if (baseStream == nullptr) {
        return 0;
    }

    // Write the chunk size in hexadecimal followed by CRLF
    mla_string_t chunkSizeStr = mla_string_from_uint64_hex(length);

    if (!mla_stream_output_write_string(*baseStream, chunkSizeStr)) {
        return 0;
    }

    if (!mla_stream_output_write_string(*baseStream, mla_string_const("\r\n"))) {
        return 0;
    }

    // Write the actual chunk data
    mla_size_t written = baseStream->write(*baseStream, offset, length, buffer);

    if (written != length) {
        return written;
    }

    // Write the trailing CRLF
    if (!mla_stream_output_write_string(*baseStream, mla_string_const("\r\n"))) {
        return 0;
    }

    return written;

}

mla_http_chunked_stream_output_t mla_http_chunked_stream_output(const mla_stream_output_t &baseStream) {

    return {
        {
            reinterpret_cast<mla_callback_userdata>(&baseStream),
            __mla_http_chunked_stream_output_write,
            baseStream.available_bytes,
            baseStream.refOwner
        },
        baseStream
    };
}