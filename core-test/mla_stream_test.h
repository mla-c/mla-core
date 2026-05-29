//
// Created by christian on 11/25/25.
//

#ifndef MLA_STREAM_TEST_H
#define MLA_STREAM_TEST_H

#include "../base-lib/core/system/mla_stream.h"
#include "../base-lib/test-support/mla_test_executor.h"

void MemoryStreamCreateEmptyTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)0, "Empty stream size should be 0");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)0, "Empty stream position should be 0");
}

void MemoryStreamCreateWithInitialSizeTest() {
    mla_memory_stream_t stream = mla_memory_stream(1024);
    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)0, "Initial size should be 0 before writing");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)0, "Initial position should be 0");
}

void MemoryStreamWriteAndReadTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t test_string = mla_string("hello world");

    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    mla_size_t written = stream.output.write(stream.output, 0, test_string_length, (const mla_byte_t*)test_string_data);

    assert_equal(written, test_string_length, "Should write the full string");
    assert_equal(mla_memory_stream_get_size(stream), test_string_length, "Stream size should match written length");
    assert_equal(mla_memory_stream_get_position(stream), test_string_length, "Position should be at the end of written data");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t read_buffer[32] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, test_string_length, read_buffer);

    assert_equal(read_bytes, test_string_length, "Should read the full string back");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string_data, test_string_length), (mla_test_int32_t)0, "Read data should match written data");
}

void MemoryStreamSetPositionAndSeekTest() {

    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t test_string = mla_string_const("hello world");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);


    stream.output.write(stream.output, 0, test_string_length, (const mla_byte_t*)test_string_data);

    assert_true(mla_memory_stream_set_position(stream, 6), "Should be able to set position");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)6, "Position should be updated");

    mla_byte_t read_buffer[16] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, 5, read_buffer); // Read from current position
    assert_equal(read_bytes, (mla_size_t)5, "Should read from the new position");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "world", 5), (mla_test_int32_t)0, "Read data should be 'world'");

    assert_false(mla_memory_stream_set_position(stream, 20), "Should not be able to set position beyond size");
}

void MemoryStreamResetTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t test_string = mla_string_const("data");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    stream.output.write(stream.output, 0, test_string_length, (const mla_byte_t*)test_string_data);

    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)4, "Size should be 4 before reset");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)4, "Position should be 4 before reset");

    mla_memory_stream_reset(stream);

    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)0, "Size should be 0 after reset");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)0, "Position should be 0 after reset");
}

void MemoryStreamOverwriteTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t initial_string = mla_string_const("initial data");
    mla_size_t initial_string_length = mla_string_length(initial_string);
    const mla_char_t* initial_string_data = mla_string_data(initial_string);

    stream.output.write(stream.output, 0, initial_string_length, (const mla_byte_t*)initial_string_data);

    mla_string_t overwrite_string = mla_string_const("new");
    mla_size_t overwrite_string_length = mla_string_length(overwrite_string);
    const mla_char_t* overwrite_string_data = mla_string_data(overwrite_string);
    mla_memory_stream_set_position(stream, 8); // Move position to overwrite part of the data
    mla_size_t written = stream.output.write(stream.output, 0, overwrite_string_length, (const mla_byte_t*)overwrite_string_data);

    assert_equal(written, overwrite_string_length, "Should write overwrite string");
    assert_equal(mla_memory_stream_get_size(stream), initial_string_length, "Size should not change on overwrite within bounds");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t read_buffer[32] = {0};
    stream.input.read(stream.input, 0, mla_memory_stream_get_size(stream), read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "initial newa", 12), (mla_test_int32_t)0, "Data should be partially overwritten");
}

void MemoryStreamAutoGrowTest() {
    mla_memory_stream_t stream = mla_memory_stream(4); // Small initial size
    mla_string_t test_string = mla_string_const("a long string that will cause reallocation");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    mla_size_t written = stream.output.write(stream.output, 0, test_string_length, (const mla_byte_t*)test_string_data);

    assert_equal(written, test_string_length, "Should write the full string");
    assert_equal(mla_memory_stream_get_size(stream), test_string_length, "Stream size should grow to accommodate data");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t* read_buffer = (mla_byte_t*)mla_platform_malloc(test_string_length);

    if (read_buffer != nullptr) {
        mla_size_t read_bytes = stream.input.read(stream.input, 0, test_string_length, read_buffer);

        assert_equal(read_bytes, test_string_length, "Should read back the full string");
        assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string_data, test_string_length), (mla_test_int32_t)0, "Read data should match after auto-grow");
        mla_platform_free(read_buffer);
    } else {
        assert_fail("Memory allocation for read buffer failed");
    }

}

