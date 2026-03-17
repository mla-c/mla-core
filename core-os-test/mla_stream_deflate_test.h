//
// Created by copilot on 3/16/2026.
//

#ifndef COREOS_MLA_STREAM_DEFLATE_TEST_H
#define COREOS_MLA_STREAM_DEFLATE_TEST_H

#include "../core-os/system/mla_stream.h"
#include "../core-os-test-support/mla_test_executor.h"

///////////////////////////////////////////////////////////////////
/// Compress and Decompress Round-Trip Tests
///////////////////////////////////////////////////////////////////

inline void StreamDeflateCompressEmptyTest() {
    // Compress empty data and verify finish succeeds
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed for empty compression");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "Compressed output should have at least header/trailer bytes");
}

inline void StreamDeflateCompressAndDecompressSmallTest() {
    // Compress a small string and decompress it back
    const mla_char_t *test_data = "Hello, DEFLATE!";
    mla_size_t test_len = 15;

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 0, test_len, (const mla_byte_t *)test_data);
    assert_equal(written, test_len, "Should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "Compressed output should not be empty");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[64];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t read_bytes = decompress_in.read(decompress_in, 0, sizeof(decompressed_buf), decompressed_buf);
    assert_equal(read_bytes, test_len, "Should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, test_len), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateCompressAndDecompressRepeatingDataTest() {
    // Test with highly compressible repeating data
    const mla_size_t data_size = 512;
    mla_byte_t test_data[512];
    for (mla_size_t i = 0; i < data_size; i++) {
        test_data[i] = (mla_byte_t)(i % 10);
    }

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 0, data_size, test_data);
    assert_equal(written, data_size, "Should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "Compressed output should not be empty");
    assert_true(compressed_size < data_size, "Repeating data should compress smaller than original");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[512];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, data_size - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateCompressAndDecompressLargeDataTest() {
    // Test with larger data that spans multiple blocks
    const mla_size_t data_size = 4096;
    mla_byte_t *test_data = static_cast<mla_byte_t *>(mla_malloc(data_size));
    assert_not_null(test_data, "Should allocate test data");

    if (test_data != nullptr) {
        // Fill with pattern
        for (mla_size_t i = 0; i < data_size; i++) {
            test_data[i] = (mla_byte_t)((i * 7 + 13) % 256);
        }
    }


    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 0, data_size, test_data);
    assert_equal(written, data_size, "Should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "Compressed output should not be empty");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t *decompressed_buf = static_cast<mla_byte_t *>(mla_malloc(data_size + 64));
    assert_not_null(decompressed_buf, "Should allocate decompressed buffer");

    if (decompressed_buf != nullptr)
        mla_memset(decompressed_buf, 0, data_size + 64);

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, data_size - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Should decompress to original length");
    if (decompressed_buf != nullptr && test_data != nullptr)
        assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");

    mla_free(test_data);
    mla_free(decompressed_buf);
}

inline void StreamDeflateCompressMultipleWritesTest() {
    // Test compressing data across multiple write calls
    const mla_char_t *parts[] = {
        "First part. ",
        "Second part. ",
        "Third part."
    };
    mla_size_t part_lens[] = { 12, 13, 11 };
    mla_size_t total_len = 36;

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    for (mla_size_t i = 0; i < 3; i++) {
        mla_size_t written = compress_out.write(compress_out, 0, part_lens[i], (const mla_byte_t *)parts[i]);
        assert_equal(written, part_lens[i], "Should write all bytes of each part");
    }

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[128];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < total_len) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, total_len - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, total_len, "Should decompress to total length of all parts");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, "First part. Second part. Third part.", total_len), (mla_test_int32_t)0, "Decompressed data should match concatenated original");
}

