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
    assert_equal((mla_test_int32_t)memcmp(read_buffer, test_string.data, test_string.length), (mla_test_int32_t)0, "Read data should match written data");
}

void MemoryStreamSetPositionAndSeekTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t test_string = mla_string("hello world");
    stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_true(mla_memory_stream_set_position(stream, 6), "Should be able to set position");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)6, "Position should be updated");

    mla_byte_t read_buffer[16] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, 5, read_buffer); // Read from current position
    assert_equal(read_bytes, (mla_size_t)5, "Should read from the new position");
    assert_equal((mla_test_int32_t)memcmp(read_buffer, "world", 5), (mla_test_int32_t)0, "Read data should be 'world'");

    assert_false(mla_memory_stream_set_position(stream, 20), "Should not be able to set position beyond size");
}

void MemoryStreamResetTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t test_string = mla_string("data");
    stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)4, "Size should be 4 before reset");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)4, "Position should be 4 before reset");

    mla_memory_stream_reset(stream);

    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)0, "Size should be 0 after reset");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)0, "Position should be 0 after reset");
}

void MemoryStreamOverwriteTest() {
    mla_memory_stream_t stream = mla_memory_stream_empty();
    mla_string_t initial_string = mla_string("initial data");
    stream.output.write(stream.output, 0, initial_string.length, (const mla_byte_t*)initial_string.data);

    mla_string_t overwrite_string = mla_string("new");
    mla_memory_stream_set_position(stream, 8); // Move position to overwrite part of the data
    mla_size_t written = stream.output.write(stream.output, 0, overwrite_string.length, (const mla_byte_t*)overwrite_string.data);

    assert_equal(written, overwrite_string.length, "Should write overwrite string");
    assert_equal(mla_memory_stream_get_size(stream), initial_string.length, "Size should not change on overwrite within bounds");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t read_buffer[32] = {0};
    stream.input.read(stream.input, 0, mla_memory_stream_get_size(stream), read_buffer);
    assert_equal((mla_test_int32_t)memcmp(read_buffer, "initial newa", 12), (mla_test_int32_t)0, "Data should be partially overwritten");
}

void MemoryStreamAutoGrowTest() {
    mla_memory_stream_t stream = mla_memory_stream(4); // Small initial size
    mla_string_t test_string = mla_string("a long string that will cause reallocation");
    mla_size_t written = stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(written, test_string.length, "Should write the full string");
    assert_equal(mla_memory_stream_get_size(stream), test_string.length, "Stream size should grow to accommodate data");

    mla_memory_stream_set_position(stream, 0); // Reset position to read from start

    mla_byte_t* read_buffer = (mla_byte_t*)malloc(test_string.length);
    mla_size_t read_bytes = stream.input.read(stream.input, 0, test_string.length, read_buffer);

    assert_equal(read_bytes, test_string.length, "Should read back the full string");
    assert_equal((mla_test_int32_t)memcmp(read_buffer, test_string.data, test_string.length), (mla_test_int32_t)0, "Read data should match after auto-grow");
    free(read_buffer);
}

void MemoryStreamNoGrowTest() {

    mla_memory_stream_t stream = mla_memory_stream(8, false);
    mla_string_t test_string = mla_string("0123456789AB");
    mla_size_t written = stream.output.write(stream.output, 0, test_string.length, (const mla_byte_t*)test_string.data);

    assert_equal(written, (mla_size_t)8, "Non-grow stream should stop at capacity");
    assert_equal(mla_memory_stream_get_size(stream), (mla_size_t)8, "Size should equal capacity");
    assert_equal(mla_memory_stream_get_position(stream), (mla_size_t)8, "Position should be at capacity");

    assert_true(mla_memory_stream_set_position(stream, 0), "Reset position to read");
    mla_byte_t read_buffer[16] = {0};
    mla_size_t read_bytes = stream.input.read(stream.input, 0, 8, read_buffer);

    assert_equal(read_bytes, (mla_size_t)8, "Should read only written bytes");
    assert_equal((mla_test_int32_t)memcmp(read_buffer, test_string.data, 8), (mla_test_int32_t)0, "Data should match truncated content");
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
}

#endif //COREOS_MLA_STREAM_TEST_H
