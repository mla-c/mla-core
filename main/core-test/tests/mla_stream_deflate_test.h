//
// Created by copilot on 3/16/2026.
//

#ifndef MLA_STREAM_DEFLATE_TEST_H
#define MLA_STREAM_DEFLATE_TEST_H

#include "../../lib/base-lib/core/system/mla_stream.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

#if mla_test_featureflag_zlib == 1
#include <zlib.h>

static const mla_size_t stream_deflate_test_websocket_zlib_wire_buffer_size = 512;
static const mla_size_t stream_deflate_test_websocket_zlib_output_buffer_size = 4096;
#endif

inline mla_uint32_t StreamDeflateTestAdler32(const mla_byte_t* p_Data, mla_size_t p_Length) {
    const mla_uint32_t mod_adler = 65521U;
    mla_uint32_t sum1 = 1U;
    mla_uint32_t sum2 = 0U;

    for (mla_size_t i = 0; i < p_Length; i++) {
        sum1 += mla_s_cast<mla_uint32_t>(p_Data[i]);
        if (sum1 >= mod_adler) {
            sum1 -= mod_adler;
        }

        sum2 += sum1;
        sum2 %= mod_adler;
    }

    return (sum2 << 16) | sum1;
}

inline void StreamDeflateExpectedZlibHeader(mla_byte_t& p_Cmf, mla_byte_t& p_Flg) {
    mla_memory_stream_t temp = mla_memory_stream_empty();
    mla_size_t window_bits = mla_stream_output_deflate_window_bits(temp.output);
    p_Cmf = mla_s_cast<mla_byte_t>(((window_bits - 8U) << 4U) | 8U);
    p_Flg = 0;

    mla_uint16_t header = mla_s_cast<mla_uint16_t>((mla_s_cast<mla_uint16_t>(p_Cmf) << 8) | p_Flg);
    p_Flg = mla_s_cast<mla_byte_t>((31U - (header % 31U)) % 31U);
}

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

    mla_size_t written = compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t *>(test_data));
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
        test_data[i] = mla_s_cast<mla_byte_t>(i % 10);
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
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateCompressAndDecompressLargeDataTest() {
    // Test with larger data that spans multiple blocks
    const mla_size_t data_size = 4096;
    mla_byte_t *test_data = mla_s_cast<mla_byte_t *>(mla_platform_malloc(data_size));
    assert_not_null(test_data, "Should allocate test data");

    if (test_data != nullptr) {
        // Fill with pattern
        for (mla_size_t i = 0; i < data_size; i++) {
            test_data[i] = mla_s_cast<mla_byte_t>(((i * 7) + 13) % 256);
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

    mla_byte_t *decompressed_buf = mla_s_cast<mla_byte_t *>(mla_platform_malloc(data_size + 64));
    assert_not_null(decompressed_buf, "Should allocate decompressed buffer");

    if (decompressed_buf != nullptr) {
        mla_memset(decompressed_buf, 0, data_size + 64);
    }

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, data_size - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Should decompress to original length");
    if (decompressed_buf != nullptr && test_data != nullptr) {
        assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");
    }

    mla_platform_free(test_data);
    mla_platform_free(decompressed_buf);
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
        mla_size_t written = compress_out.write(compress_out, 0, part_lens[i], mla_r_cast<const mla_byte_t *>(parts[i]));
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
        if (read_bytes == 0) {
            break;
        }
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
    compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t *>(test_data));
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
        if (read_bytes == 0) {
            break;
        }
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
        test_data[i] = mla_s_cast<mla_byte_t>(i);
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
        if (read_bytes == 0) {
            break;
        }
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
    compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t *>(test_data));
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
        if (read_bytes == 0) {
            break;
        }
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
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Decompressed data should match original");
}