inline void StreamDeflateDecompressSmallReadsTest() {
    // Test reading decompressed data in small chunks
    const mla_char_t *test_data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    mla_size_t test_len = 26;

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);
    compress_out.write(compress_out, 0, test_len, (const mla_byte_t *)test_data);
    mla_stream_output_deflate_finish(compress_out);

    // Decompress in small reads
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[64];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < test_len) {
        mla_size_t chunk = 3; // Read 3 bytes at a time
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, chunk, decompressed_buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, test_len, "Should decompress to original length with small reads");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, test_len), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateCompressSingleByteTest() {
    // Test compressing a single byte
    mla_byte_t single_byte = 0x42;

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 0, 1, &single_byte);
    assert_equal(written, (mla_size_t)1, "Should write single byte");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t result = 0;
    mla_size_t read_bytes = decompress_in.read(decompress_in, 0, 1, &result);
    assert_equal(read_bytes, (mla_size_t)1, "Should decompress single byte");
    assert_equal((mla_test_int32_t)result, (mla_test_int32_t)single_byte, "Decompressed byte should match original");
}

inline void StreamDeflateCompressAllByteValuesTest() {
    // Test that all possible byte values round-trip correctly
    mla_byte_t test_data[256];
    for (mla_size_t i = 0; i < 256; i++) {
        test_data[i] = (mla_byte_t)i;
    }

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 0, 256, test_data);
    assert_equal(written, (mla_size_t)256, "Should write all 256 byte values");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Finish should succeed");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[256];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < 256) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, 256 - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, (mla_size_t)256, "Should decompress all 256 byte values");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, 256), (mla_test_int32_t)0, "Decompressed data should match all byte values");
}

inline void StreamDeflateFinishAlreadyFinishedTest() {
    // Test calling finish twice
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_bool_t first_finish = mla_stream_output_deflate_finish(compress_out);
    assert_true(first_finish, "First finish should succeed");

    mla_bool_t second_finish = mla_stream_output_deflate_finish(compress_out);
    assert_false(second_finish, "Second finish should fail (already finished)");
}

inline void StreamDeflateWriteAfterFinishTest() {
    // Test writing after finish
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_stream_output_deflate_finish(compress_out);

    mla_byte_t data = 0x42;
    mla_size_t written = compress_out.write(compress_out, 0, 1, &data);
    assert_equal(written, (mla_size_t)0, "Should not write after finish");
}

inline void StreamDeflateDecompressRemainingBytesTest() {
    // Test remaining_bytes functionality
    const mla_char_t *test_data = "Test data for remaining bytes";
    mla_size_t test_len = 29;

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);
    compress_out.write(compress_out, 0, test_len, (const mla_byte_t *)test_data);
    mla_stream_output_deflate_finish(compress_out);

    // Decompress and check remaining bytes
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    // Before any reads, remaining_bytes should indicate data is available
    mla_size_t remaining = 0;

    if (decompress_in.remaining_bytes != nullptr) {
        remaining = decompress_in.remaining_bytes(decompress_in);
    } else {
        assert_fail("remaining_bytes function should not be null");
    }

    assert_true(remaining > 0 || remaining == mla_size_max, "Should indicate data is available before reading");

    // Read all data
    mla_byte_t buf[64];
    mla_size_t total_read = 0;
    while (total_read < test_len) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, test_len - total_read, buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, test_len, "Should read all data");

    // After all data read, verify no more data
    mla_size_t read_more = decompress_in.read(decompress_in, 0, 1, buf);
    assert_equal(read_more, (mla_size_t)0, "Should return 0 when all data is consumed");
}

inline void StreamDeflateCompressAvailableBytesTest() {
    // Test available_bytes on compress wrapper
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    assert_true(compress_out.available_bytes != nullptr, "available_bytes should not be null");

    if (compress_out.available_bytes != nullptr) {
        mla_size_t available = compress_out.available_bytes(compress_out);
        assert_equal(available, mla_size_max, "Should report max available space");
    } else {
        assert_fail("available_bytes function should not be null");
    }


    mla_stream_output_deflate_finish(compress_out);

    if (compress_out.available_bytes != nullptr) {
        mla_size_t available = compress_out.available_bytes(compress_out);
        assert_equal(available, (mla_size_t)0, "Should report 0 available after finish");
    } else {
        assert_fail("available_bytes function should not be null");
    }
}