void MemoryStreamNoGrowTest() {

    mla_memory_stream_t stream = mla_memory_stream(8, false);
    mla_string_t test_string = mla_string_const("0123456789AB");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);
    mla_size_t written = stream.output.write(stream.output, 0, test_string_length, (const mla_byte_t*)test_string_data);

    assert_equal(written, (mla_size_t)8, "Non-grow stream should stop at capacity");
    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)8, "Size should equal capacity");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)8, "Position should be at capacity");

    assert_true(mla_memory_stream_set_position(stream, 0), "Reset position to read");
    mla_byte_t read_buffer[16] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, 8, read_buffer);

    assert_equal(read_bytes, (mla_size_t)8, "Should read only written bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string_data, 8), (mla_test_int32_t)0, "Data should match truncated content");
}

void StreamInputBufferedWrapperTest() {
    // Create a memory stream as source
    mla_memory_stream_t source = mla_memory_stream_empty();
    mla_string_t test_data = mla_string_const("buffered input test data");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_data = mla_string_data(test_data);

    source.output.write(source.output, 0, test_data_length, (const mla_byte_t*)test_data_data);
    mla_memory_stream_set_position(source, 0);

    // Create buffered wrapper with small buffer to test multiple reads
    mla_stream_input_t buffered = mla_stream_input_buffered_wrapper(source.input, 8);

    // Read in chunks
    mla_byte_t read_buffer[32] = {0};
    mla_size_t read1 = buffered.read(buffered, 0, 5, read_buffer);
    assert_equal(read1, (mla_size_t)5, "Should read 5 bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "buffe", 5), (mla_test_int32_t)0, "First read data should match");

    // Continue reading - offset 0 because read_buffer + 5 already points to the right location
    mla_size_t read2 = buffered.read(buffered, 5, 10, read_buffer);
    assert_equal(read2, (mla_size_t)10, "Should read 10 more bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "buffered input ", 15), (mla_test_int32_t)0, "Combined read data should match");

    // Read remaining data
    mla_size_t read3 = buffered.read(buffered, 15, test_data_length - 15, read_buffer);
    assert_equal(read3, test_data_length - 15, "Should read remaining bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_data_data, test_data_length), (mla_test_int32_t)0, "All data should match");
}

void StreamOutputBufferedWrapperTest() {
    // Create a memory stream as destination
    mla_memory_stream_t dest = mla_memory_stream_empty();

    // Create buffered wrapper with small buffer to test buffering
    mla_stream_output_t buffered = mla_stream_output_buffered_wrapper(dest.output, 16);

    // Write data smaller than buffer (should stay in buffer)
    mla_string_t data1 = mla_string_const("hello");
    mla_size_t data1_length = mla_string_length(data1);
    const mla_char_t* data1_data = mla_string_data(data1);
    mla_size_t written1 = buffered.write(buffered, 0, data1_length, (const mla_byte_t*)data1_data);
    assert_equal(written1, data1_length, "Should write first chunk");
    assert_equal(mla_memory_stream_get_size(dest), (mla_size_t)0, "Data should still be in buffer");

    // Write more data to exceed buffer size
    mla_string_t data2 = mla_string_const(" world test");
    mla_size_t data2_length = mla_string_length(data2);
    const mla_char_t* data2_data = mla_string_data(data2);
    mla_size_t written2 = buffered.write(buffered, 0, data2_length, (const mla_byte_t*)data2_data);
    assert_equal(written2, data2_length, "Should write second chunk");
    assert_true(mla_memory_stream_get_size(dest) > 0, "Buffer should have flushed");

    // Flush remaining data
    mla_stream_output_flush_buffered_wrapper(buffered);

    // Verify all data was written
    mla_memory_stream_set_position(dest, 0);
    mla_byte_t read_buffer[32] = {0};
    mla_size_t total_length = data1_length + data2_length;
    dest.input.read(dest.input, 0, total_length, read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "hello world test", total_length), (mla_test_int32_t)0, "All buffered data should match");
}

void StreamOutputBufferedFlushTest() {
    mla_memory_stream_t dest = mla_memory_stream_empty();
    mla_stream_output_t buffered = mla_stream_output_buffered_wrapper(dest.output, 32);

    // Write data that doesn't fill buffer
    mla_string_t test_data = mla_string_const("partial data");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_data = mla_string_data(test_data);
    buffered.write(buffered, 0, test_data_length, (const mla_byte_t*)test_data_data);

    assert_equal(mla_memory_stream_get_size(dest), (mla_size_t)0, "Data should be buffered before flush");

    // Explicit flush
    mla_stream_output_flush_buffered_wrapper(buffered);

    assert_equal(mla_memory_stream_get_size(dest), test_data_length, "Data should be written after flush");

    // Verify data
    mla_memory_stream_set_position(dest, 0);
    mla_byte_t read_buffer[32] = {0};
    dest.input.read(dest.input, 0, test_data_length, read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_data_data, test_data_length), (mla_test_int32_t)0, "Flushed data should match");
}

void StreamBufferedLargeDataTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_stream_output_t buffered_out = mla_stream_output_buffered_wrapper(stream.output, 64);

    // Write large data that exceeds buffer multiple times
    mla_string_t large_data = mla_string_const("This is a longer string that will exceed the buffer size and trigger multiple flushes automatically");
    mla_size_t large_data_length = mla_string_length(large_data);
    const mla_char_t* large_data_data = mla_string_data(large_data);

    mla_size_t written = buffered_out.write(buffered_out, 0, large_data_length, (const mla_byte_t*)large_data_data);
    assert_equal(written, large_data_length, "Should write all data");

    mla_stream_output_flush_buffered_wrapper(buffered_out);

    // Read back with buffered input
    mla_memory_stream_set_position(stream, 0);
    mla_stream_input_t buffered_in = mla_stream_input_buffered_wrapper(stream.input, 32);

    mla_byte_t* read_buffer = (mla_byte_t*)mla_platform_malloc(large_data_length);

    if (read_buffer != nullptr) {
        mla_size_t read_bytes = buffered_in.read(buffered_in, 0, large_data_length, read_buffer);

        assert_equal(read_bytes, large_data_length, "Should read all data back");
        assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, large_data_data, large_data_length), (mla_test_int32_t)0, "Buffered data should match");
        mla_platform_free(read_buffer);
    } else {
        assert_fail("Memory allocation for read buffer failed");
    }


}

