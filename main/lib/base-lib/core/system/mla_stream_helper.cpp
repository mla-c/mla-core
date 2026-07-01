//
// Created by christian on 10/8/25.
//

#include "mla_stream.h"

mla_bool_t mla_stream_equal(mla_stream_input_t &input1, mla_stream_input_t &input2) {

    mla_byte_t buffer1[mla_global_config_stream_fast_read_buffer_size];
    mla_byte_t buffer2[mla_global_config_stream_fast_read_buffer_size];

    mla_size_t bytesRead1;
    mla_size_t bytesRead2;

    mla_size_t offset = 0;

    do {
        bytesRead1 = input1.read(input1, offset, sizeof(buffer1), buffer1);
        bytesRead2 = input2.read(input2, offset, sizeof(buffer2), buffer2);

        if (bytesRead1 != bytesRead2) {
            return false; // Different sizes
        }

        if (bytesRead1 > 0 && mla_memcmp(buffer1, buffer2, bytesRead1) != 0) {
            return false; // Different content
        }

        offset += bytesRead1;

    } while (bytesRead1 == mla_global_config_stream_fast_read_buffer_size && bytesRead2 == mla_global_config_stream_fast_read_buffer_size);

    return true;}

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

    return output.write(output, 0, length, mla_r_cast<const mla_byte_t*>(data)) == length;
}