#ifndef MLA_SHA1_TEST_H
#define MLA_SHA1_TEST_H

#include "../base-lib/core/hash/mla_sha1.h"
#include "../base-lib/test-support/mla_test_executor.h"
#include "../base-lib/test-support/mla_benchmark_executor.h"

// Test Functions
void Sha1EmptyStringTest() {

    mla_string_t empty = mla_string_empty();
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(empty));
    const mla_byte_t* hash_data = mla_bytes_get_data_readonly(hash);


    if (hash_data != nullptr) {
        // Expected hash for empty string: da39a3ee5e6b4b0d3255bfef95601890afd80709
        assert_equal(hash_data[0], (mla_uint8_t)0xda, "Byte 0 should be 0xda");
        assert_equal(hash_data[1], (mla_uint8_t)0x39, "Byte 1 should be 0x39");
        assert_equal(hash_data[2], (mla_uint8_t)0xa3, "Byte 2 should be 0xa3");
        assert_equal(hash_data[3], (mla_uint8_t)0xee, "Byte 3 should be 0xee");
        assert_equal(hash_data[19], (mla_uint8_t)0x09, "Byte 19 should be 0x09");
    } else {
        assert_fail("SHA1 hash of empty string returned null data");
    }

}

void Sha1SimpleStringTest() {

    mla_string_t str = mla_string_const("abc");
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(str));
    const mla_byte_t* hash_data = mla_bytes_get_data_readonly(hash);

    if (hash_data != nullptr) {
        // Expected hash for "abc": a9993e364706816aba3e25717850c26c9cd0d89d
        assert_equal(hash_data[0], (mla_uint8_t)0xa9, "Byte 0 should be 0xa9");
        assert_equal(hash_data[1], (mla_uint8_t)0x99, "Byte 1 should be 0x99");
        assert_equal(hash_data[2], (mla_uint8_t)0x3e, "Byte 2 should be 0x3e");
        assert_equal(hash_data[19], (mla_uint8_t)0x9d, "Byte 19 should be 0x9d");
    } else {
        assert_fail("SHA1 hash of empty string returned null data");
    }

}

void Sha1LongerStringTest() {
    mla_string_t str = mla_string_const("The quick brown fox jumps over the lazy dog");
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(str));
    const mla_byte_t* hash_data = mla_bytes_get_data_readonly(hash);
    
    // Expected hash: 2fd4e1c67a2d28fced849ee1bb76e7391b93eb12

    if (hash_data != nullptr) {
        assert_equal(hash_data[0], (mla_uint8_t)0x2f, "Byte 0 should be 0x2f");
        assert_equal(hash_data[1], (mla_uint8_t)0xd4, "Byte 1 should be 0xd4");
        assert_equal(hash_data[19], (mla_uint8_t)0x12, "Byte 19 should be 0x12");
    } else {
        assert_fail("SHA1 hash of longer string returned null data");
    }

}


void Sha1LargeInputTest() {

    // Test with input larger than 64 bytes (SHA1 block size)
    mla_string_t large = mla_string_const(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
    );
    // Just verify it completes without crashing and produces a valid hash
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(large));
    const mla_byte_t* hash_data = mla_bytes_get_data_readonly(hash);
    if (hash_data != nullptr) {
        // Check the first and last byte of the hash for validity
        assert_true(hash.size == 20, "SHA1 hash size should be 20 bytes");
        assert_true(hash_data[0] != 0 || hash_data[19] != 0, "SHA1 hash should not be all zeros");
    } else {
        assert_fail("SHA1 hash of large input returned null data");
    }
}

// Registration Functions
void RegisterSha1Tests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("EmptyString", test_category, Sha1EmptyStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SimpleString", test_category, Sha1SimpleStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LongerString", test_category, Sha1LongerStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LargeInput", test_category, Sha1LargeInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}



// Benchmark Functions
void Sha1ShortStringBenchmark() {
    mla_string_t str = mla_string_const("abc");
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(str));
    (void)hash;
}

void Sha1MediumStringBenchmark() {
    mla_string_t str = mla_string_const("The quick brown fox jumps over the lazy dog");
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(str));
    (void)hash;
}


void RegisterSha1Benchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("ShortString", benchmark_category, Sha1ShortStringBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("MediumString", benchmark_category, Sha1MediumStringBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}

#endif