void StreamOutputSizeCalculationEmptyTest() {
    mla_stream_output_t sizeStream = mla_stream_output_size_calculation();
    assert_equal(mla_stream_output_size_calculation_get_size(sizeStream), (mla_size_t)0, "Initial size should be 0");
}

void StreamOutputSizeCalculationSingleWriteTest() {
    mla_stream_output_t sizeStream = mla_stream_output_size_calculation();

    mla_string_t test_string = mla_string_const("hello");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    mla_size_t written = sizeStream.write(sizeStream, 0, test_string_length, (const mla_byte_t*)test_string_data);

    assert_equal(written, test_string_length, "Should return the written length");
    assert_equal(mla_stream_output_size_calculation_get_size(sizeStream), test_string_length, "Size should match written length");
}

void StreamOutputSizeCalculationMultipleWritesTest() {
    mla_stream_output_t sizeStream = mla_stream_output_size_calculation();

    mla_string_t data1 = mla_string_const("hello");
    mla_string_t data2 = mla_string_const(" world");
    mla_size_t data1_length = mla_string_length(data1);
    mla_size_t data2_length = mla_string_length(data2);

    sizeStream.write(sizeStream, 0, data1_length, (const mla_byte_t*)mla_string_data(data1));
    sizeStream.write(sizeStream, 0, data2_length, (const mla_byte_t*)mla_string_data(data2));

    assert_equal(mla_stream_output_size_calculation_get_size(sizeStream), data1_length + data2_length, "Size should be cumulative of all writes");
}

