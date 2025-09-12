//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_NATIVE_STRING_TEST_H
#define COREOS_NATIVE_STRING_TEST_H

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
#include "string"
#endif

#include "cstring"
#include "../core-os-test-support/mla_benchmark_executor.h"

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
void StdStringConcatBenchmark() {

    std::string str1("Hello, ");
    std::string str2("World!");
    std::string str3(" This is a test of concatenation.");
    std::string result = str1 + str2 + str3;
    // Just to ensure the operation is performed
    mla_test_int32_t length = (mla_test_int32_t)result.length();
    (void)length; // Prevent unused variable warning
}

void StdStringContainsBenchmark() {

    std::string str("Hello, World! This is a test string for benchmarking.");
    std::string subString("for");

    if (str.find(subString) == std::string::npos) {
        static_assert(true, "Substring not found in StdStringContainsBenchmark");
    }
}

void StdStringIndexOfBenchmark() {

    std::string str("Hello, World! This is a test string for benchmarking.");
    std::string subString("for");

    size_t index = str.find(subString);
    if (index == std::string::npos || index != 38) {
        static_assert(true, "Index of substring not found in StdStringIndexOfBenchmark");
    }
}

#endif

void CStringConcatBenchmark() {

    const char* str1 = "Hello, ";
    const char* str2 = "World!";
    const char* str3 = " This is a test of concatenation.";
    const size_t size = strlen(str1) + strlen(str2) + strlen(str3);
    char* result = new char[size + 1];
    strcpy(result, str1);
    strcat(result, str2);
    strcat(result, str3);
    result[size] = '\0'; // Null-terminate the string
    // Just to ensure the operation is performed
    mla_test_int32_t length = (mla_test_int32_t)strlen(result);
    (void)length; // Prevent unused variable warning
    delete[] result; // Clean up allocated memory
}




void CStringContainsBenchmark() {

    const char* str = "Hello, World! This is a test string for benchmarking.";
    const char* subString = "for";

    if (strstr(str, subString) == nullptr) {
        static_assert(true, "Substring not found in CStringContainsBenchmark");
    }
}

void CStringIndexOfBenchmark() {

    const char* str = "Hello, World! This is a test string for benchmarking.";
    const char* subString = "for";

    const char* found = strstr(str, subString);
    if (found == nullptr || (found - str) != 38) {
        static_assert(true, "Index of substring not found in CStringIndexOfBenchmark");
    }
}


void RegisterNativeStringBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    // Concat Benchmarks
    ////////////////////////////////////////////

    mla_benchmark_t benchmark = mla_benchmark("CStringConcatBenchmark", benchmark_category, CStringConcatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
    benchmark = mla_benchmark("StdStringConcat", benchmark_category, StdStringConcatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
#endif


    // Contains Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("CStringContains", benchmark_category, CStringContainsBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
    benchmark = mla_benchmark("StdStringContains", benchmark_category, StdStringContainsBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
#endif

    // IndexOf Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("CStringIndexOf", benchmark_category, CStringIndexOfBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
    benchmark = mla_benchmark("StdStringIndexOf", benchmark_category, StdStringIndexOfBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
#endif

}


#endif
