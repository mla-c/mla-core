//
// Created by christian on 11/25/25.
//

#ifndef COREOS_MLA_STREAM_TEST_H
#define COREOS_MLA_STREAM_TEST_H

#include "../core-os/system/mla_stream.h"
#include "../core-os-test-support/mla_test_executor.h"

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
    mla_size_t written = stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(written, test_string.length, "Should write the full string");
    assert_equal(mla_memory_stream_get_size(stream), test_string.length, "Stream size should match written length");
    assert_equal(mla_memory_stream_get_position(stream), test_string.length, "Position should be at the end of written data");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t read_buffer[32] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, test_string.length, read_buffer);

    assert_equal(read_bytes, test_string.length, "Should read the full string back");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string.data, test_string.length), (mla_test_int32_t)0, "Read data should match written data");
}

void MemoryStreamSetPositionAndSeekTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t test_string = mla_string_const("hello world");
    stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

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
    stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)4, "Size should be 4 before reset");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)4, "Position should be 4 before reset");

    mla_memory_stream_reset(stream);

    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)0, "Size should be 0 after reset");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)0, "Position should be 0 after reset");
}

void MemoryStreamOverwriteTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t initial_string = mla_string_const("initial data");
    stream.output.write(stream.output, 0, initial_string.length, (const mla_byte_t*)initial_string.data);

    mla_string_t overwrite_string = mla_string_const("new");
    mla_memory_stream_set_position(stream, 8); // Move position to overwrite part of the data
    mla_size_t written = stream.output.write(stream.output, 0, overwrite_string.length, (const mla_byte_t*)overwrite_string.data);

    assert_equal(written, overwrite_string.length, "Should write overwrite string");
    assert_equal(mla_memory_stream_get_size(stream), initial_string.length, "Size should not change on overwrite within bounds");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t read_buffer[32] = {0};
    stream.input.read(stream.input, 0, mla_memory_stream_get_size(stream), read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "initial newa", 12), (mla_test_int32_t)0, "Data should be partially overwritten");
}

void MemoryStreamAutoGrowTest() {
    mla_memory_stream_t stream = mla_memory_stream(4); // Small initial size
    mla_string_t test_string = mla_string_const("a long string that will cause reallocation");
    mla_size_t written = stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(written, test_string.length, "Should write the full string");
    assert_equal(mla_memory_stream_get_size(stream), test_string.length, "Stream size should grow to accommodate data");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t* read_buffer = (mla_byte_t*)mla_malloc(test_string.length);

    if (read_buffer != nullptr) {
        mla_size_t read_bytes = stream.input.read(stream.input, 0, test_string.length, read_buffer);

        assert_equal(read_bytes, test_string.length, "Should read back the full string");
        assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string.data, test_string.length), (mla_test_int32_t)0, "Read data should match after auto-grow");
        mla_free(read_buffer);
    } else {
        assert_fail("Memory allocation for read buffer failed");
    }

}

void MemoryStreamNoGrowTest() {

    mla_memory_stream_t stream = mla_memory_stream(8, false);
    mla_string_t test_string = mla_string_const("0123456789AB");
    mla_size_t written = stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(written, (mla_size_t)8, "Non-grow stream should stop at capacity");
    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)8, "Size should equal capacity");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)8, "Position should be at capacity");

    assert_true(mla_memory_stream_set_position(stream, 0), "Reset position to read");
    mla_byte_t read_buffer[16] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, 8, read_buffer);

    assert_equal(read_bytes, (mla_size_t)8, "Should read only written bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string.data, 8), (mla_test_int32_t)0, "Data should match truncated content");
}

void StreamInputBufferedWrapperTest() {
    // Create a memory stream as source
    mla_memory_stream_t source = mla_memory_stream_empty();
    mla_string_t test_data = mla_string_const("buffered input test data");
    source.output.write(source.output, 0, test_data.length, (const mla_byte_t*)test_data.data);
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
    mla_size_t read3 = buffered.read(buffered, 15, test_data.length - 15, read_buffer);
    assert_equal(read3, test_data.length - 15, "Should read remaining bytes");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_data.data, test_data.length), (mla_test_int32_t)0, "All data should match");
}

void StreamOutputBufferedWrapperTest() {
    // Create a memory stream as destination
    mla_memory_stream_t dest = mla_memory_stream_empty();

    // Create buffered wrapper with small buffer to test buffering
    mla_stream_output_t buffered = mla_stream_output_buffered_wrapper(dest.output, 16);

    // Write data smaller than buffer (should stay in buffer)
    mla_string_t data1 = mla_string_const("hello");
    mla_size_t written1 = buffered.write(buffered, 0, data1.length, (const mla_byte_t*)data1.data);
    assert_equal(written1, data1.length, "Should write first chunk");
    assert_equal(mla_memory_stream_get_size(dest), (mla_size_t)0, "Data should still be in buffer");

    // Write more data to exceed buffer size
    mla_string_t data2 = mla_string_const(" world test");
    mla_size_t written2 = buffered.write(buffered, 0, data2.length, (const mla_byte_t*)data2.data);
    assert_equal(written2, data2.length, "Should write second chunk");
    assert_true(mla_memory_stream_get_size(dest) > 0, "Buffer should have flushed");

    // Flush remaining data
    mla_stream_output_flush_buffered_wrapper(buffered);

    // Verify all data was written
    mla_memory_stream_set_position(dest, 0);
    mla_byte_t read_buffer[32] = {0};
    mla_size_t total_length = data1.length + data2.length;
    dest.input.read(dest.input, 0, total_length, read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "hello world test", total_length), (mla_test_int32_t)0, "All buffered data should match");
}

void StreamOutputBufferedFlushTest() {
    mla_memory_stream_t dest = mla_memory_stream_empty();
    mla_stream_output_t buffered = mla_stream_output_buffered_wrapper(dest.output, 32);

    // Write data that doesn't fill buffer
    mla_string_t test_data = mla_string_const("partial data");
    buffered.write(buffered, 0, test_data.length, (const mla_byte_t*)test_data.data);

    assert_equal(mla_memory_stream_get_size(dest), (mla_size_t)0, "Data should be buffered before flush");

    // Explicit flush
    mla_stream_output_flush_buffered_wrapper(buffered);

    assert_equal(mla_memory_stream_get_size(dest), test_data.length, "Data should be written after flush");

    // Verify data
    mla_memory_stream_set_position(dest, 0);
    mla_byte_t read_buffer[32] = {0};
    dest.input.read(dest.input, 0, test_data.length, read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_data.data, test_data.length), (mla_test_int32_t)0, "Flushed data should match");
}

void StreamBufferedLargeDataTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_stream_output_t buffered_out = mla_stream_output_buffered_wrapper(stream.output, 64);

    // Write large data that exceeds buffer multiple times
    mla_string_t large_data = mla_string_const("This is a longer string that will exceed the buffer size and trigger multiple flushes automatically");
    mla_size_t written = buffered_out.write(buffered_out, 0, large_data.length, (const mla_byte_t*)large_data.data);
    assert_equal(written, large_data.length, "Should write all data");

    mla_stream_output_flush_buffered_wrapper(buffered_out);

    // Read back with buffered input
    mla_memory_stream_set_position(stream, 0);
    mla_stream_input_t buffered_in = mla_stream_input_buffered_wrapper(stream.input, 32);

    mla_byte_t* read_buffer = (mla_byte_t*)mla_malloc(large_data.length);

    if (read_buffer != nullptr) {
        mla_size_t read_bytes = buffered_in.read(buffered_in, 0, large_data.length, read_buffer);

        assert_equal(read_bytes, large_data.length, "Should read all data back");
        assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, large_data.data, large_data.length), (mla_test_int32_t)0, "Buffered data should match");
        mla_free(read_buffer);
    } else {
        assert_fail("Memory allocation for read buffer failed");
    }


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
}

#endif //COREOS_MLA_STREAM_TEST_H