// --- Interceptor state helpers (file-scope, reset per test) ---
static mla_bool_t g_input_interceptor_read_called = false;
static mla_size_t g_input_interceptor_read_length = 0;
static mla_bool_t g_input_interceptor_remaining_called = false;

static mla_bool_t g_output_interceptor_write_called = false;
static mla_size_t g_output_interceptor_write_length = 0;
static mla_bool_t g_output_interceptor_available_called = false;

// --- Input interceptor callbacks ---
static mla_size_t test_input_intercept_read(mla_stream_input_t& wrapper, mla_stream_input_t& input, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)wrapper; (void)input; (void)offset; (void)buffer;
    g_input_interceptor_read_called = true;
    g_input_interceptor_read_length = length;
    return length; // allow the read to proceed
}

static mla_size_t test_input_intercept_remaining_bytes(mla_stream_input_t& wrapper, mla_stream_input_t& input) {
    (void)wrapper; (void)input;
    g_input_interceptor_remaining_called = true;
    return mla_size_max; // return sentinel to verify callback was used
}

// --- Output interceptor callbacks ---
static mla_size_t test_output_intercept_write(mla_stream_output_t& wrapper, mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)wrapper; (void)output; (void)offset; (void)buffer;
    g_output_interceptor_write_called = true;
    g_output_interceptor_write_length = length;
    return length; // allow the write to proceed
}

static mla_size_t test_output_intercept_available_bytes(mla_stream_output_t& wrapper, mla_stream_output_t& output) {
    (void)wrapper; (void)output;
    g_output_interceptor_available_called = true;
    return 42; // return sentinel to verify callback was used
}

// --- Tests ---

void StreamInputInterceptorWrapperReadCallbackTest() {
    // Setup source memory stream
    mla_memory_stream_t source = mla_memory_stream_empty();
    mla_string_t test_data = mla_string_const("intercept input");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);
    source.output.write(source.output, 0, test_data_length, (const mla_byte_t*)test_data_ptr);
    mla_memory_stream_set_position(source, 0);

    // Reset state
    g_input_interceptor_read_called = false;
    g_input_interceptor_read_length = 0;

    mla_stream_input_t intercepted = mla_stream_input_interceptor_wrapper(source.input, test_input_intercept_read, nullptr);

    mla_byte_t read_buffer[32] = {0};
    mla_size_t read_bytes = intercepted.read(intercepted, 0, test_data_length, read_buffer);

    assert_equal(read_bytes, test_data_length, "Intercepted read should return correct byte count");
    assert_true(g_input_interceptor_read_called, "Read interceptor callback should have been called");
    assert_equal(g_input_interceptor_read_length, test_data_length, "Interceptor should receive correct length");
}

void StreamInputInterceptorWrapperRemainingBytesCallbackTest() {
    mla_memory_stream_t source = mla_memory_stream_empty();
    mla_string_t test_data = mla_string_const("hello");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);
    source.output.write(source.output, 0, test_data_length, (const mla_byte_t*)test_data_ptr);
    mla_memory_stream_set_position(source, 0);

    // Reset state
    g_input_interceptor_remaining_called = false;

    mla_stream_input_t intercepted = mla_stream_input_interceptor_wrapper(source.input, nullptr, test_input_intercept_remaining_bytes);

    if (intercepted.remaining_bytes == nullptr) {
        assert_fail("Remaining bytes callback should be set in interceptor");
    } else {
        mla_size_t remaining = intercepted.remaining_bytes(intercepted);

        assert_true(g_input_interceptor_remaining_called, "Remaining bytes interceptor callback should have been called");
        assert_equal(remaining, mla_size_max, "Interceptor remaining_bytes should return the sentinel value");
    }

}

void StreamInputInterceptorWrapperNullCallbacksPassthroughTest() {
    // When both callbacks are null the wrapper should pass through to the underlying stream
    mla_memory_stream_t source = mla_memory_stream_empty();
    mla_string_t test_data = mla_string_const("passthrough data");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);
    source.output.write(source.output, 0, test_data_length, (const mla_byte_t*)test_data_ptr);
    mla_memory_stream_set_position(source, 0);

    mla_stream_input_t intercepted = mla_stream_input_interceptor_wrapper(source.input, nullptr, nullptr);

    mla_byte_t read_buffer[32] = {0};
    mla_size_t read_bytes = intercepted.read(intercepted, 0, test_data_length, read_buffer);

    assert_equal(read_bytes, test_data_length, "Passthrough interceptor should read correct byte count");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_data_ptr, test_data_length), (mla_test_int32_t)0, "Passthrough interceptor should read correct data");
}