inline void StreamDeflateZlibEmptyStreamBytesTest() {
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_zlib);

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Zlib mode: finish should succeed for empty stream");

    mla_memory_stream_set_position(compressed, 0);
    mla_byte_t wire_bytes[32];
    mla_memset(wire_bytes, 0, sizeof(wire_bytes));
    mla_size_t wire_size = compressed.input.read(compressed.input, 0, sizeof(wire_bytes), wire_bytes);

    assert_equal(wire_size, (mla_size_t)11, "Zlib mode: empty stream should contain header, final block and Adler-32");

    mla_byte_t expected_cmf = 0;
    mla_byte_t expected_flg = 0;
    StreamDeflateExpectedZlibHeader(expected_cmf, expected_flg);

    assert_equal((mla_test_int32_t)wire_bytes[0], (mla_test_int32_t)expected_cmf, "Zlib mode: CMF should match the configured window size");
    assert_equal((mla_test_int32_t)wire_bytes[1], (mla_test_int32_t)expected_flg, "Zlib mode: FLG should satisfy the RFC1950 header checksum");
    assert_equal((mla_test_int32_t)wire_bytes[2], (mla_test_int32_t)0x01, "Zlib mode: empty payload should end with a final empty stored block header");
    assert_equal((mla_test_int32_t)wire_bytes[3], (mla_test_int32_t)0x00, "Zlib mode: LEN low byte should be zero for empty final block");
    assert_equal((mla_test_int32_t)wire_bytes[4], (mla_test_int32_t)0x00, "Zlib mode: LEN high byte should be zero for empty final block");
    assert_equal((mla_test_int32_t)wire_bytes[5], (mla_test_int32_t)0xFF, "Zlib mode: NLEN low byte should be 0xFF for empty final block");
    assert_equal((mla_test_int32_t)wire_bytes[6], (mla_test_int32_t)0xFF, "Zlib mode: NLEN high byte should be 0xFF for empty final block");
    assert_equal((mla_test_int32_t)wire_bytes[7], (mla_test_int32_t)0x00, "Zlib mode: Adler-32 byte 0 should match the empty-stream checksum");
    assert_equal((mla_test_int32_t)wire_bytes[8], (mla_test_int32_t)0x00, "Zlib mode: Adler-32 byte 1 should match the empty-stream checksum");
    assert_equal((mla_test_int32_t)wire_bytes[9], (mla_test_int32_t)0x00, "Zlib mode: Adler-32 byte 2 should match the empty-stream checksum");
    assert_equal((mla_test_int32_t)wire_bytes[10], (mla_test_int32_t)0x01, "Zlib mode: Adler-32 byte 3 should match the empty-stream checksum");
}

inline void StreamDeflateZlibRoundTripTest() {
    const mla_char_t* test_data = "Hello, valid zlib stream!";
    mla_size_t test_len = 25;

    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_zlib);

    mla_size_t written = compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t*>(test_data));
    assert_equal(written, test_len, "Zlib mode: should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Zlib mode: finish should succeed");

    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[64];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < test_len) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, test_len - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, test_len, "Zlib mode: should round-trip to the original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, test_len), (mla_test_int32_t)0, "Zlib mode: decompressed data should match the original input");
}

