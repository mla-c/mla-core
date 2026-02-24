//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_STREAM_H
#define COREOS_MLA_STREAM_H

#include "mla_bytes.h"
#include "../mla_data_types.h"
#include "mla_user_data.h"

// Define the buffer size for reading from fast streams non blocking streams
// Is better to keep the memory low and read more often than allocating a big buffer
#define mla_stream_fast_read_buffer_size 128

struct mla_stream_input_t {
    mla_user_data_t userdata; // Userdata for the read function
    mla_size_t (*read)(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer); // Read function
    mla_size_t (*remaining_bytes)(mla_stream_input_t& input); // Function to get the remaining bytes, can be nullptr. If result is max size_t, means data are there but unknown size
};

struct mla_stream_output_t {
    mla_user_data_t userdata;
    mla_size_t (*write)(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
    mla_size_t (*available_bytes)(mla_stream_output_t& output); // Function to get the available bytes, can be nullptr. If result is max size_t, means space is there but unknown size
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

///////////////////////////////////////////////////////////////////
/// Wrapper and Helpers
///////////////////////////////////////////////////////////////////

mla_size_t mla_stream_input_read_with_timeout(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer, mla_int32_t timeout_ms);

mla_stream_input_t mla_stream_input_timeout_wrapper(mla_stream_input_t &input, mla_int32_t timeout_ms);
mla_stream_input_t mla_stream_input_limited_wrapper(mla_stream_input_t &input, mla_size_t size);

mla_stream_input_t mla_stream_input_buffered_wrapper(mla_stream_input_t &input, mla_size_t buffer_size);

mla_stream_output_t mla_stream_output_buffered_wrapper(mla_stream_output_t &output, mla_size_t buffer_size);
void mla_stream_output_flush_buffered_wrapper(const mla_stream_output_t &output);


//////////////////////////////////////////////////////////////////
// Memory Stream
/////////////////////////////////////////////////////////////////
struct mla_memory_stream_t {
    mla_stream_input_t input;
    mla_stream_output_t output;
};

mla_memory_stream_t mla_memory_stream_invalid();
mla_memory_stream_t mla_memory_stream_empty();
mla_memory_stream_t mla_memory_stream(mla_size_t initial_size, mla_bool_t can_grow);
mla_memory_stream_t mla_memory_stream(mla_size_t initial_size);
mla_size_t mla_memory_stream_get_size(const mla_memory_stream_t &memoryStream);
mla_size_t mla_memory_stream_get_position(const mla_memory_stream_t &memoryStream);
mla_bool_t mla_memory_stream_set_position(mla_memory_stream_t &memoryStream, mla_size_t position);
void mla_memory_stream_reset(mla_memory_stream_t &memoryStream);

//////////////////////////////////////////////////////////////////
/// Size Calculation Output Stream
/// Stream which discards all data written to it but keeps track of the total size of the data that would have been written,
/// useful for calculating the size of serialized data without actually serializing it
/////////////////////////////////////////////////////////////////


mla_stream_output_t mla_stream_output_size_calculation();
mla_size_t mla_stream_output_size_calculation_get_size(const mla_stream_output_t &output);

#endif