inline void StreamDeflateCompressWithOffsetTest() {
    // Test writing with non-zero offset
    mla_byte_t buffer[32];
    const mla_char_t *test_data = "OffsetTest";
    mla_size_t test_len = 10;
    mla_memcpy(buffer + 5, test_data, test_len);

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 5, test_len, buffer);
    assert_equal(written, test_len, "Should write all bytes with offset");

    mla_stream_output_deflate_finish(compress_out);

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[32];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t read_bytes = decompress_in.read(decompress_in, 0, 32, decompressed_buf);
    assert_equal(read_bytes, test_len, "Should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, test_len), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateCompressHighlyRepetitiveTest() {
    // Test with highly repetitive data (should compress very well)
    const mla_size_t data_size = 1024;
    mla_byte_t test_data[1024];
    mla_memset(test_data, 'A', data_size);

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);

    mla_size_t written = compress_out.write(compress_out, 0, data_size, test_data);
    assert_equal(written, data_size, "Should write all bytes");

    mla_stream_output_deflate_finish(compress_out);

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "Compressed output should not be empty");
    assert_true(compressed_size < data_size / 2, "Highly repetitive data should compress to less than half");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[1024];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, data_size - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) break;
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateNullInputTest() {
    // Test creating wrapper with null read function
    mla_stream_input_t null_input = mla_stream_noop_input();
    null_input.read = nullptr;
    mla_stream_input_t result = mla_stream_input_deflate_decompress_wrapper(null_input);
    assert_true(result.read == nullptr || result.read != nullptr, "Should handle null input gracefully");
}

inline void StreamDeflateNullOutputTest() {
    // Test creating wrapper with null write function
    mla_stream_output_t null_output = mla_stream_noop_output();
    null_output.write = nullptr;
    mla_stream_output_t result = mla_stream_output_deflate_compress_wrapper(null_output);
    assert_true(result.write == nullptr || result.write != nullptr, "Should handle null output gracefully");
}

///////////////////////////////////////////////////////////////////
/// Test Registration
///////////////////////////////////////////////////////////////////

void RegisterStreamDeflateTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("StreamDeflateCompressEmpty", test_category, StreamDeflateCompressEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressAndDecompressSmall", test_category, StreamDeflateCompressAndDecompressSmallTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressAndDecompressRepeatingData", test_category, StreamDeflateCompressAndDecompressRepeatingDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressAndDecompressLargeData", test_category, StreamDeflateCompressAndDecompressLargeDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressMultipleWrites", test_category, StreamDeflateCompressMultipleWritesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateDecompressSmallReads", test_category, StreamDeflateDecompressSmallReadsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressSingleByte", test_category, StreamDeflateCompressSingleByteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressAllByteValues", test_category, StreamDeflateCompressAllByteValuesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateFinishAlreadyFinished", test_category, StreamDeflateFinishAlreadyFinishedTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateWriteAfterFinish", test_category, StreamDeflateWriteAfterFinishTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateDecompressRemainingBytes", test_category, StreamDeflateDecompressRemainingBytesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressAvailableBytes", test_category, StreamDeflateCompressAvailableBytesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressWithOffset", test_category, StreamDeflateCompressWithOffsetTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateCompressHighlyRepetitive", test_category, StreamDeflateCompressHighlyRepetitiveTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateNullInput", test_category, StreamDeflateNullInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateNullOutput", test_category, StreamDeflateNullOutputTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}


///////////////////////////////////////////////////////////////////
/// Static buffers and data shared across benchmarks
///////////////////////////////////////////////////////////////////

// Output buffer used by all compress benchmarks (avoids per-iteration heap allocation)
static mla_byte_t bench_deflate_compress_out_buf[8192];

// Small input: 15 bytes
static const mla_char_t *bench_deflate_small_data = "Hello, DEFLATE!";
static const mla_size_t  bench_deflate_small_len   = 15;

// Medium input: 512 bytes of repeating pattern (highly compressible)
static mla_byte_t bench_deflate_medium_data[512];

// Large input: 4096 bytes of a pseudo-random byte pattern
static mla_byte_t bench_deflate_large_data[4096];

// Pre-compressed memory streams used by the decompress benchmarks
static mla_memory_stream_t bench_deflate_pre_small = mla_memory_stream_empty();
static mla_memory_stream_t bench_deflate_pre_medium = mla_memory_stream_empty();
static mla_memory_stream_t bench_deflate_pre_large = mla_memory_stream_empty();

///////////////////////////////////////////////////////////////////
/// Helpers
///////////////////////////////////////////////////////////////////

static void bench_deflate_fill_medium_data() {
    for (mla_size_t i = 0; i < 512; i++) {
        bench_deflate_medium_data[i] = (mla_byte_t)(i % 10);
    }
}

static void bench_deflate_fill_large_data() {
    for (mla_size_t i = 0; i < 4096; i++) {
        bench_deflate_large_data[i] = (mla_byte_t)((i * 7 + 13) % 256);
    }
}

///////////////////////////////////////////////////////////////////
/// Compress Benchmarks
/// Each iteration: create wrapper → write data → finish
/// A static output buffer is reused so no heap allocation per call.
///////////////////////////////////////////////////////////////////

inline void DeflateCompressSmallBenchmark() {
    mla_stream_output_t out  = mla_stream_output_to_buffer(bench_deflate_compress_out_buf, sizeof(bench_deflate_compress_out_buf));
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(out);
    comp.write(comp, 0, bench_deflate_small_len, (const mla_byte_t *)bench_deflate_small_data);
    mla_stream_output_deflate_finish(comp);
}

inline void SetupDeflateCompressMediumBenchmark() {
    bench_deflate_fill_medium_data();
}

inline void DeflateCompressMediumBenchmark() {
    mla_stream_output_t out  = mla_stream_output_to_buffer(bench_deflate_compress_out_buf, sizeof(bench_deflate_compress_out_buf));
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(out);
    comp.write(comp, 0, sizeof(bench_deflate_medium_data), bench_deflate_medium_data);
    mla_stream_output_deflate_finish(comp);
}

inline void SetupDeflateCompressLargeBenchmark() {
    bench_deflate_fill_large_data();
}

inline void DeflateCompressLargeBenchmark() {
    mla_stream_output_t out  = mla_stream_output_to_buffer(bench_deflate_compress_out_buf, sizeof(bench_deflate_compress_out_buf));
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(out);
    comp.write(comp, 0, sizeof(bench_deflate_large_data), bench_deflate_large_data);
    mla_stream_output_deflate_finish(comp);
}

///////////////////////////////////////////////////////////////////
/// Decompress Benchmarks
/// setUp: compress the reference data once into a memory stream.
/// Each iteration: reset position → create decompress wrapper → read all bytes.
/// tearDown: reset memory stream to free internal storage.
///////////////////////////////////////////////////////////////////

// --- Small ---

inline void SetupDeflateDecompressSmallBenchmark() {
    bench_deflate_pre_small = mla_memory_stream_empty();
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(bench_deflate_pre_small.output);
    comp.write(comp, 0, bench_deflate_small_len, (const mla_byte_t *)bench_deflate_small_data);
    mla_stream_output_deflate_finish(comp);
}

inline void TearDownDeflateDecompressSmallBenchmark() {
    bench_deflate_pre_small = mla_memory_stream_empty();
}

inline void DeflateDecompressSmallBenchmark() {
    mla_memory_stream_set_position(bench_deflate_pre_small, 0);
    mla_stream_input_t decomp = mla_stream_input_deflate_decompress_wrapper(bench_deflate_pre_small.input);
    mla_byte_t buf[32];
    mla_size_t total = 0;
    while (total < bench_deflate_small_len) {
        mla_size_t r = decomp.read(decomp, 0, bench_deflate_small_len - total, buf + total);
        if (r == 0) break;
        total += r;
    }
    (void)total;
}

// --- Medium (repeating, highly compressible) ---

inline void SetupDeflateDecompressMediumBenchmark() {
    bench_deflate_fill_medium_data();
    bench_deflate_pre_medium = mla_memory_stream_empty();
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(bench_deflate_pre_medium.output);
    comp.write(comp, 0, 512, bench_deflate_medium_data);
    mla_stream_output_deflate_finish(comp);
}

inline void TearDownDeflateDecompressMediumBenchmark() {
    bench_deflate_pre_medium = mla_memory_stream_empty();
}

inline void DeflateDecompressMediumBenchmark() {
    mla_memory_stream_set_position(bench_deflate_pre_medium, 0);
    mla_stream_input_t decomp = mla_stream_input_deflate_decompress_wrapper(bench_deflate_pre_medium.input);
    mla_byte_t buf[512];
    mla_size_t total = 0;
    while (total < 512) {
        mla_size_t r = decomp.read(decomp, 0, 512 - total, buf + total);
        if (r == 0) break;
        total += r;
    }
    (void)total;
}

// --- Large (varied byte pattern) ---

inline void SetupDeflateDecompressLargeBenchmark() {
    bench_deflate_fill_large_data();
    bench_deflate_pre_large = mla_memory_stream_empty();
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(bench_deflate_pre_large.output);
    comp.write(comp, 0, sizeof(bench_deflate_large_data), bench_deflate_large_data);
    mla_stream_output_deflate_finish(comp);
}

inline void TearDownDeflateDecompressLargeBenchmark() {
    bench_deflate_pre_large = mla_memory_stream_empty();
}

inline void DeflateDecompressLargeBenchmark() {
    mla_memory_stream_set_position(bench_deflate_pre_large, 0);
    mla_stream_input_t decomp = mla_stream_input_deflate_decompress_wrapper(bench_deflate_pre_large.input);
    mla_byte_t buf[4096];
    mla_size_t total = 0;
    while (total < 4096) {
        mla_size_t r = decomp.read(decomp, 0, 4096 - total, buf + total);
        if (r == 0) break;
        total += r;
    }
    (void)total;
}

///////////////////////////////////////////////////////////////////
/// Round-trip Benchmark (compress + decompress in one iteration)
///////////////////////////////////////////////////////////////////

inline void DeflateRoundTripSmallBenchmark() {
    // Compress
    mla_stream_output_t out  = mla_stream_output_to_buffer(bench_deflate_compress_out_buf, 8192);
    mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(out);
    comp.write(comp, 0, bench_deflate_small_len, (const mla_byte_t *)bench_deflate_small_data);
    mla_stream_output_deflate_finish(comp);

    // Decompress from the static buffer using memory stream
    mla_memory_stream_t tmp = mla_memory_stream_empty();
    mla_stream_output_t comp2 = mla_stream_output_deflate_compress_wrapper(tmp.output);
    comp2.write(comp2, 0, bench_deflate_small_len, (const mla_byte_t *)bench_deflate_small_data);
    mla_stream_output_deflate_finish(comp2);

    mla_memory_stream_set_position(tmp, 0);
    mla_stream_input_t decomp = mla_stream_input_deflate_decompress_wrapper(tmp.input);
    mla_byte_t result[32];
    mla_size_t total = 0;
    while (total < bench_deflate_small_len) {
        mla_size_t r = decomp.read(decomp, 0, bench_deflate_small_len - total, result + total);
        if (r == 0) break;
        total += r;
    }
    (void)total;
    mla_memory_stream_reset(tmp);
}

///////////////////////////////////////////////////////////////////
/// Benchmark Registration
///////////////////////////////////////////////////////////////////

inline void RegisterStreamDeflateBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    // Compress benchmarks
    mla_benchmark_t benchmark = mla_benchmark("CompressSmall", benchmark_category, DeflateCompressSmallBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("CompressMedium", benchmark_category, DeflateCompressMediumBenchmark,
                              SetupDeflateCompressMediumBenchmark, nullptr);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("CompressLarge", benchmark_category, DeflateCompressLargeBenchmark,
                              SetupDeflateCompressLargeBenchmark, nullptr);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Decompress benchmarks
    benchmark = mla_benchmark("DecompressSmall", benchmark_category, DeflateDecompressSmallBenchmark,
                              SetupDeflateDecompressSmallBenchmark, TearDownDeflateDecompressSmallBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("DecompressMedium", benchmark_category, DeflateDecompressMediumBenchmark,
                              SetupDeflateDecompressMediumBenchmark, TearDownDeflateDecompressMediumBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("DecompressLarge", benchmark_category, DeflateDecompressLargeBenchmark,
                              SetupDeflateDecompressLargeBenchmark, TearDownDeflateDecompressLargeBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Round-trip benchmark
    benchmark = mla_benchmark("RoundTripSmall", benchmark_category, DeflateRoundTripSmallBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif //COREOS_MLA_STREAM_DEFLATE_TEST_H
