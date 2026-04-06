//
// Created by christian on 10/8/25.
//

#include "mla_stream.h"

mla_bool_t mla_stream_copy(mla_stream_input_t &input, mla_stream_output_t &output) {

    mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size];
    mla_size_t bytesRead;

    do {
        bytesRead = input.read(input, 0, sizeof(buffer), buffer);

        if (bytesRead > 0) {
            mla_size_t bytesWritten = output.write(output, 0, bytesRead, buffer);
            if (bytesWritten != bytesRead) {
                return false; // Write error
            }
        }

    } while (bytesRead == mla_global_config_stream_fast_read_buffer_size);

    return true;

}

mla_bool_t mla_stream_output_write_string(mla_stream_output_t &output, const mla_string_t &string) {

    mla_size_t length = mla_string_length(string);

    if (length == 0) {
        return true; // Nothing to write
    }

    const mla_char_t* data = mla_string_data(string);

    return output.write(output, 0, length, reinterpret_cast<const mla_byte_t*>(data)) == length;
}