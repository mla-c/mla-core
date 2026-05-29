//
// Created by chris on 9/22/2025.
//

#ifndef MLA_NUMBER_TEST_H
#define MLA_NUMBER_TEST_H

#include "../base-lib/core/system/mla_string.h"
#include "../base-lib/core/system/mla_number.h"
#include "../base-lib/core-test-support/mla_test_executor.h"
#include "../base-lib/core-test-support/mla_benchmark_executor.h"

inline void ParseDoubleTest() {

    mla_string_t str = mla_string("123.456");
    mla_double_t value;
    mla_test_bool_t result = mla_parse_double(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, 123.456, "Parsed value should be 123.456");

    str = mla_string("invalid");
    result = mla_parse_double(str, value);
    assert_false(result, "Parsing should fail for invalid string");

    str = mla_string("1.7976931348623157E+308"); // Max double
    result = mla_parse_double(str, value);
    assert_true(result, "Parsing should succeed for max double");
    assert_equal(value, 1.7976931348623157E+308, "Parsed value should be max double");

    str = mla_string("2.2250738585072014E-308"); // Min positive double
    result = mla_parse_double(str, value);
    assert_true(result, "Parsing should succeed for min positive double");
    assert_equal(value, 2.2250738585072014E-308, "Parsed value should be min positive double");

    str = mla_string("-123.456");
    result = mla_parse_double(str, value);
    assert_true(result, "Parsing should succeed for negative number");
    assert_equal(value, -123.456, "Parsed value should be -123.456");
}


inline void ParseFloatTest() {

    mla_string_t str = mla_string("123.456");
    mla_float_t value;
    mla_test_bool_t result = mla_parse_float(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, 123.456f, "Parsed value should be 123.456");

    str = mla_string("invalid");
    result = mla_parse_float(str, value);
    assert_false(result, "Parsing should fail for invalid string");

    str = mla_string("3.4028235E+38"); // Max float
    result = mla_parse_float(str, value);
    assert_true(result, "Parsing should succeed for max float");
    assert_equal(value, 3.4028235E+38f, "Parsed value should be max float");

    str = mla_string("-123.456");
    result = mla_parse_float(str, value);
    assert_true(result, "Parsing should succeed for negative number");
    assert_equal(value, -123.456f, "Parsed value should be -123.456");
}

inline void ParseInt64Test() {

    mla_string_t str = mla_string("9223372036854775807"); // Max int64
    mla_int64_t value;
    mla_test_bool_t result = mla_parse_int64(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, 9223372036854775807LL, "Parsed value should be max int64");

    str = mla_string("-9223372036854775808"); // Min int64
    result = mla_parse_int64(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, -9223372036854775807LL - 1, "Parsed value should be min int64");

    str = mla_string("invalid");
    result = mla_parse_int64(str, value);
    assert_false(result, "Parsing should fail for invalid string");
}

inline void ParseUInt64Test() {

    mla_string_t str = mla_string("18446744073709551615"); // Max uint64
    mla_uint64_t value;
    mla_test_bool_t result = mla_parse_uint64(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, 18446744073709551615ULL, "Parsed value should be max uint64");

    str = mla_string("0");
    result = mla_parse_uint64(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, 0ULL, "Parsed value should be 0");

    str = mla_string("-1");
    result = mla_parse_uint64(str, value);
    assert_false(result, "Parsing should fail for negative number");

    str = mla_string("invalid");
    result = mla_parse_uint64(str, value);
    assert_false(result, "Parsing should fail for invalid string");
}

inline void ParseInt32Test() {

    mla_string_t str = mla_string("2147483647"); // Max int32
    mla_int32_t value;
    mla_test_bool_t result = mla_parse_int32(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, (mla_int32_t)2147483647, "Parsed value should be max int32");

    str = mla_string("-2147483648"); // Min int32
    result = mla_parse_int32(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, (mla_int32_t)-2147483647 - 1, "Parsed value should be min int32");

    str = mla_string("invalid");
    result = mla_parse_int32(str, value);
    assert_false(result, "Parsing should fail for invalid string");
}

inline void ParseUInt32Test() {

    mla_string_t str = mla_string("4294967295"); // Max uint32
    mla_uint32_t value;
    mla_test_bool_t result = mla_parse_uint32(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, (mla_uint32_t)4294967295U, "Parsed value should be max uint32");

    str = mla_string("0");
    result = mla_parse_uint32(str, value);
    assert_true(result, "Parsing should succeed");
    assert_equal(value, (mla_uint32_t)0U, "Parsed value should be 0");

    str = mla_string("-1");
    result = mla_parse_uint32(str, value);
    assert_false(result, "Parsing should fail for negative number");
}

inline void ParseBoolTest() {

    mla_string_t str = mla_string("true");
    mla_bool_t value;
    mla_test_bool_t result = mla_parse_bool(str, value);
    assert_true(result, "Parsing should succeed");
    assert_true(value, "Parsed value should be true");

    str = mla_string("false");
    result = mla_parse_bool(str, value);
    assert_true(result, "Parsing should succeed");
    assert_false(value, "Parsed value should be false");

    str = mla_string("1");
    result = mla_parse_bool(str, value);
    assert_true(result, "Parsing should succeed for '1'");
    assert_true(value, "Parsed value should be true");

    str = mla_string("0");
    result = mla_parse_bool(str, value);
    assert_true(result, "Parsing should succeed for '0'");
    assert_false(value, "Parsed value should be false");

    str = mla_string("invalid");
    result = mla_parse_bool(str, value);
    assert_false(result, "Parsing should fail for invalid string");
}

inline void ParseInt16Test() {
    mla_string_t str = mla_string("32767");
    mla_int16_t value;
    mla_test_bool_t result = mla_parse_int16(str, value);
    assert_true(result, "Parsing should succeed for max int16");
    assert_equal(value, (mla_int16_t)32767, "Parsed value should be 32767");

    str = mla_string("-32768");
    result = mla_parse_int16(str, value);
    assert_true(result, "Parsing should succeed for min int16");
    assert_equal(value, (mla_int16_t)-32768, "Parsed value should be -32768");

    str = mla_string("32768");
    result = mla_parse_int16(str, value);
    assert_false(result, "Parsing should fail for out of range int16");
}

inline void ParseUInt16Test() {
    mla_string_t str = mla_string("65535");
    mla_uint16_t value;
    mla_test_bool_t result = mla_parse_uint16(str, value);
    assert_true(result, "Parsing should succeed for max uint16");
    assert_equal(value, (mla_uint16_t)65535, "Parsed value should be 65535");

    str = mla_string("65536");
    result = mla_parse_uint16(str, value);
    assert_false(result, "Parsing should fail for out of range uint16");
}

inline void ParseInt8Test() {
    mla_string_t str = mla_string("127");
    mla_int8_t value;
    mla_test_bool_t result = mla_parse_int8(str, value);
    assert_true(result, "Parsing should succeed for max int8");
    assert_equal(value, (mla_int8_t)127, "Parsed value should be 127");

    str = mla_string("-128");
    result = mla_parse_int8(str, value);
    assert_true(result, "Parsing should succeed for min int8");
    assert_equal(value, (mla_int8_t)-128, "Parsed value should be -128");

    str = mla_string("128");
    result = mla_parse_int8(str, value);
    assert_false(result, "Parsing should fail for out of range int8");
}

inline void ParseUInt8Test() {
    mla_string_t str = mla_string("255");
    mla_uint8_t value;
    mla_test_bool_t result = mla_parse_uint8(str, value);
    assert_true(result, "Parsing should succeed for max uint8");
    assert_equal(value, (mla_uint8_t)255, "Parsed value should be 255");

    str = mla_string("256");
    result = mla_parse_uint8(str, value);
    assert_false(result, "Parsing should fail for out of range uint8");
}

inline void ParseUInt8HexTest() {
    mla_string_t str = mla_string("0xFF");
    mla_uint8_t value;
    mla_test_bool_t result = mla_parse_uint8_hex(str, value);
    assert_true(result, "Parsing should succeed for 0xFF");
    assert_equal(value, (mla_uint8_t)0xFF, "Parsed value should be 255");

    str = mla_string("100");
    result = mla_parse_uint8_hex(str, value);
    assert_false(result, "Parsing should fail for out of range uint8 hex");
}

inline void ParseUInt16HexTest() {
    mla_string_t str = mla_string("0xFFFF");
    mla_uint16_t value;
    mla_test_bool_t result = mla_parse_uint16_hex(str, value);
    assert_true(result, "Parsing should succeed for 0xFFFF");
    assert_equal(value, (mla_uint16_t)0xFFFF, "Parsed value should be 65535");

    str = mla_string("10000");
    result = mla_parse_uint16_hex(str, value);
    assert_false(result, "Parsing should fail for out of range uint16 hex");
}

inline void ParseUInt32HexTest() {
    mla_string_t str = mla_string("0xFFFFFFFF");
    mla_uint32_t value;
    mla_test_bool_t result = mla_parse_uint32_hex(str, value);
    assert_true(result, "Parsing should succeed for 0xFFFFFFFF");
    assert_equal(value, (mla_uint32_t)0xFFFFFFFFU, "Parsed value should be max uint32");

    str = mla_string("100000000");
    result = mla_parse_uint32_hex(str, value);
    assert_false(result, "Parsing should fail for out of range uint32 hex");
}