// --- Input interceptor callbacks ---
static mla_size_t test_input_intercept_blocking(mla_stream_input_t& wrapper, mla_stream_input_t& input, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)wrapper; (void)input; (void)offset; (void)length; (void)buffer;
    return 0; // block the read
}

void StreamInputInterceptorWrapperBlockReadTest() {
    // Interceptor returning false should block / indicate no data forwarded
    mla_memory_stream_t source = mla_memory_stream_empty();
    mla_string_t test_data = mla_string_const("blocked data");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);
    source.output.write(source.output, 0, test_data_length, (const mla_byte_t*)test_data_ptr);
    mla_memory_stream_set_position(source, 0);

    mla_stream_input_t intercepted = mla_stream_input_interceptor_wrapper(source.input, test_input_intercept_blocking, nullptr);

    mla_byte_t read_buffer[32] = {0};
    mla_size_t read_bytes = intercepted.read(intercepted, 0, test_data_length, read_buffer);

    assert_equal(read_bytes, (mla_size_t)0, "Blocking interceptor should return 0 bytes read");
}

void StreamOutputInterceptorWrapperWriteCallbackTest() {
    // Setup destination memory stream
    mla_memory_stream_t dest = mla_memory_stream_empty();

    // Reset state
    g_output_interceptor_write_called = false;
    g_output_interceptor_write_length = 0;

    mla_stream_output_t intercepted = mla_stream_output_interceptor_wrapper(dest.output, test_output_intercept_write, nullptr);

    mla_string_t test_data = mla_string_const("intercept output");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);

    mla_size_t written = intercepted.write(intercepted, 0, test_data_length, (const mla_byte_t*)test_data_ptr);

    assert_equal(written, test_data_length, "Intercepted write should return correct byte count");
    assert_true(g_output_interceptor_write_called, "Write interceptor callback should have been called");
    assert_equal(g_output_interceptor_write_length, test_data_length, "Interceptor should receive correct length");

    // Verify data actually reached destination
    mla_memory_stream_set_position(dest, 0);
    mla_byte_t read_buffer[32] = {0};
    dest.input.read(dest.input, 0, test_data_length, read_buffer);
}

void StreamOutputInterceptorWrapperAvailableBytesCallbackTest() {
    mla_memory_stream_t dest = mla_memory_stream_empty();

    // Reset state
    g_output_interceptor_available_called = false;

    mla_stream_output_t intercepted = mla_stream_output_interceptor_wrapper(dest.output, nullptr, test_output_intercept_available_bytes);

    if (intercepted.available_bytes == nullptr) {
        assert_fail("Available bytes callback should be set in interceptor");
    } else {
        mla_size_t available = intercepted.available_bytes(intercepted);

        assert_true(g_output_interceptor_available_called, "Available bytes interceptor callback should have been called");
        assert_equal(available, (mla_size_t)42, "Interceptor available_bytes should return the sentinel value");
    }
}

void StreamOutputInterceptorWrapperNullCallbacksPassthroughTest() {
    mla_memory_stream_t dest = mla_memory_stream_empty();

    mla_stream_output_t intercepted = mla_stream_output_interceptor_wrapper(dest.output, nullptr, nullptr);

    mla_string_t test_data = mla_string_const("passthrough write");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);

    mla_size_t written = intercepted.write(intercepted, 0, test_data_length, (const mla_byte_t*)test_data_ptr);

    assert_equal(written, test_data_length, "Passthrough interceptor should write correct byte count");

    mla_memory_stream_set_position(dest, 0);
    mla_byte_t read_buffer[32] = {0};
    dest.input.read(dest.input, 0, test_data_length, read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_data_ptr, test_data_length), (mla_test_int32_t)0, "Passthrough interceptor should write correct data to destination");
}

static mla_size_t test_output_intercept_write_blocking(mla_stream_output_t& wrapper, mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)wrapper; (void)output; (void)offset; (void)length; (void)buffer;
    return 0; // block the write
}

void StreamOutputInterceptorWrapperBlockWriteTest() {
    mla_memory_stream_t dest = mla_memory_stream_empty();

    mla_stream_output_t intercepted = mla_stream_output_interceptor_wrapper(dest.output, test_output_intercept_write_blocking, nullptr);

    mla_string_t test_data = mla_string_const("should not reach dest");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);

    mla_size_t written = intercepted.write(intercepted, 0, test_data_length, (const mla_byte_t*)test_data_ptr);

    assert_equal(written, (mla_size_t)0, "Blocking interceptor should return 0 bytes written");
    assert_equal(mla_memory_stream_get_size(dest), (mla_size_t)0, "Blocked write should not reach destination");
}

void StreamInterceptorWrapperChainedTest() {
    // Chain: input interceptor -> memory stream -> output interceptor
    mla_memory_stream_t storage = mla_memory_stream_empty();

    // Write via output interceptor
    mla_stream_output_t out_intercepted = mla_stream_output_interceptor_wrapper(storage.output, test_output_intercept_write, nullptr);
    mla_string_t test_data = mla_string_const("chained interceptor data");
    mla_size_t test_data_length = mla_string_length(test_data);
    const mla_char_t* test_data_ptr = mla_string_data(test_data);
    out_intercepted.write(out_intercepted, 0, test_data_length, (const mla_byte_t*)test_data_ptr);

    // Read back via input interceptor
    mla_memory_stream_set_position(storage, 0);
    mla_stream_input_t in_intercepted = mla_stream_input_interceptor_wrapper(storage.input, test_input_intercept_read, nullptr);

    mla_byte_t read_buffer[32] = {0};
    mla_size_t read_bytes = in_intercepted.read(in_intercepted, 0, test_data_length, read_buffer);

    assert_equal(read_bytes, test_data_length, "Chained interceptors should read correct byte count");
}

void RegisterStreamTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("MemoryStreamCreateEmpty", test_category, MemoryStreamCreateEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamCreateWithInitialSize", test_category, MemoryStreamCreateWithInitialSizeTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamWriteAndRead", test_category, MemoryStreamWriteAndReadTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamSetPositionAndSeek", test_category, MemoryStreamSetPositionAndSeekTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamReset", test_category, MemoryStreamResetTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamOverwrite", test_category, MemoryStreamOverwriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamAutoGrow", test_category, MemoryStreamAutoGrowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MemoryStreamNoGrow", test_category, MemoryStreamNoGrowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamInputBufferedWrapper", test_category, StreamInputBufferedWrapperTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputBufferedWrapper", test_category, StreamOutputBufferedWrapperTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputBufferedFlush", test_category, StreamOutputBufferedFlushTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamBufferedLargeData", test_category, StreamBufferedLargeDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputSizeCalculationEmpty", test_category, StreamOutputSizeCalculationEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputSizeCalculationSingleWrite", test_category, StreamOutputSizeCalculationSingleWriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputSizeCalculationMultipleWrites", test_category, StreamOutputSizeCalculationMultipleWritesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamInputInterceptorWrapperReadCallback", test_category, StreamInputInterceptorWrapperReadCallbackTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamInputInterceptorWrapperRemainingBytesCallback", test_category, StreamInputInterceptorWrapperRemainingBytesCallbackTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamInputInterceptorWrapperNullCallbacksPassthrough", test_category, StreamInputInterceptorWrapperNullCallbacksPassthroughTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamInputInterceptorWrapperBlockRead", test_category, StreamInputInterceptorWrapperBlockReadTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputInterceptorWrapperWriteCallback", test_category, StreamOutputInterceptorWrapperWriteCallbackTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputInterceptorWrapperAvailableBytesCallback", test_category, StreamOutputInterceptorWrapperAvailableBytesCallbackTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputInterceptorWrapperNullCallbacksPassthrough", test_category, StreamOutputInterceptorWrapperNullCallbacksPassthroughTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamOutputInterceptorWrapperBlockWrite", test_category, StreamOutputInterceptorWrapperBlockWriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamInterceptorWrapperChained", test_category, StreamInterceptorWrapperChainedTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
