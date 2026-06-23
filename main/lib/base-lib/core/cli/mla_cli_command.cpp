//
// Created by chris on 6/23/2026.
//

#include "mla_cli_command.h"

mla_user_data_id_init(mla_cli_command_execute_outstream_user_data_id)

mla_size_t mla_private_mla_cli_command_execute_outstream_as_stream_output_write(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_pointer_t out_ptr = mla_user_data_get_pointer(output.userdata, mla_cli_command_execute_outstream_user_data_id);
    mla_cli_command_execute_outstream_t* out = mla_pointer_get_data<mla_cli_command_execute_outstream_t>(out_ptr);

    if (out == nullptr || out->writeBuffer == nullptr || buffer == nullptr) {
        return 0;
    }

    // Call the write function
    out->writeBuffer(out->userdata, mla_r_cast<const mla_char_t *>(buffer), length);

    return length; // Return the number of bytes written
}

mla_size_t mla_private_mla_cli_command_execute_outstream_as_stream_output_available_bytes(mla_stream_output_t& output) {
    (void)output;
    return mla_size_max; // Unknown available bytes
}

mla_stream_output_t mla_cli_command_execute_outstream_as_stream_output(const mla_cli_command_execute_outstream_t &out) {

    mla_user_data_t userdata = mla_user_data_empty();
    mla_pointer_t out_ptr  = mla_platform_pointer_to_managed_pointer(&out);
    mla_user_data_set_pointer(userdata, mla_cli_command_execute_outstream_user_data_id, out_ptr);

    mla_stream_output_t stream_output = {
        userdata,
        mla_private_mla_cli_command_execute_outstream_as_stream_output_write,
        mla_private_mla_cli_command_execute_outstream_as_stream_output_available_bytes
    };

    return stream_output;
}
