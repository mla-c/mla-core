//
// Created by Jules on 2025.
//

#ifndef MLA_BYTES_TEST_H
#define MLA_BYTES_TEST_H

#include "../../lib/base-lib/core/system/mla_bytes.h"
#include "../../lib/base-lib/core/system/mla_string.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"
#include "../../lib/base-lib/test-support/Test/mla_test.h"

inline void BytesEmptyTest() {
    mla_bytes_t bytes = mla_bytes_empty();
    assert_true(mla_bytes_is_empty(bytes), "Empty bytes should be empty");
    assert_equal(mla_bytes_length(bytes), (mla_size_t)0, "Empty bytes size should be 0");
    assert_null(mla_bytes_get_data_readonly(bytes), "Empty bytes data should be null");
}

inline void BytesAllocTest() {
    mla_size_t length = 10;
    mla_bytes_t bytes = mla_bytes(length);
    assert_false(mla_bytes_is_empty(bytes), "Allocated bytes should not be empty");
    assert_equal(mla_bytes_length(bytes), length, "Allocated bytes size should match requested length");
    const mla_byte_t* data = mla_bytes_get_data_readonly(bytes);
    assert_not_null(data, "Allocated bytes data should not be null");

    if (data != nullptr) {
        for (mla_size_t i = 0; i < length; ++i) {
            assert_equal(data[i], (mla_byte_t)0, "Allocated bytes should be initialized to 0");
        }
    }
}

inline void BytesFromExternalBufferTest() {
    mla_byte_t data[] = {1, 2, 3, 4, 5};
    mla_size_t size = sizeof(data);
    mla_bytes_t bytes = mla_bytes_from_external_buffer(mla_platform_pointer_to_managed_pointer(data), size);

    assert_equal(mla_bytes_get_data_readonly(bytes), data, "Bytes data should point to external buffer");
    assert_equal(mla_bytes_length(bytes), size, "Bytes size should match external buffer size");
}

inline void BytesFromBufferWithOwnershipTest() {
    mla_size_t size = 5;

    mla_pointer_t data_ptr = mla_malloc_buffer(size);

    mla_byte_t* data = mla_pointer_get_data<mla_byte_t>(data_ptr);

    assert_not_null(data, "Malloc failed");
    if (data != nullptr) {
        data[0] = 10;
    }

    mla_bytes_t bytes = mla_bytes_from_external_buffer(data_ptr, size);
    assert_equal(mla_bytes_get_data_readonly(bytes), data, "Bytes data should point to provided buffer");
    assert_equal(mla_bytes_length(bytes), size, "Bytes size should match provided size");
}

inline void BytesBase64Test() {
    mla_byte_t raw_data[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f}; // "Hello"
    mla_bytes_t bytes = mla_bytes_from_external_buffer(mla_platform_pointer_to_managed_pointer(raw_data), sizeof(raw_data));

    mla_string_t base64 = mla_bytes_to_base64(bytes);
    mla_string_t expected = mla_string("SGVsbG8=");
    assert_true(mla_string_equals(base64, expected), "Base64 encoding failed");

    mla_bytes_t decoded = mla_bytes_from_base64(base64);
    assert_equal(mla_bytes_length(decoded), mla_bytes_length(bytes), "Decoded size mismatch");
    const mla_byte_t* decoded_data = mla_bytes_get_data_readonly(decoded);
    const mla_byte_t* original_data = mla_bytes_get_data_readonly(bytes);
    if (decoded_data != nullptr && original_data != nullptr) {
        for (mla_size_t i = 0; i < mla_bytes_length(decoded); ++i) {
            assert_equal(decoded_data[i], original_data[i], "Decoded data mismatch");
        }
    }
}

inline void BytesStringConversionTest() {
    mla_byte_t raw_data[] = "Test Data";
    mla_size_t size = sizeof(raw_data) - 1;
    mla_bytes_t bytes = mla_bytes_from_external_buffer(mla_platform_pointer_to_managed_pointer(raw_data), size);

    mla_string_t str = mla_bytes_to_string(bytes);
    assert_equal(mla_string_length(str), size, "String length mismatch");
    mla_string_t expected = mla_string("Test Data");
    assert_true(mla_string_equals(str, expected), "String content mismatch");

    mla_bytes_t from_str = mla_bytes_from_string(str);
    assert_equal(mla_bytes_length(from_str), size, "Bytes from string size mismatch");
    const mla_byte_t* from_str_data = mla_bytes_get_data_readonly(from_str);
    const mla_byte_t* original_data = mla_bytes_get_data_readonly(bytes);
    if (from_str_data != nullptr && original_data != nullptr) {
        for (mla_size_t i = 0; i < mla_bytes_length(from_str); ++i) {
            assert_equal(from_str_data[i], original_data[i], "Bytes from string data mismatch");
        }
    }
}

inline void BytesCopyTest() {
    mla_size_t size = 5;
    mla_bytes_t original = mla_bytes(size);
    mla_byte_t* data = mla_bytes_get_data_for_writing(original);
    if (data != nullptr) {
        for (mla_size_t i = 0; i < size; ++i) data[i] = (mla_byte_t)i;
    }

    mla_bytes_t copy = mla_bytes_copy(original);
    assert_equal(mla_bytes_length(copy), mla_bytes_length(original), "Copy size mismatch");
    assert_not_equal(mla_bytes_get_data_readonly(copy), mla_bytes_get_data_readonly(original), "Copy should have different data pointer");

    const mla_byte_t* copy_data = mla_bytes_get_data_readonly(copy);
    const mla_byte_t* original_data = mla_bytes_get_data_readonly(original);
    if (copy_data != nullptr && original_data != nullptr) {
        for (mla_size_t i = 0; i < size; ++i) {
            assert_equal(copy_data[i], original_data[i], "Copy data content mismatch");
        }
    }
}

inline void BytesAccessTest() {
    mla_size_t size = 5;
    mla_bytes_t bytes = mla_bytes(size);

    assert_equal(mla_bytes_length(bytes), size, "Length mismatch");
    assert_equal(mla_bytes_get_data_readonly(bytes), (const mla_byte_t*)mla_bytes_get_data_for_writing(bytes), "Data pointer mismatch");
}

inline void BytesIsEmptyTest() {
    mla_bytes_t empty = mla_bytes_empty();
    assert_true(mla_bytes_is_empty(empty), "mla_bytes_empty should be empty");

    mla_bytes_t not_empty = mla_bytes(1);
    assert_false(mla_bytes_is_empty(not_empty), "mla_bytes(1) should not be empty");
}

inline void BytesManualDestroyTest() {
    mla_bytes_t bytes = mla_bytes(10);
    mla_bytes_destroy(bytes);
    assert_true(mla_bytes_is_empty(bytes), "Destroyed bytes should be empty");
    assert_equal(mla_bytes_length(bytes), (mla_size_t)0, "Destroyed bytes size should be 0");
    assert_null(mla_bytes_get_data_readonly(bytes), "Destroyed bytes data should be null");
}

inline void RegisterBytesTests(mla_test_executor_t& p_TestExecutor) {
    mla_test_t test = mla_test("BytesEmpty", test_category, BytesEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesAlloc", test_category, BytesAllocTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesFromExternalBuffer", test_category, BytesFromExternalBufferTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesFromBufferWithOwnership", test_category, BytesFromBufferWithOwnershipTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesBase64", test_category, BytesBase64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesStringConversion", test_category, BytesStringConversionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesCopy", test_category, BytesCopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesAccess", test_category, BytesAccessTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesIsEmpty", test_category, BytesIsEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("BytesManualDestroy", test_category, BytesManualDestroyTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
