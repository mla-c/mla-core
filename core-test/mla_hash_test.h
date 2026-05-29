#ifndef MLA_HASH_TEST_H
#define MLA_HASH_TEST_H

#include "../framework/core/hash/mla_hash.h"
#include "../framework/core-test-support/mla_test_executor.h"
#include "../framework/core-test-support/mla_benchmark_executor.h"

// Test Functions
void HashStringEmptyTest() {
    const char* empty = "";
    mla_size_t hash = mla_string_hash(empty, 0);
    
    // FNV-1a initial value for empty string
    assert_equal(hash, 2166136261U, "Empty string should return FNV-1a initial hash value");
}

void HashStringSingleCharTest() {
    const char* single = "a";
    mla_size_t hash = mla_string_hash(single, 1);
    
    // Expected hash for "a": (2166136261 ^ 'a') * 16777619
    mla_size_t expected = (2166136261U ^ 'a') * 16777619U;
    assert_equal(hash, expected, "Single character 'a' should produce correct hash");
}

void HashStringSimpleTest() {
    const char* str = "abc";
    mla_size_t hash = mla_string_hash(str, 3);
    
    // Compute expected hash manually
    mla_size_t expected = 2166136261U;
    expected ^= 'a';
    expected *= 16777619U;
    expected ^= 'b';
    expected *= 16777619U;
    expected ^= 'c';
    expected *= 16777619U;
    
    assert_equal(hash, expected, "String 'abc' should produce correct hash");
}

void HashStringLongerTest() {
    const char* str = "The quick brown fox jumps over the lazy dog";
    mla_size_t length = 44;
    mla_size_t hash = mla_string_hash(str, length);
    
    // Just verify it produces a non-initial hash value
    assert_true(hash != 2166136261U, "Long string should produce different hash than initial value");
}

void HashStringNullPointerSafetyTest() {
    // Test null pointer with zero length - should be safe
    mla_size_t hash1 = mla_string_hash(nullptr, 0);
    assert_equal(hash1, 2166136261U, "Null pointer with zero length should return initial hash");
    
    // Test null pointer with non-zero length - should return initial hash as safety fallback
    mla_size_t hash2 = mla_string_hash(nullptr, 5);
    assert_equal(hash2, 2166136261U, "Null pointer with non-zero length should return initial hash safely");
}

void HashStringConsistencyTest() {
    const char* str = "test string";
    mla_size_t length = 11;
    
    mla_size_t hash1 = mla_string_hash(str, length);
    mla_size_t hash2 = mla_string_hash(str, length);
    
    assert_equal(hash1, hash2, "Same input should always produce same hash");
}

void HashStringDistributionTest() {
    const char* str1 = "hello";
    const char* str2 = "world";
    const char* str3 = "Hello"; // Capital H - different from str1
    
    mla_size_t hash1 = mla_string_hash(str1, 5);
    mla_size_t hash2 = mla_string_hash(str2, 5);
    mla_size_t hash3 = mla_string_hash(str3, 5);
    
    assert_true(hash1 != hash2, "Different strings should produce different hashes");
    assert_true(hash1 != hash3, "Case-sensitive strings should produce different hashes");
    assert_true(hash2 != hash3, "All different strings should have unique hashes");
}

void HashStringDifferentLengthsTest() {
    const char* base = "test";
    
    mla_size_t hash1 = mla_string_hash(base, 1); // "t"
    mla_size_t hash2 = mla_string_hash(base, 2); // "te"
    mla_size_t hash3 = mla_string_hash(base, 3); // "tes"
    mla_size_t hash4 = mla_string_hash(base, 4); // "test"
    
    assert_true(hash1 != hash2, "Different length substrings should have different hashes");
    assert_true(hash2 != hash3, "Different length substrings should have different hashes");
    assert_true(hash3 != hash4, "Different length substrings should have different hashes");
}

// Registration Functions
void RegisterHashTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("StringHashEmpty", test_category, HashStringEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashSingleChar", test_category, HashStringSingleCharTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashSimple", test_category, HashStringSimpleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashLonger", test_category, HashStringLongerTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashNullPointerSafety", test_category, HashStringNullPointerSafetyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashConsistency", test_category, HashStringConsistencyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashDistribution", test_category, HashStringDistributionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StringHashDifferentLengths", test_category, HashStringDifferentLengthsTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

// Benchmark Functions
void HashStringShortBenchmark() {
    const char* str = "abc";
    mla_size_t hash = mla_string_hash(str, 3);
    (void)hash;
}

void HashStringMediumBenchmark() {
    const char* str = "The quick brown fox jumps over the lazy dog";
    mla_size_t hash = mla_string_hash(str, 44);
    (void)hash;
}

void HashStringLongBenchmark() {
    const char* str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                      "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
    mla_size_t hash = mla_string_hash(str, 124);
    (void)hash;
}

void RegisterHashBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    mla_benchmark_t benchmark = mla_benchmark("StringHashShort", benchmark_category, HashStringShortBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("StringHashMedium", benchmark_category, HashStringMediumBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("StringHashLong", benchmark_category, HashStringLongBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}

#endif
