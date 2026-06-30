//
// Created by christian on 9/13/25.
//

#ifndef MLA_STREAM_H
#define MLA_STREAM_H

#include "mla_bytes.h"
#include "../mla_data_types.h"
#include "mla_user_data.h"

struct mla_stream_input_t {
    mla_user_data_t userdata; // Userdata for the read function
    mla_size_t (*read)(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer); // Read function
    mla_size_t (*remaining_bytes)(mla_stream_input_t& input); // Function to get the remaining bytes, can be nullptr. If result is max size_t, means data are there but unknown size

    static mla_stream_input_t init();
};

struct mla_stream_output_t {
    mla_user_data_t userdata;
    mla_size_t (*write)(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
    mla_size_t (*available_bytes)(mla_stream_output_t& output); // Function to get the available bytes, can be nullptr. If result is max size_t, means space is there but unknown size

    static mla_stream_output_t init();
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


// From String not (Owing)
mla_stream_input_t mla_stream_input_from_string(const mla_string_t &string);

mla_string_t mla_string_from_stream(mla_stream_input_t &input, mla_size_t max_length);
mla_bytes_t mla_bytes_from_stream(mla_stream_input_t &input, mla_size_t max_length);

mla_size_t mla_stream_input_read_with_timeout(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer, mla_size_t timeout_ms);

// Writes exactly `length` bytes to `output`, retrying with 10 ms sleeps until all bytes
// are accepted or `timeout_ms` elapses.  Returns the total number of bytes written.
mla_size_t mla_stream_output_write_with_timeout(mla_stream_output_t &output, mla_size_t offset, mla_size_t length, const mla_byte_t *buffer, mla_size_t timeout_ms);

mla_stream_input_t mla_stream_input_timeout_wrapper(mla_stream_input_t &input, mla_size_t timeout_ms);
mla_stream_input_t mla_stream_input_limited_wrapper(mla_stream_input_t &input, mla_size_t size);

mla_stream_input_t mla_stream_input_buffered_wrapper(mla_stream_input_t &input, mla_size_t buffer_size);

typedef mla_size_t (*mla_stream_input_interceptor_read)(mla_stream_input_t& wrapper, mla_stream_input_t& input, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
typedef mla_size_t (*mla_stream_input_interceptor_remaining_bytes)(mla_stream_input_t& wrapper, mla_stream_input_t& input);

mla_stream_input_t mla_stream_input_interceptor_wrapper(mla_stream_input_t &input, mla_stream_input_interceptor_read intercept_read_function, mla_stream_input_interceptor_remaining_bytes intercept_remaining_bytes_function);

mla_stream_output_t mla_stream_output_buffered_wrapper(mla_stream_output_t &output, mla_size_t buffer_size);
mla_bool_t mla_stream_output_flush_buffered_wrapper(const mla_stream_output_t &output);

typedef mla_size_t (*mla_stream_output_interceptor_write)(mla_stream_output_t& wrapper, mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
typedef mla_size_t (*mla_stream_output_interceptor_available_bytes)(mla_stream_output_t& wrapper, mla_stream_output_t& output);

mla_stream_output_t mla_stream_output_interceptor_wrapper(mla_stream_output_t &output, mla_stream_output_interceptor_write intercept_write_function, mla_stream_output_interceptor_available_bytes intercept_available_bytes_function);

///////////////////////////////////////////////////////////////////
/// Deflate Compression/Decompression Wrappers
///////////////////////////////////////////////////////////////////

// Deflate compression mode
// mla_deflate_mode_raw   – standard RFC 1951 terminator (BFINAL=1 empty block).
// mla_deflate_mode_raw_websocket – RFC 7692 permessage-deflate: the compressor emits the
// mla_deflate_mode_zlib - RFC1950 small zlib header + RFC1951 stream + Adler32 checksum
// mla_deflate_mode_gzip - RFC1952 gzip header + RFC1951 stream + CRC32 + ISIZE trailer

enum mla_deflate_mode_t: mla_uint8_t {
    mla_deflate_mode_raw,
    mla_deflate_mode_raw_websocket,
    mla_deflate_mode_zlib,
    mla_deflate_mode_gzip
};

// Deflate compression output wrapper
// Wraps an output stream to compress data written to it using the DEFLATE algorithm.
// Data written to the returned stream is compressed and forwarded to the base output stream.
// Call mla_stream_output_deflate_finish() when done writing to flush remaining compressed data.
mla_stream_output_t mla_stream_output_deflate_compress_wrapper(mla_stream_output_t &output);
mla_stream_output_t mla_stream_output_deflate_compress_wrapper(mla_stream_output_t &output, mla_deflate_mode_t mode);
mla_bool_t mla_stream_output_deflate_finish(mla_stream_output_t &output);

// Deflate decompression input wrapper
// Wraps an input stream to decompress DEFLATE-compressed data read from it.
// The wrapper auto-detects raw RFC 1951, RFC 1950 zlib, and RFC 1952 gzip streams.
// Data read from the returned stream is decompressed from the base input stream.
mla_stream_input_t mla_stream_input_deflate_decompress_wrapper(mla_stream_input_t &input);


mla_size_t mla_stream_input_deflate_decompressed_size_calculation(mla_stream_input_t &input);
mla_size_t mla_stream_input_deflate_compressed_size_calculation(mla_stream_input_t &input, mla_deflate_mode_t mode);

mla_size_t mla_stream_output_deflate_window_bits(mla_stream_output_t &output);

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
/// Size Calculation Output/Input Stream
/// Stream which discards all data written to it but keeps track of the total size of the data that would have been written,
/// useful for calculating the size of serialized data without actually serializing it
/////////////////////////////////////////////////////////////////


mla_stream_output_t mla_stream_output_size_calculation();
mla_size_t mla_stream_output_size_calculation_get_size(const mla_stream_output_t &output);

#endif
