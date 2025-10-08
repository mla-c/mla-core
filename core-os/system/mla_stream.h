//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_STREAM_H
#define COREOS_MLA_STREAM_H

#include "mla_bytes.h"
#include "../mla_data_types.h"
#include "../system/mla_reference.h"

// Define the buffer size for reading from fast streams non blocking streams
// Is better to keep the memory low and read more often than allocating a big buffer
#define mla_stream_fast_read_buffer_size 128

struct mla_stream_input_t {
    mla_callback_userdata userdata; // Userdata for the read function
    mla_size_t (*read)(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer); // Read function
    mla_size_t (*remaining_bytes)(const mla_stream_input_t& input); // Function to get the remaining bytes, can be nullptr
    mla_buffer_reference_t refOwner;
};

struct mla_stream_output_t {
    mla_callback_userdata userdata;
    mla_size_t (*write)(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
    mla_size_t (*available_bytes)(const mla_stream_output_t& output); // Function to get the available bytes, can be nullptr
    mla_buffer_reference_t refOwner;
};

// No-op streams
mla_stream_input_t mla_stream_noop_input();
mla_stream_output_t mla_stream_noop_output();

// Standard IO (Console) streams
mla_stream_input_t mla_stream_input_stdin();
mla_stream_output_t mla_stream_output_stdout();

// Buffer streams (Non-owning))
mla_stream_input_t mla_stream_input_from_buffer(mla_byte_t* buffer, mla_size_t size);
mla_stream_output_t mla_stream_output_to_buffer(mla_byte_t* buffer, mla_size_t size);

// Buffer streams (Owning)
mla_stream_input_t mla_stream_input_from_buffer(mla_size_t size);
mla_stream_output_t mla_stream_output_to_buffer(mla_size_t size);

//////////////////////////////////////////////////////////////////
/// Helpers
//////////////////////////////////////////////////////////////////

mla_bool_t mla_stream_copy(mla_stream_input_t &input, mla_stream_output_t &output);

mla_bool_t mla_stream_output_write_string(mla_stream_output_t &output, const mla_string_t &string);

#endif