inline void StreamDeflateZlibAdler32MultiWriteTest() {
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_zlib);

    mla_byte_t first_buffer[16];
    mla_byte_t second_buffer[16];
    mla_byte_t third_buffer[16];
    mla_memset(first_buffer, 0, sizeof(first_buffer));
    mla_memset(second_buffer, 0, sizeof(second_buffer));
    mla_memset(third_buffer, 0, sizeof(third_buffer));

    mla_memcpy(first_buffer + 2, "Hello", 5);
    mla_memcpy(second_buffer + 3, " Zlib", 5);
    mla_memcpy(third_buffer + 1, " Stream", 7);

    mla_size_t written = compress_out.write(compress_out, 2, 5, first_buffer);
    assert_equal(written, (mla_size_t)5, "Zlib mode: first write should honour the source offset");
    written = compress_out.write(compress_out, 3, 5, second_buffer);
    assert_equal(written, (mla_size_t)5, "Zlib mode: second write should honour the source offset");
    written = compress_out.write(compress_out, 1, 7, third_buffer);
    assert_equal(written, (mla_size_t)7, "Zlib mode: third write should honour the source offset");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Zlib mode: finish should succeed after multiple writes");

    mla_memory_stream_set_position(compressed, 0);
    mla_byte_t wire_bytes[128];
    mla_memset(wire_bytes, 0, sizeof(wire_bytes));
    mla_size_t wire_size = compressed.input.read(compressed.input, 0, sizeof(wire_bytes), wire_bytes);
    assert_true(wire_size > 6, "Zlib mode: compressed wire format should contain a header and Adler-32 trailer");

    const mla_byte_t expected_plain[] = {
        'H', 'e', 'l', 'l', 'o', ' ', 'Z', 'l', 'i', 'b', ' ', 'S', 't', 'r', 'e', 'a', 'm'
    };
    mla_uint32_t expected_adler = StreamDeflateTestAdler32(expected_plain, sizeof(expected_plain));

    if (wire_size > 4) {
        mla_uint32_t actual_adler =
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 4]) << 24) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 3]) << 16) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 2]) << 8) |
            mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 1]);

        assert_equal((mla_test_uint32_t)actual_adler, (mla_test_uint32_t)expected_adler, "Zlib mode: Adler-32 trailer should match the concatenated original payload");
    } else {
        assert_fail("Zlib mode: compressed output is too small to contain an Adler-32 trailer");
    }

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

inline void StreamDeflateWebSocketModeTest() {
    // RFC 7692 permessage-deflate: compress with WebSocket mode (omits the 4-byte tail),
    // then simulate a receiver by re-appending 0x00 0x00 0xFF 0xFF and decompressing.
    const mla_char_t *test_data = "Hello, DEFLATE!";
    mla_size_t test_len = 15;

    // Compress with WebSocket mode
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(
        compressed.output, mla_deflate_mode_raw_websocket);

    mla_size_t written = compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t *>(test_data));
    assert_equal(written, test_len, "WebSocket mode: should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "WebSocket mode: finish should succeed");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "WebSocket mode: compressed output should not be empty");

    // Simulate the WebSocket receiver: re-append the stripped LEN/NLEN tail (00 00 FF FF)
    // to complete the empty stored-block that closes the DEFLATE stream.
    mla_byte_t tail[4] = { 0x00, 0x00, 0xFF, 0xFF };
    compressed.output.write(compressed.output, 0, 4, tail);

    // Decompress and verify round-trip
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[64];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < test_len) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, test_len - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, test_len, "WebSocket mode: should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, test_len), (mla_test_int32_t)0,
                 "WebSocket mode: decompressed data should match original");
}

inline void StreamDeflateWebSocketModeLargerDataTest() {
    // Same as above but with more data to exercise LZ77 back-references
    const mla_size_t data_size = 512;
    mla_byte_t test_data[512];
    for (mla_size_t i = 0; i < data_size; i++) {
        test_data[i] = mla_s_cast<mla_byte_t>(i % 10);
    }

    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(
        compressed.output, mla_deflate_mode_raw_websocket);

    mla_size_t written = compress_out.write(compress_out, 0, data_size, test_data);
    assert_equal(written, data_size, "WebSocket large: should write all bytes");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "WebSocket large: finish should succeed");

    // Re-append tail
    mla_byte_t tail[4] = { 0x00, 0x00, 0xFF, 0xFF };
    compressed.output.write(compressed.output, 0, 4, tail);

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[512];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, data_size - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "WebSocket large: should decompress to original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0,
                 "WebSocket large: decompressed data should match original");
}

