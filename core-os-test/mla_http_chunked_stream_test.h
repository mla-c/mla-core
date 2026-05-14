//
// Created by chris on 3/17/2026.
//

#ifndef MLA_MLA_HTTP_CHUNKED_STREAM_TEST_H
#define MLA_MLA_HTTP_CHUNKED_STREAM_TEST_H

#include "../core-os/http/mla_http_chunked_stream.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os/system/mla_stream.h"

static void HttpChunkedStreamOutputTest() {
    mla_memory_stream_t wire = mla_memory_stream(1024, true);
    mla_http_chunked_stream_output_t chunked = mla_http_chunked_stream_output(wire.output);

    const char* txt = "Hello";
    chunked.output.write(chunked.output, 0, 5, (const mla_byte_t*)txt);
    mla_http_chunked_stream_output_finished(chunked);

    mla_byte_t buffer[1024];

    // Verify chunk 1
    mla_memory_stream_set_position(wire, 0);
    mla_size_t read = wire.input.read(wire.input, 0, 1024, buffer);

    // "5\r\nHello\r\n"
    assert_equal(read, (mla_size_t)15, "First chunk length");
    assert_true(buffer[0] == '5', "Chunk size header");
    assert_true(buffer[1] == '\r' && buffer[2] == '\n', "CRLF after size");
    assert_true(mla_memcmp(buffer + 3, "Hello", 5) == 0, "Data content");
    assert_true(buffer[8] == '\r' && buffer[9] == '\n', "CRLF after data");
    assert_true(mla_memcmp(buffer + 10, "0\r\n\r\n", 5) == 0, "End chunk content");
}

static void HttpChunkedStreamInputTest() {
    mla_memory_stream_t wire = mla_memory_stream(1024, true);

    const char* chunk1 = "5\r\nHello\r\n";
    const char* chunk2 = "5\r\nWorld\r\n";
    const char* end = "0\r\n\r\n";

    wire.output.write(wire.output, 0, 10, (const mla_byte_t*)chunk1);
    wire.output.write(wire.output, 0, 10, (const mla_byte_t*)chunk2);
    wire.output.write(wire.output, 0, 5, (const mla_byte_t*)end);

    mla_memory_stream_set_position(wire, 0);

    mla_stream_input_t chunkedIn = mla_http_chunked_stream_input(wire.input, 100);

    mla_byte_t buffer[1024];

    // Read all
    mla_size_t read = chunkedIn.read(chunkedIn, 0, 100, buffer);

    // Expect "HelloWorld" (10 bytes)
    assert_equal(read, (mla_size_t)10, "Should read 10 bytes");
    assert_true(mla_memcmp(buffer, "HelloWorld", 10) == 0, "Content match");
}

static void HttpChunkedStreamDeflateTest() {
    mla_memory_stream_t wire = mla_memory_stream(2048, true);

    mla_http_chunked_stream_output_t chunkedOut = mla_http_chunked_stream_output_deflate(wire.output);
    const char* data = "Hello Deflate World";
    mla_size_t len = 19;

    chunkedOut.output.write(chunkedOut.output, 0, len, (const mla_byte_t*)data);
    mla_http_chunked_stream_output_finished(chunkedOut);

    mla_memory_stream_set_position(wire, 0);

    mla_stream_input_t chunkedIn = mla_http_chunked_stream_input_deflate(wire.input, 100);

    mla_byte_t buffer[2048];
    mla_size_t total = 0;
    while(true) {
        mla_size_t r = chunkedIn.read(chunkedIn, 0, 100, buffer + total);
        if (r == 0) break;
        total += r;
    }

    assert_equal(total, len, "Decompressed length match");
    assert_true(mla_memcmp(buffer, data, len) == 0, "Decompressed content match");
}

static void HttpChunkedStreamRoundtripTest() {
    mla_memory_stream_t wire = mla_memory_stream(1024, true);

    // Setup Output
    mla_http_chunked_stream_output_t chunkedOut = mla_http_chunked_stream_output(wire.output);
    const char* data = "Hello Roundtrip World";
    mla_size_t len = 21;

    // Write data in parts to ensure multiple chunks
    chunkedOut.output.write(chunkedOut.output, 0, 5, (const mla_byte_t*)data); // "Hello"
    chunkedOut.output.write(chunkedOut.output, 0, 11, (const mla_byte_t*)(data + 5)); // " Roundtrip "
    chunkedOut.output.write(chunkedOut.output, 0, 5, (const mla_byte_t*)(data + 16)); // "World"

    mla_http_chunked_stream_output_finished(chunkedOut);

    // Reset wire options
    mla_memory_stream_set_position(wire, 0);

    // Setup Input
    mla_stream_input_t chunkedIn = mla_http_chunked_stream_input(wire.input, 100);

    // Read back
    mla_byte_t buffer[1024];
    mla_size_t readPos = 0;
    while(true) {
        mla_size_t r = chunkedIn.read(chunkedIn, 0, 100, buffer + readPos);
        if (r == 0) break;
        readPos += r;
    }

    assert_equal(readPos, len, "Roundtrip length match");
    assert_true(mla_memcmp(buffer, data, len) == 0, "Roundtrip content match");
}

void RegisterHttpChunkedStreamTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("OutputChunking", test_category, HttpChunkedStreamOutputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InputChunking", test_category, HttpChunkedStreamInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeflateChunking", test_category, HttpChunkedStreamDeflateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Roundtrip", test_category, HttpChunkedStreamRoundtripTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif

