//
// Created by copilot on 3/15/26.
//

#ifndef COREOS_MLA_STREAM_DEFLATE_TEST_H
#define COREOS_MLA_STREAM_DEFLATE_TEST_H

#include "../core-os/system/mla_stream.h"
#include "../core-os-test-support/mla_test_executor.h"

void StreamDeflateCompressAndInflateTest() {
    // Write uncompressed data through deflate wrapper into a memory stream
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    mla_string_t test_string = mla_string_const("hello world deflate stream test");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    mla_size_t written = deflate_out.write(deflate_out, 0, test_string_length, (const mla_byte_t*)test_string_data);
    assert_equal(written, test_string_length, "Should write all uncompressed data");

    mla_bool_t finished = mla_stream_output_deflate_finish(deflate_out);
    assert_true(finished, "Deflate finish should succeed");

    // Compressed data should be in the memory stream
    mla_size_t compressed_size = mla_memory_stream_get_size(compressed_storage);
    assert_true(compressed_size > 0, "Compressed data should have been written");

    // Now decompress: read from the compressed memory stream through inflate wrapper
    mla_memory_stream_set_position(compressed_storage, 0);
    mla_stream_input_t inflate_in = mla_stream_input_inflate_wrapper(compressed_storage.input);

    mla_byte_t read_buffer[64] = {0};
    mla_size_t read_bytes = inflate_in.read(inflate_in, 0, test_string_length, read_buffer);

    assert_equal(read_bytes, test_string_length, "Should read back the full decompressed string");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string_data, test_string_length), (mla_test_int32_t)0, "Decompressed data should match original");
}

void StreamDeflateEmptyDataTest() {
    // Compress empty data
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    // Don't write anything, just finish
    mla_bool_t finished = mla_stream_output_deflate_finish(deflate_out);
    assert_true(finished, "Deflate finish on empty stream should succeed");

    // Compressed stream should have some header/trailer bytes
    mla_size_t compressed_size = mla_memory_stream_get_size(compressed_storage);
    assert_true(compressed_size > 0, "Even empty deflate should produce output bytes");

    // Decompress empty data
    mla_memory_stream_set_position(compressed_storage, 0);
    mla_stream_input_t inflate_in = mla_stream_input_inflate_wrapper(compressed_storage.input);

    mla_byte_t read_buffer[16] = {0};
    mla_size_t read_bytes = inflate_in.read(inflate_in, 0, 16, read_buffer);

    assert_equal(read_bytes, (mla_size_t)0, "Decompressing empty data should return 0 bytes");
}

void StreamDeflateLargeDataTest() {
    // Create large repetitive data that compresses well
    mla_size_t data_size = 8192;
    mla_byte_t* original_data = (mla_byte_t*)mla_malloc(data_size);

    if (original_data == nullptr) {
        assert_fail("Memory allocation for test data failed");
        return;
    }

    // Fill with repeating pattern
    for (mla_size_t i = 0; i < data_size; i++) {
        original_data[i] = (mla_byte_t)(i % 256);
    }

    // Compress
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    mla_size_t written = deflate_out.write(deflate_out, 0, data_size, original_data);
    assert_equal(written, data_size, "Should write all large data");

    mla_bool_t finished = mla_stream_output_deflate_finish(deflate_out);
    assert_true(finished, "Deflate finish should succeed for large data");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed_storage);
    assert_true(compressed_size > 0, "Compressed data should exist");

    // Decompress
    mla_memory_stream_set_position(compressed_storage, 0);
    mla_stream_input_t inflate_in = mla_stream_input_inflate_wrapper(compressed_storage.input);

    mla_byte_t* decompressed_data = (mla_byte_t*)mla_malloc(data_size);

    if (decompressed_data == nullptr) {
        mla_free(original_data);
        assert_fail("Memory allocation for decompressed data failed");
        return;
    }

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t chunk = inflate_in.read(inflate_in, total_read, data_size - total_read, decompressed_data);
        if (chunk == 0) break;
        total_read += chunk;
    }

    assert_equal(total_read, data_size, "Should read back all decompressed data");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_data, original_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");

    mla_free(original_data);
    mla_free(decompressed_data);
}

void StreamDeflateCompressionRatioTest() {
    // Highly repetitive data should compress significantly
    mla_size_t data_size = 4096;
    mla_byte_t* original_data = (mla_byte_t*)mla_malloc(data_size);

    if (original_data == nullptr) {
        assert_fail("Memory allocation for test data failed");
        return;
    }

    // Fill with all zeros - maximally compressible
    mla_memset(original_data, 0, data_size);

    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    deflate_out.write(deflate_out, 0, data_size, original_data);
    mla_stream_output_deflate_finish(deflate_out);

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed_storage);
    assert_true(compressed_size < data_size, "Compressed size should be smaller than original for repetitive data");

    mla_free(original_data);
}