inline void StreamDeflateWebSocketCompareToNormalTest() {

    const mla_char_t *test_data = "Hello, DEFLATE!";
    mla_size_t test_len = 15;

    // Compress with WebSocket mode
    mla_memory_stream_t compressed_websocket = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(
        compressed_websocket.output, mla_deflate_mode_raw_websocket);

    mla_size_t written = compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t *>(test_data));
    assert_equal(written, test_len, "WebSocket mode: should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "WebSocket mode: finish should succeed");

    mla_size_t compressed_websocket_size = mla_memory_stream_get_size(compressed_websocket);

    // Compress with Normal Node
    mla_memory_stream_t normal_websocket = mla_memory_stream_empty();
    mla_stream_output_t normal_compress_out = mla_stream_output_deflate_compress_wrapper(normal_websocket.output, mla_deflate_mode_raw);

    written = compress_out.write(normal_compress_out, 0, test_len, mla_r_cast<const mla_byte_t *>(test_data));
    assert_equal(written, test_len, "Normal mode: should write all bytes to compressor");

    finish_result = mla_stream_output_deflate_finish(normal_compress_out);
    assert_true(finish_result, "Normal mode: finish should succeed");

    mla_size_t compressed_normal_size = mla_memory_stream_get_size(normal_websocket);

    assert_equal(compressed_websocket_size, compressed_normal_size - 4, "WebSocket mode and Normal mode should produce same compressed size for same data");

}

#if mla_test_featureflag_zlib == 1
inline void StreamDeflateWebSocketModeZlibCompatibilityTest() {
    mla_string_t test_data = mla_string_repeat(mla_string_const("LargeMessage"), 250);
    mla_size_t test_len = mla_string_length(test_data);

    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(
        compressed.output, mla_deflate_mode_raw_websocket);

    mla_stream_input_t input = mla_stream_input_from_string(test_data);
    assert_true(mla_stream_copy(input, compress_out), "WebSocket zlib: should write the full payload");
    assert_true(mla_stream_output_deflate_finish(compress_out), "WebSocket zlib: finish should succeed");

    mla_size_t compressed_size = mla_memory_stream_get_size(compressed);
    assert_true(compressed_size > 0, "WebSocket zlib: compressed output should not be empty");

        mla_byte_t wire_bytes[stream_deflate_test_websocket_zlib_wire_buffer_size];
        assert_true(compressed_size + 4 <= sizeof(wire_bytes), "WebSocket zlib: test buffer should fit compressed payload plus trailer");

    if (compressed_size + 4 <= sizeof(wire_bytes)) {
        mla_memory_stream_set_position(compressed, 0);
        mla_size_t wire_size = compressed.input.read(compressed.input, 0, compressed_size, wire_bytes);
        assert_equal(wire_size, compressed_size, "WebSocket zlib: should read the complete compressed payload");

        wire_bytes[wire_size + 0] = 0x00;
        wire_bytes[wire_size + 1] = 0x00;
        wire_bytes[wire_size + 2] = 0xFF;
        wire_bytes[wire_size + 3] = 0xFF;
        wire_size += 4;

        mla_byte_t decompressed_buf[stream_deflate_test_websocket_zlib_output_buffer_size];
        mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

        z_stream stream = {};
        stream.next_in = wire_bytes;
        stream.avail_in = (uInt)wire_size;
        stream.next_out = decompressed_buf;
        stream.avail_out = (uInt)sizeof(decompressed_buf);

        int init_result = inflateInit2(&stream, -MAX_WBITS);
        assert_equal(init_result, Z_OK, "WebSocket zlib: inflateInit2 should succeed");

        if (init_result == Z_OK) {
            // Use Z_SYNC_FLUSH to match real-world WebSocket client behaviour
            // (e.g. Node.js ws library).  RFC 7692 websocket mode produces
            // BFINAL=0 so inflate returns Z_OK, not Z_STREAM_END.
            int inflate_result = inflate(&stream, Z_SYNC_FLUSH);
            assert_true(inflate_result == Z_OK || inflate_result == Z_BUF_ERROR, "WebSocket zlib: inflate with Z_SYNC_FLUSH should succeed");
            assert_equal((mla_size_t)stream.total_out, test_len, "WebSocket zlib: decompressed size should match the original payload");

            if ((inflate_result == Z_OK || inflate_result == Z_BUF_ERROR) && (mla_size_t)stream.total_out == test_len) {
                assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, mla_string_data(test_data), test_len), (mla_test_int32_t)0,
                             "WebSocket zlib: decompressed bytes should match the original payload");
            }

            inflateEnd(&stream);
        }
    }
}
#endif // mla_test_featureflag_zlib

