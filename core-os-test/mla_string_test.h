//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_STRING_TEST_H
#define COREOS_MLA_STRING_TEST_H

#include "../core-os/memory/mla_memory_hook.h"
#include "../core-os/system/mla_string.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

void SizeOfTest() {

    if (sizeof(mla_pointer_t) == 8) {
        // 64 bit
        assert_equal((mla_test_int32_t)sizeof(mla_string_t), (mla_test_int32_t)32, "Size of mla_string_t should be 32 bytes");
    } else if (sizeof(mla_pointer_t) == 4) {
        // 32 bit
        assert_equal((mla_test_int32_t)sizeof(mla_string_t), (mla_test_int32_t)16, "Size of mla_string_t should be 16 bytes");
    } else {
        assert_true(false, "Unknown pointer size");
    }

}

void ContainsCLayoutTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void ContainsBufferLayoutTest() {

    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void StartsWithTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    assert_true(mla_string_starts_with(mla_str, mla_string("Hello")), "MlaString should start with 'Hello'");
    assert_false(mla_string_starts_with(mla_str, mla_string("World")), "MlaString should not start with 'World'");
}

void LengthTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.length, (mla_uint32_t)13, "MlaString length should be 13");
    mla_string_t empty_mla_str = mla_string("");
    assert_equal(empty_mla_str.length, (mla_uint32_t)0, "Length of empty MlaString should be 0");
}

void EndsWithTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    assert_true(mla_string_ends_with(mla_str, mla_string("World!")), "MlaString should end with 'World!'");
    assert_false(mla_string_ends_with(mla_str, mla_string("Hello")), "MlaString should not end with 'Hello'");
}

void EqualsTest() {

    mla_string_t mla_str1 = mla_string("Hello, World!");
    mla_string_t mla_str2 = mla_string("Hello, World!");
    mla_string_t mla_str3 = mla_string("Goodbye, World!");

    assert_true(mla_string_equals(mla_str1, mla_str2), "MlaString should be equal");
    assert_false(mla_string_equals(mla_str1, mla_str3), "MlaString should not be equal");

}

void EqualsIgnoreCaseTest() {

    mla_string_t mla_str1 = mla_string("Hello, World!");
    mla_string_t mla_str2 = mla_string("hello, wOrld!");
    mla_string_t mla_str3 = mla_string("Goodbye, World!");

    assert_true(mla_string_equals_ignore_case(mla_str1, mla_str2), "MlaString should be equal ignoring case");
    assert_false(mla_string_equals_ignore_case(mla_str1, mla_str3), "MlaString should not be equal ignoring case");
}

void IndexOfCLayoutTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7, "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1, "MlaString index of 'world' should be -1 (not found)");
}

void IndexOfBufferLayoutTest() {

    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be buffer layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7, "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1, "MlaString index of 'world' should be -1 (not found)");
}

void LastIndexOfTest() {

    mla_string_t mla_str = mla_string("Hello, World! Hello, Universe!");
    assert_equal(mla_string_last_index_of(mla_str, mla_string("Hello")), (mla_int32_t)14, "MlaString last index of 'Hello' should be 19");
    assert_equal(mla_string_last_index_of(mla_str, mla_string("world")), (mla_int32_t)-1, "MlaString last index of 'world' should be -1 (not found)");

}

void ToCStringTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, true);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_true(mla_c_str.isOwner, "MlaString C-string should be owned by the caller");
    mla_free(const_cast<mla_char_t*>(mla_c_str.c_str)); // Clean up allocated memory
}

void ToCString_No_Force_CopyTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, false);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_false(mla_c_str.isOwner, "MlaString C-string should not be owned by the caller");
}

void ToCStringFromBufferTest() {

    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, true);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_true(mla_c_str.isOwner, "MlaString C-string should be owned by the caller");
    mla_free(const_cast<mla_char_t*>(mla_c_str.c_str)); // Clean up allocated memory
}

