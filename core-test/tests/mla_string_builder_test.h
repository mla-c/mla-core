//
// Created by chris on 5/17/2026.
//

#ifndef MLA_STRING_BUILDER_TEST_H
#define MLA_STRING_BUILDER_TEST_H

#include "../../lib/base-lib/core/system/mla_string_builder.h"
#include "../../lib/base-lib/core/system/mla_string_concat.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

void StringBuilder_AppendExpected(mla_string_t& p_Current, const mla_string_t& p_Part) {
    mla_string_t next = mla_string_concat(p_Current, p_Part);
    p_Current = next;
}

void StringBuilderCreateDefaultTest() {
    mla_string_builder_t builder = mla_string_builder_create();
    assert_equal(builder.bufferSize, (mla_size_t)mla_global_config_string_builder_default_buffer_size,
                 "Default string builder capacity should match global config");
    assert_equal(builder.position, (mla_size_t)0, "Default string builder position should be 0");

}

void StringBuilderCreateWithCapacityTest() {
    mla_string_builder_t builder = mla_string_builder_create(8);
    assert_equal(builder.bufferSize, (mla_size_t)8, "String builder should use provided initial capacity");

    mla_bool_t appendResult = mla_string_builder_append(builder, mla_string_const("hello"));
    assert_true(appendResult, "Append should succeed");

    mla_string_t result = mla_string_builder_to_string(builder);
    assert_true(mla_string_equals(result, mla_string_const("hello")), "String builder content should match");

}

void StringBuilderCreateWithBufferAndPositionTest() {
    mla_pointer_t buffer = mla_create_char_array(12);
    mla_char_t* bufferData = mla_pointer_get_data<mla_char_t>(buffer);

    if (bufferData == nullptr) {
        assert_fail("Buffer allocation failed");
        return;
    }

    bufferData[0] = 'p';
    bufferData[1] = 'r';
    bufferData[2] = 'e';

    mla_string_builder_t builder = mla_string_builder_create(buffer, 3);
    mla_bool_t appendResult = mla_string_builder_append(builder, mla_string_const("fix"));
    assert_true(appendResult, "Append should succeed for existing buffer");

    mla_string_t result = mla_string_builder_to_string(builder);
    assert_true(mla_string_equals(result, mla_string_const("prefix")), "String builder should continue from given position");
}

void StringBuilderAppendAllBasicTypesTest() {
    mla_string_builder_t builder = mla_string_builder_create(4);

    assert_true(mla_string_builder_append(builder, true), "Append bool should succeed");
    assert_true(mla_string_builder_append(builder, (mla_int8_t)-8), "Append int8 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_uint8_t)8), "Append uint8 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_int16_t)-16), "Append int16 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_uint16_t)16), "Append uint16 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_int32_t)-32), "Append int32 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_uint32_t)32), "Append uint32 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_int64_t)-64), "Append int64 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_uint64_t)64), "Append uint64 should succeed");
    assert_true(mla_string_builder_append(builder, (mla_float_t)1.5F, 1), "Append float should succeed");
    assert_true(mla_string_builder_append(builder, (mla_double_t)2.25, 2), "Append double should succeed");
    assert_true(mla_string_builder_append(builder, (mla_char_t)'Z'), "Append char should succeed");
    assert_true(mla_string_builder_append(builder, (mla_platform_pointer_t)nullptr), "Append pointer should succeed");
    assert_true(mla_string_builder_append(builder, mla_string_const("!")), "Append mla_string_t should succeed");

    mla_string_t result = mla_string_builder_to_string(builder);
    mla_string_t expected = mla_string_empty();

    mla_string_t part = mla_string_from_bool(true);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_int8(-8);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_uint8(8);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_int16(-16);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_uint16(16);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_int32(-32);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_uint32(32);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_int64(-64);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_uint64(64);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_float(static_cast<mla_float_t>(1.5F), 1);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_double(static_cast<mla_double_t>(2.25), 2);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_const("Z");
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_from_uint64_hex(0);
    StringBuilder_AppendExpected(expected, part);

    part = mla_string_const("!");
    StringBuilder_AppendExpected(expected, part);

    assert_true(mla_string_equals(result, expected),
                "String builder should append all basic data types in order");

}

void RegisterStringBuilderTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("CreateDefault", test_category, StringBuilderCreateDefaultTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CreateWithCapacity", test_category, StringBuilderCreateWithCapacityTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CreateWithBufferAndPosition", test_category, StringBuilderCreateWithBufferAndPositionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AppendAllBasicTypes", test_category, StringBuilderAppendAllBasicTypesTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