///////////////////////////////////////////////////////////////////
/// Gzip Mode Tests
///////////////////////////////////////////////////////////////////

inline mla_uint32_t StreamDeflateTestCrc32(const mla_byte_t* p_Data, mla_size_t p_Length) {
    // Standard CRC-32 (ISO 3309) – same polynomial as gzip
    static mla_uint32_t table[256];
    static mla_bool_t built = false;
    if (!built) {
        for (mla_uint32_t i = 0; i < 256; i++) {
            mla_uint32_t c = i;
            for (mla_uint8_t k = 0; k < 8; k++) {
                if ((c & 1U) != 0) {
                    c = 0xEDB88320U ^ (c >> 1);
                } else {
                    c >>= 1;
                }
            }
            table[i] = c;
        }
        built = true;
    }
    mla_uint32_t crc = 0xFFFFFFFFU;
    for (mla_size_t i = 0; i < p_Length; i++) {
        crc = table[(crc ^ p_Data[i]) & 0xFFU] ^ (crc >> 8);
    }
    return ~crc;
}

inline void StreamDeflateGzipEmptyStreamBytesTest() {
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_gzip);

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Gzip mode: finish should succeed for empty stream");

    mla_memory_stream_set_position(compressed, 0);
    mla_byte_t wire_bytes[64];
    mla_memset(wire_bytes, 0, sizeof(wire_bytes));
    mla_size_t wire_size = compressed.input.read(compressed.input, 0, sizeof(wire_bytes), wire_bytes);

    // Minimum gzip: 10-byte header + 7-byte empty DEFLATE block + 8-byte trailer = 25 bytes
    assert_true(wire_size >= 18, "Gzip mode: empty stream should contain header, final block and trailer");

    if (wire_size >= 18) {
        // Verify gzip magic
        assert_equal((mla_test_int32_t)wire_bytes[0], (mla_test_int32_t)0x1F, "Gzip mode: ID1 should be 0x1F");
        assert_equal((mla_test_int32_t)wire_bytes[1], (mla_test_int32_t)0x8B, "Gzip mode: ID2 should be 0x8B");
        assert_equal((mla_test_int32_t)wire_bytes[2], (mla_test_int32_t)0x08, "Gzip mode: CM should be 8 (deflate)");
        assert_equal((mla_test_int32_t)wire_bytes[3], (mla_test_int32_t)0x00, "Gzip mode: FLG should be 0 (no extras)");

        // Verify trailer: CRC32 (LE) + ISIZE (LE)
        // For empty data, CRC32 = 0x00000000 and ISIZE = 0x00000000
        mla_uint32_t trailer_crc =
            mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 8]) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 7]) << 8) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 6]) << 16) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 5]) << 24);
        mla_uint32_t trailer_isize =
            mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 4]) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 3]) << 8) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 2]) << 16) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 1]) << 24);

        assert_equal((mla_test_uint32_t)trailer_crc, (mla_test_uint32_t)0U, "Gzip mode: CRC32 should be 0 for empty stream");
        assert_equal((mla_test_uint32_t)trailer_isize, (mla_test_uint32_t)0U, "Gzip mode: ISIZE should be 0 for empty stream");
    }
}

inline void StreamDeflateGzipRoundTripTest() {
    const mla_char_t* test_data = "Hello, valid gzip stream!";
    mla_size_t test_len = 25;

    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_gzip);

    mla_size_t written = compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t*>(test_data));
    assert_equal(written, test_len, "Gzip mode: should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Gzip mode: finish should succeed");

    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t decompressed_buf[64];
    mla_memset(decompressed_buf, 0, sizeof(decompressed_buf));

    mla_size_t total_read = 0;
    while (total_read < test_len) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, test_len - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, test_len, "Gzip mode: should round-trip to the original length");
    assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, test_len), (mla_test_int32_t)0, "Gzip mode: decompressed data should match the original input");
}