void ToCStringFromBuffer_No_Force_CopyTest() {

    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, false);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_false(mla_c_str.isOwner, "MlaString C-string should be owned by the caller");

    mla_c_string_t mla_c_str2 = mla_string_to_cString(mla_str, false);
    assert_true(mla_c_str2.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str2.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_false(mla_c_str2.isOwner, "MlaString C-string should not be owned by the caller");
}

void AccessCharTest() {

    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.data[0], 'H', "First character of MlaString should be 'H'");
    assert_equal(mla_str.data[7], 'W', "Eighth character of MlaString should be 'W'");
    assert_equal(mla_str.data[12], '!', "Last character of MlaString should be '!'");
}

void ConcatTest() {

    mla_string_t mla_str1 = mla_string("Hello, ");
    mla_string_t mla_str2 = mla_string("World!");
    mla_string_t mla_result = mla_string_concat(mla_str1, mla_str2);

    assert_equal(mla_result.length, (mla_uint32_t)13, "MlaString concatenated length should be 13");
    assert_true(mla_string_equals(mla_result, mla_string("Hello, World!")), "MlaString concatenated should equal 'Hello, World!'");

    mla_string_destroy(mla_result);
}

static mla_pointer_t AutoMemoryManagementTest_last_pointer;

mla_pointer_t AutoMemoryManagementTest_Malloc(mla_size_t size) {
    (void)size;
    return nullptr;
}

mla_bool_t AutoMemoryManagementTest_Free(mla_pointer_t ptr) {
    AutoMemoryManagementTest_last_pointer = ptr;
    return false;
}

void AutoMemoryManagementTest() {

    mla_buffer_t* unsafePointer = nullptr;

    mla_memory_hook_t hook = mla_memory_hook_install(AutoMemoryManagementTest_Malloc, AutoMemoryManagementTest_Free);

    {

        mla_string_t datacopy = mla_string_empty();

        {
            mla_string_t data = mla_string_concat(mla_string("Hello, "), mla_string("World!"), mla_string(" This is a test of concatenation."));
            assert_equal(data.length, (mla_uint32_t)46, "Concatenated string length should be 58");

            assert_equal(data.dataOwner.buffer->refCount, (mla_uint32_t)1, "Reference count should be 1 after concatenation");
            datacopy = data;
            assert_equal(data.dataOwner.buffer->refCount, (mla_uint32_t)2, "Reference count should be 2 after copying");
            assert_equal(datacopy.dataOwner.buffer->refCount, (mla_uint32_t)2, "Reference count should be 2 after copying");
            assert_equal(datacopy.dataOwner.buffer->data, data.dataOwner.buffer->data, "Copied string data should match original");

            data = mla_string_empty(); // Clear the original string
        }

        assert_equal(datacopy.dataOwner.buffer->refCount, (mla_uint32_t)1, "Reference count should be 1 after clearing the original string");

        assert_equal(datacopy.length, (mla_uint32_t)46, "Copied string length should still be 58");
        assert_true(mla_string_equals(datacopy, mla_string("Hello, World! This is a test of concatenation.")),
                   "Copied string should equal 'Hello, World! This is a test of concatenation.'");

        AutoMemoryManagementTest_last_pointer = nullptr;
        unsafePointer = datacopy.dataOwner.buffer;

    }
    // Check ich the buffer was released
    assert_equal(AutoMemoryManagementTest_last_pointer, unsafePointer, "Last pointer should match the unsafe pointer");

    mla_memory_hook_uninstall(hook);
    unsafePointer = nullptr; // Clear the unsafe pointer
}

void SubStringTest() {

    mla_string_t mla_str = mla_string_concat(mla_string("Hello, "), mla_string("World!"));
    mla_string_t sub_str = mla_string_substr(mla_str, 7, 11); // "World"

    // Check memory managemant
    assert_equal(sub_str.dataOwner.buffer->refCount, (mla_uint32_t)2, "Reference count should be 2 after creating substring");
    assert_equal(sub_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_SUB_STRING, "Substring should have SUB_STRING layout");
    assert_equal(sub_str.length, (mla_uint32_t)5, "Substring length should be 5");
    assert_true(mla_string_equals(sub_str, mla_string("World")), "Substring should equal 'World'");


}


void RegisterStringTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SizeOf", test_category, SizeOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsCLayout", test_category, ContainsCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsBufferLayout", test_category, ContainsBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StartsWith", test_category, StartsWithTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Length", test_category, LengthTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EndsWith", test_category, EndsWithTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Equals", test_category, EqualsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsIgnoreCase", test_category, EqualsIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOf", test_category, IndexOfCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfBufferLayout", test_category, IndexOfBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LastIndexOf", test_category, LastIndexOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCString", test_category, ToCStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCString_No_Force_Copy", test_category, ToCString_No_Force_CopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCStringFromBuffer", test_category, ToCStringFromBufferTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCStringFromBuffer_No_Force_Copy", test_category, ToCStringFromBuffer_No_Force_CopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AccessChar", test_category, AccessCharTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Concat", test_category, ConcatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoMemoryManagement", test_category, AutoMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SubString", test_category, SubStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

void StringConcatBenchmark() {

    mla_string_t str1 = mla_string("Hello, ");
    mla_string_t str2 = mla_string("World!");
    mla_string_t str3 = mla_string(" This is a test of concatenation.");

    mla_string_t result = mla_string_concat(str1, str2, str3);

    mla_test_int32_t length = result.length;
    (void)length; // Prevent unused variable warning

    mla_string_destroy(result);
}

void StringContains_Buffer_LayoutBenchmark() {

    const mla_test_char_t* data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = (mla_test_int32_t)strlen(data); // Length of the string

    mla_string_t str = mla_string(data, length);
    mla_string_t subString = mla_string("for");

    if (!mla_string_contains(str, subString)) {
        static_assert(true, "Substring not found in MlaStringContainsBenchmark");
    }
}

void StringContains_C_LayoutBenchmark() {

    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_string_t subString = mla_string("for");


    if (!mla_string_contains(str, subString)) {
        static_assert(true, "Substring not found in MlaStringContainsBenchmark");
    }
}

void StringIndexOf_Buffer_LayoutBenchmark() {

    const mla_test_char_t* data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = (mla_test_int32_t)strlen(data); // Length of the string

    mla_string_t str = mla_string(data, length);
    mla_string_t subString = mla_string("for");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }

}

void StringIndexOf_C_LayoutBenchmark() {

    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_string_t subString = mla_string("for");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }

}


void String_to_C_LayoutBenchmark() {

    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_c_string_t cStr = mla_string_to_cString(str, false);

    if (cStr.isOwner) {
        delete cStr.c_str;
    }

    mla_c_string_t cStr2 = mla_string_to_cString(str, false);

    if (cStr2.isOwner) {
        delete cStr2.c_str;
    }
}

void String_to_Buffer_LayoutBenchmark() {

    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.", 53);
    mla_c_string_t cStr = mla_string_to_cString(str, false);

    if (cStr.isOwner) {
        delete cStr.c_str;
    }

    mla_c_string_t cStr2 = mla_string_to_cString(str, false);

    if (cStr2.isOwner) {
        delete cStr2.c_str;
    }
}



void RegisterStringBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    // Concat Benchmarks
    ////////////////////////////////////////////
    mla_benchmark_t benchmark = mla_benchmark("Concat", benchmark_category, StringConcatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Contains Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("Contains_Buffer", benchmark_category, StringContains_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Contains_CStr", benchmark_category, StringContains_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // IndexOf Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("IndexOf_Buffer", benchmark_category, StringIndexOf_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("IndexOf_CStr", benchmark_category, StringIndexOf_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // To C String Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("C_String_From_C_Layout", benchmark_category, String_to_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("C_String_From_Buffer_Layout", benchmark_category, String_to_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);




}


#endif