void StreamDeflateMultipleWritesTest() {
    // Write data in multiple small chunks, then decompress as one
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    mla_string_t chunk1 = mla_string_const("first chunk ");
    mla_string_t chunk2 = mla_string_const("second chunk ");
    mla_string_t chunk3 = mla_string_const("third chunk");

    mla_size_t len1 = mla_string_length(chunk1);
    mla_size_t len2 = mla_string_length(chunk2);
    mla_size_t len3 = mla_string_length(chunk3);

    mla_size_t written1 = deflate_out.write(deflate_out, 0, len1, (const mla_byte_t*)mla_string_data(chunk1));
    assert_equal(written1, len1, "Should write first chunk");

    mla_size_t written2 = deflate_out.write(deflate_out, 0, len2, (const mla_byte_t*)mla_string_data(chunk2));
    assert_equal(written2, len2, "Should write second chunk");

    mla_size_t written3 = deflate_out.write(deflate_out, 0, len3, (const mla_byte_t*)mla_string_data(chunk3));
    assert_equal(written3, len3, "Should write third chunk");

    mla_bool_t finished = mla_stream_output_deflate_finish(deflate_out);
    assert_true(finished, "Deflate finish should succeed after multiple writes");

    // Decompress
    mla_size_t total_length = len1 + len2 + len3;
    mla_memory_stream_set_position(compressed_storage, 0);
    mla_stream_input_t inflate_in = mla_stream_input_inflate_wrapper(compressed_storage.input);

    mla_byte_t read_buffer[128] = {0};
    mla_size_t total_read = 0;
    while (total_read < total_length) {
        mla_size_t chunk = inflate_in.read(inflate_in, total_read, total_length - total_read, read_buffer);
        if (chunk == 0) break;
        total_read += chunk;
    }

    assert_equal(total_read, total_length, "Should decompress all chunks combined");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, "first chunk second chunk third chunk", total_length), (mla_test_int32_t)0, "Decompressed data should match concatenated chunks");
}

void StreamDeflateSmallChunkReadTest() {
    // Compress data and read it back in very small chunks
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    mla_string_t test_string = mla_string_const("small chunk read test data for inflate");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    deflate_out.write(deflate_out, 0, test_string_length, (const mla_byte_t*)test_string_data);
    mla_stream_output_deflate_finish(deflate_out);

    mla_memory_stream_set_position(compressed_storage, 0);
    mla_stream_input_t inflate_in = mla_stream_input_inflate_wrapper(compressed_storage.input);

    mla_byte_t read_buffer[64] = {0};
    mla_size_t total_read = 0;

    // Read in small 5-byte chunks
    while (total_read < test_string_length) {
        mla_size_t to_read = mla_min((mla_size_t)5, test_string_length - total_read);
        mla_size_t chunk = inflate_in.read(inflate_in, total_read, to_read, read_buffer);
        if (chunk == 0) break;
        total_read += chunk;
    }

    assert_equal(total_read, test_string_length, "Should read all data in small chunks");
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string_data, test_string_length), (mla_test_int32_t)0, "Small chunk reads should reconstruct original data");
}

void StreamDeflateNullInputTest() {
    // Writing null/zero length should return 0
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    mla_size_t written = deflate_out.write(deflate_out, 0, 0, nullptr);
    assert_equal(written, (mla_size_t)0, "Writing null/zero data should return 0");

    mla_stream_output_deflate_finish(deflate_out);
}

void StreamDeflateWithStreamCopyTest() {
    // Test using mla_stream_copy to compress and decompress
    mla_string_t test_string = mla_string_const("stream copy with deflate and inflate");
    mla_size_t test_string_length = mla_string_length(test_string);
    const mla_char_t* test_string_data = mla_string_data(test_string);

    // Source data in memory stream
    mla_memory_stream_t source = mla_memory_stream_empty();
    source.output.write(source.output, 0, test_string_length, (const mla_byte_t*)test_string_data);
    mla_memory_stream_set_position(source, 0);

    // Compress: copy source -> deflate wrapper -> compressed storage
    mla_memory_stream_t compressed_storage = mla_memory_stream_empty();
    mla_stream_output_t deflate_out = mla_stream_output_deflate_wrapper(compressed_storage.output);

    mla_bool_t copy_result = mla_stream_copy(source.input, deflate_out);
    assert_true(copy_result, "Stream copy for compression should succeed");

    mla_bool_t finished = mla_stream_output_deflate_finish(deflate_out);
    assert_true(finished, "Deflate finish should succeed after stream copy");

    // Decompress: inflate wrapper over compressed storage -> read
    mla_memory_stream_set_position(compressed_storage, 0);
    mla_stream_input_t inflate_in = mla_stream_input_inflate_wrapper(compressed_storage.input);

    mla_memory_stream_t decompressed_storage = mla_memory_stream_empty();
    copy_result = mla_stream_copy(inflate_in, decompressed_storage.output);
    assert_true(copy_result, "Stream copy for decompression should succeed");

    assert_equal(mla_memory_stream_get_size(decompressed_storage), test_string_length, "Decompressed size should match original");

    mla_memory_stream_set_position(decompressed_storage, 0);
    mla_byte_t read_buffer[64] = {0};
    decompressed_storage.input.read(decompressed_storage.input, 0, test_string_length, read_buffer);
    assert_equal((mla_test_int32_t)mla_memcmp(read_buffer, test_string_data, test_string_length), (mla_test_int32_t)0, "Stream copy round-trip should preserve data");
}

void RegisterStreamDeflateTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("StreamDeflateCompressAndInflate", test_category, StreamDeflateCompressAndInflateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateEmptyData", test_category, StreamDeflateEmptyDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateLargeData", test_category, StreamDeflateLargeDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressionRatio", test_category, StreamDeflateCompressionRatioTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateMultipleWrites", test_category, StreamDeflateMultipleWritesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateSmallChunkRead", test_category, StreamDeflateSmallChunkReadTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateNullInput", test_category, StreamDeflateNullInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateWithStreamCopy", test_category, StreamDeflateWithStreamCopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif //COREOS_MLA_STREAM_DEFLATE_TEST_H