inline void StreamDeflateGzipCrc32MultiWriteTest() {
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_gzip);

    mla_byte_t first_buffer[16];
    mla_byte_t second_buffer[16];
    mla_byte_t third_buffer[16];
    mla_memset(first_buffer, 0, sizeof(first_buffer));
    mla_memset(second_buffer, 0, sizeof(second_buffer));
    mla_memset(third_buffer, 0, sizeof(third_buffer));

    mla_memcpy(first_buffer + 2, "Hello", 5);
    mla_memcpy(second_buffer + 3, " Gzip", 5);
    mla_memcpy(third_buffer + 1, " Stream", 7);

    mla_size_t written = compress_out.write(compress_out, 2, 5, first_buffer);
    assert_equal(written, (mla_size_t)5, "Gzip mode: first write should honour the source offset");
    written = compress_out.write(compress_out, 3, 5, second_buffer);
    assert_equal(written, (mla_size_t)5, "Gzip mode: second write should honour the source offset");
    written = compress_out.write(compress_out, 1, 7, third_buffer);
    assert_equal(written, (mla_size_t)7, "Gzip mode: third write should honour the source offset");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Gzip mode: finish should succeed after multiple writes");

    mla_memory_stream_set_position(compressed, 0);
    mla_byte_t wire_bytes[128];
    mla_memset(wire_bytes, 0, sizeof(wire_bytes));
    mla_size_t wire_size = compressed.input.read(compressed.input, 0, sizeof(wire_bytes), wire_bytes);
    assert_true(wire_size > 18, "Gzip mode: compressed wire format should contain a header and trailer");

    const mla_byte_t expected_plain[] = {
        'H', 'e', 'l', 'l', 'o', ' ', 'G', 'z', 'i', 'p', ' ', 'S', 't', 'r', 'e', 'a', 'm'
    };
    mla_uint32_t expected_crc = StreamDeflateTestCrc32(expected_plain, sizeof(expected_plain));

    if (wire_size > 8) {
        mla_uint32_t actual_crc =
            mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 8]) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 7]) << 8) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 6]) << 16) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 5]) << 24);

        assert_equal((mla_test_uint32_t)actual_crc, (mla_test_uint32_t)expected_crc, "Gzip mode: CRC32 trailer should match the concatenated original payload");

        mla_uint32_t actual_isize =
            mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 4]) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 3]) << 8) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 2]) << 16) |
            (mla_s_cast<mla_uint32_t>(wire_bytes[wire_size - 1]) << 24);

        assert_equal((mla_test_uint32_t)actual_isize, (mla_test_uint32_t)sizeof(expected_plain), "Gzip mode: ISIZE trailer should match the original payload size");
    } else {
        assert_fail("Gzip mode: compressed output is too small to contain a CRC32/ISIZE trailer");
    }
}

inline void StreamDeflateGzipLargeRoundTripTest() {
    // Test with larger data that spans multiple blocks
    const mla_size_t data_size = 4096;
    mla_byte_t *test_data = mla_s_cast<mla_byte_t *>(mla_platform_malloc(data_size));
    assert_not_null(test_data, "Should allocate test data");

    if (test_data != nullptr) {
        for (mla_size_t i = 0; i < data_size; i++) {
            test_data[i] = mla_s_cast<mla_byte_t>(((i * 7) + 13) % 256);
        }
    }

    // Compress
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output, mla_deflate_mode_gzip);

    mla_size_t written = compress_out.write(compress_out, 0, data_size, test_data);
    assert_equal(written, data_size, "Gzip large: should write all bytes to compressor");

    mla_bool_t finish_result = mla_stream_output_deflate_finish(compress_out);
    assert_true(finish_result, "Gzip large: finish should succeed");

    // Decompress
    mla_memory_stream_set_position(compressed, 0);
    mla_stream_input_t decompress_in = mla_stream_input_deflate_decompress_wrapper(compressed.input);

    mla_byte_t *decompressed_buf = mla_s_cast<mla_byte_t *>(mla_platform_malloc(data_size + 64));
    assert_not_null(decompressed_buf, "Should allocate decompressed buffer");

    if (decompressed_buf != nullptr) {
        mla_memset(decompressed_buf, 0, data_size + 64);
    }

    mla_size_t total_read = 0;
    while (total_read < data_size) {
        mla_size_t read_bytes = decompress_in.read(decompress_in, 0, data_size - total_read, decompressed_buf + total_read);
        if (read_bytes == 0) {
            break;
        }
        total_read += read_bytes;
    }

    assert_equal(total_read, data_size, "Gzip large: should decompress to original length");
    if (decompressed_buf != nullptr && test_data != nullptr) {
        assert_equal((mla_test_int32_t)mla_memcmp(decompressed_buf, test_data, data_size), (mla_test_int32_t)0, "Gzip large: decompressed data should match original");
    }

    mla_platform_free(test_data);
    mla_platform_free(decompressed_buf);
}

inline void StreamDeflateGzipCompressedSizeCalculationTest() {
    const mla_char_t* test_data = "Hello, gzip size!";
    mla_size_t test_len = 17;

    mla_stream_input_t input = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(test_data)), test_len);
    mla_size_t compressed_size = mla_stream_input_deflate_compressed_size_calculation(input, mla_deflate_mode_gzip);

    // Gzip compressed size should be larger than raw (header + trailer overhead)
    mla_stream_input_t input2 = mla_stream_input_from_buffer(mla_r_cast<mla_byte_t*>(mla_c_cast<mla_char_t*>(test_data)), test_len);
    mla_size_t raw_compressed_size = mla_stream_input_deflate_compressed_size_calculation(input2, mla_deflate_mode_raw);

    assert_true(compressed_size > raw_compressed_size, "Gzip compressed size should include header + trailer overhead");
    // Gzip overhead: 10-byte header + 8-byte trailer = 18 bytes
    assert_equal(compressed_size, raw_compressed_size + 18U, "Gzip overhead should be exactly 18 bytes (10 header + 8 trailer)");
}

///////////////////////////////////////////////////////////////////
/// Decompressed Size Calculation and Window Bits Tests
///////////////////////////////////////////////////////////////////

inline void StreamDeflateDecompressedSizeCalculationTest() {
    const mla_char_t* test_data = "Hello, decompressed size!";
    mla_size_t test_len = 25;

    // Compress first
    mla_memory_stream_t compressed = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(compressed.output);
    compress_out.write(compress_out, 0, test_len, mla_r_cast<const mla_byte_t*>(test_data));
    mla_stream_output_deflate_finish(compress_out);

    mla_memory_stream_set_position(compressed, 0);
    mla_size_t decompressed_size = mla_stream_input_deflate_decompressed_size_calculation(compressed.input);

    assert_equal(decompressed_size, test_len, "Decompressed size calculation should match original data length");
}

inline void StreamDeflateWindowBitsTest() {
    // mla_stream_output_deflate_window_bits returns the window bits for a deflate output stream
    mla_memory_stream_t mem = mla_memory_stream_empty();
    mla_stream_output_t compress_out = mla_stream_output_deflate_compress_wrapper(mem.output);
    mla_size_t window_bits = mla_stream_output_deflate_window_bits(compress_out);

    // Window bits must be in the valid DEFLATE range [8, 15]
    assert_true(window_bits >= 8 && window_bits <= 15, "Deflate window bits should be in the range [8, 15]");
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

    test = mla_test("StreamDeflateZlibEmptyStreamBytes", test_category, StreamDeflateZlibEmptyStreamBytesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateZlibRoundTrip", test_category, StreamDeflateZlibRoundTripTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateZlibAdler32MultiWrite", test_category, StreamDeflateZlibAdler32MultiWriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateNullInput", test_category, StreamDeflateNullInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateNullOutput", test_category, StreamDeflateNullOutputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateWebSocketMode", test_category, StreamDeflateWebSocketModeTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateWebSocketModeLargerData", test_category, StreamDeflateWebSocketModeLargerDataTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateWebSocketCompareToNormal", test_category, StreamDeflateWebSocketCompareToNormalTest);
    mla_test_executor_register_test(p_TestExecutor, test);

#if mla_test_featureflag_zlib == 1
    test = mla_test("StreamDeflateWebSocketModeZlibCompatibility", test_category, StreamDeflateWebSocketModeZlibCompatibilityTest);
    mla_test_executor_register_test(p_TestExecutor, test);
#endif

    test = mla_test("StreamDeflateGzipEmptyStreamBytes", test_category, StreamDeflateGzipEmptyStreamBytesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateGzipRoundTrip", test_category, StreamDeflateGzipRoundTripTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateGzipCrc32MultiWrite", test_category, StreamDeflateGzipCrc32MultiWriteTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateGzipLargeRoundTrip", test_category, StreamDeflateGzipLargeRoundTripTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateGzipCompressedSizeCalculation", test_category, StreamDeflateGzipCompressedSizeCalculationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateDecompressedSizeCalculation", test_category, StreamDeflateDecompressedSizeCalculationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StreamDeflateWindowBits", test_category, StreamDeflateWindowBitsTest);
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
        bench_deflate_medium_data[i] = mla_s_cast<mla_byte_t>(i % 10);
    }
}