inline void ParseUInt64HexTest() {
    mla_string_t str = mla_string("0xFFFFFFFFFFFFFFFF");
    mla_uint64_t value;
    mla_test_bool_t result = mla_parse_uint64_hex(str, value);
    assert_true(result, "Parsing should succeed for 0xFFFFFFFFFFFFFFFF");
    assert_equal(value, 0xFFFFFFFFFFFFFFFFULL, "Parsed value should be max uint64");

    str = mla_string("G");
    result = mla_parse_uint64_hex(str, value);
    assert_false(result, "Parsing should fail for invalid hex character");
}

void RegisterNumberTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("ParseDouble", test_category, ParseDoubleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseFloat", test_category, ParseFloatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseInt64", test_category, ParseInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt64", test_category, ParseUInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseInt32", test_category, ParseInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt32", test_category, ParseUInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseBool", test_category, ParseBoolTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseInt16", test_category, ParseInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt16", test_category, ParseUInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseInt8", test_category, ParseInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt8", test_category, ParseUInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt8Hex", test_category, ParseUInt8HexTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt16Hex", test_category, ParseUInt16HexTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt32Hex", test_category, ParseUInt32HexTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseUInt64Hex", test_category, ParseUInt64HexTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

inline void ParseDoubleBenchmark() {
    mla_string_t str = mla_string_const("123.456");
    mla_double_t value;
    mla_parse_double(str, value);
    mla_test_bench_sink(value);
}

inline void ParseFloatBenchmark() {
    mla_string_t str = mla_string_const("123.456");
    mla_float_t value;
    mla_parse_float(str, value);
    mla_test_bench_sink(value);
}

inline void ParseInt64Benchmark() {
    mla_string_t str = mla_string_const("9223372036854775807");
    mla_int64_t value;
    mla_parse_int64(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt64Benchmark() {
    mla_string_t str = mla_string_const("18446744073709551615");
    mla_uint64_t value;
    mla_parse_uint64(str, value);
    mla_test_bench_sink(value);
}

inline void ParseInt32Benchmark() {
    mla_string_t str = mla_string_const("2147483647");
    mla_int32_t value;
    mla_parse_int32(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt32Benchmark() {
    mla_string_t str = mla_string_const("4294967295");
    mla_uint32_t value;
    mla_parse_uint32(str, value);
    mla_test_bench_sink(value);
}

inline void ParseBoolBenchmark() {
    mla_string_t str = mla_string_const("true");
    mla_bool_t value;
    mla_parse_bool(str, value);
    mla_test_bench_sink(value);
}

inline void ParseInt16Benchmark() {
    mla_string_t str = mla_string_const("32767");
    mla_int16_t value;
    mla_parse_int16(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt16Benchmark() {
    mla_string_t str = mla_string_const("65535");
    mla_uint16_t value;
    mla_parse_uint16(str, value);
    mla_test_bench_sink(value);
}

inline void ParseInt8Benchmark() {
    mla_string_t str = mla_string_const("127");
    mla_int8_t value;
    mla_parse_int8(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt8Benchmark() {
    mla_string_t str = mla_string_const("255");
    mla_uint8_t value;
    mla_parse_uint8(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt8HexBenchmark() {
    mla_string_t str = mla_string_const("0xFF");
    mla_uint8_t value;
    mla_parse_uint8_hex(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt16HexBenchmark() {
    mla_string_t str = mla_string_const("0xFFFF");
    mla_uint16_t value;
    mla_parse_uint16_hex(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt32HexBenchmark() {
    mla_string_t str = mla_string_const("0xFFFFFFFF");
    mla_uint32_t value;
    mla_parse_uint32_hex(str, value);
    mla_test_bench_sink(value);
}

inline void ParseUInt64HexBenchmark() {
    mla_string_t str = mla_string_const("0xFFFFFFFFFFFFFFFF");
    mla_uint64_t value;
    mla_parse_uint64_hex(str, value);
    mla_test_bench_sink(value);
}

void RegisterNumberBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("ParseDouble", benchmark_category, ParseDoubleBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseFloat", benchmark_category, ParseFloatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseInt64", benchmark_category, ParseInt64Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt64", benchmark_category, ParseUInt64Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseInt32", benchmark_category, ParseInt32Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt32", benchmark_category, ParseUInt32Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseBool", benchmark_category, ParseBoolBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseInt16", benchmark_category, ParseInt16Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt16", benchmark_category, ParseUInt16Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseInt8", benchmark_category, ParseInt8Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt8", benchmark_category, ParseUInt8Benchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt8Hex", benchmark_category, ParseUInt8HexBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt16Hex", benchmark_category, ParseUInt16HexBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt32Hex", benchmark_category, ParseUInt32HexBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ParseUInt64Hex", benchmark_category, ParseUInt64HexBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}

#endif