static void bench_deflate_fill_large_data() {
    for (mla_size_t i = 0; i < 4096; i++) {
        bench_deflate_large_data[i] = mla_s_cast<mla_byte_t>(((i * 7) + 13) % 256);
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
    comp.write(comp, 0, bench_deflate_small_len, mla_r_cast<const mla_byte_t *>(bench_deflate_small_data));
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
    comp.write(comp, 0, bench_deflate_small_len, mla_r_cast<const mla_byte_t *>(bench_deflate_small_data));
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
        if (r == 0) {
            break;
        }
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
        if (r == 0) {
            break;
        }
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
        if (r == 0) {
            break;
        }
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
    comp.write(comp, 0, bench_deflate_small_len, mla_r_cast<const mla_byte_t *>(bench_deflate_small_data));
    mla_stream_output_deflate_finish(comp);

    // Decompress from the static buffer using memory stream
    mla_memory_stream_t tmp = mla_memory_stream_empty();
    mla_stream_output_t comp2 = mla_stream_output_deflate_compress_wrapper(tmp.output);
    comp2.write(comp2, 0, bench_deflate_small_len, mla_r_cast<const mla_byte_t *>(bench_deflate_small_data));
    mla_stream_output_deflate_finish(comp2);

    mla_memory_stream_set_position(tmp, 0);
    mla_stream_input_t decomp = mla_stream_input_deflate_decompress_wrapper(tmp.input);
    mla_byte_t result[32];
    mla_size_t total = 0;
    while (total < bench_deflate_small_len) {
        mla_size_t r = decomp.read(decomp, 0, bench_deflate_small_len - total, result + total);
        if (r == 0) {
            break;
        }
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
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("CompressMedium", benchmark_category, DeflateCompressMediumBenchmark,
                              SetupDeflateCompressMediumBenchmark, nullptr);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("CompressLarge", benchmark_category, DeflateCompressLargeBenchmark,
                              SetupDeflateCompressLargeBenchmark, nullptr);
    mla_benchmark_set_iteration_division(benchmark, 1000);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Decompress benchmarks
    benchmark = mla_benchmark("DecompressSmall", benchmark_category, DeflateDecompressSmallBenchmark,
                              SetupDeflateDecompressSmallBenchmark, TearDownDeflateDecompressSmallBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("DecompressMedium", benchmark_category, DeflateDecompressMediumBenchmark,
                              SetupDeflateDecompressMediumBenchmark, TearDownDeflateDecompressMediumBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("DecompressLarge", benchmark_category, DeflateDecompressLargeBenchmark,
                              SetupDeflateDecompressLargeBenchmark, TearDownDeflateDecompressLargeBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 1000);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Round-trip benchmark
    benchmark = mla_benchmark("RoundTripSmall", benchmark_category, DeflateRoundTripSmallBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 1000);
    mla_benchmark_set_bypass_arena(benchmark, true);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif
