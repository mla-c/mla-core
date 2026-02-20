//
// Created by chris on 2/11/2026.
//

#ifndef COREOS_MLA_USER_DATA_TEST_H
#define COREOS_MLA_USER_DATA_TEST_H

#include "../core-os/system/mla_user_data.h"
#include "../core-os-test-support/mla_test_executor.h"

// Test setting and getting int8 values
inline void UserDataSetGetInt8Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_int8(userData, "test", 42);

    assert_true(success, "Should successfully set int8 value");

    mla_int8_t value = mla_user_data_get_int8(userData, "test");
    assert_equal(value, (mla_int8_t)42, "Should retrieve correct int8 value");

    mla_int8_t defaultValue = mla_user_data_get_int8(userData, "missing", -1);
    assert_equal(defaultValue, (mla_int8_t)-1, "Should return default value for missing key");
}

// Test setting and getting uint8 values
inline void UserDataSetGetUint8Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_uint8(userData, "test", 255);

    assert_true(success, "Should successfully set uint8 value");

    mla_uint8_t value = mla_user_data_get_uint8(userData, "test");
    assert_equal(value, (mla_uint8_t)255, "Should retrieve correct uint8 value");
}

// Test setting and getting int16 values
inline void UserDataSetGetInt16Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_int16(userData, "test", -1000);

    assert_true(success, "Should successfully set int16 value");

    mla_int16_t value = mla_user_data_get_int16(userData, "test");
    assert_equal(value, (mla_int16_t)-1000, "Should retrieve correct int16 value");
}

// Test setting and getting uint16 values
inline void UserDataSetGetUint16Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_uint16(userData, "test", 50000);

    assert_true(success, "Should successfully set uint16 value");

    mla_uint16_t value = mla_user_data_get_uint16(userData, "test");
    assert_equal(value, (mla_uint16_t)50000, "Should retrieve correct uint16 value");
}

// Test setting and getting int32 values
inline void UserDataSetGetInt32Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_int32(userData, "test", -100000);

    assert_true(success, "Should successfully set int32 value");

    mla_int32_t value = mla_user_data_get_int32(userData, "test");
    assert_equal(value, (mla_int32_t)-100000, "Should retrieve correct int32 value");
}

// Test setting and getting uint32 values
inline void UserDataSetGetUint32Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_uint32(userData, "test", 4000000);

    assert_true(success, "Should successfully set uint32 value");

    mla_uint32_t value = mla_user_data_get_uint32(userData, "test");
    assert_equal(value, (mla_uint32_t)4000000, "Should retrieve correct uint32 value");
}

// Test setting and getting int64 values
inline void UserDataSetGetInt64Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_int64(userData, "test", -9000000000);

    assert_true(success, "Should successfully set int64 value");

    mla_int64_t value = mla_user_data_get_int64(userData, "test");
    assert_equal(value, (mla_int64_t)-9000000000, "Should retrieve correct int64 value");
}

// Test setting and getting uint64 values
inline void UserDataSetGetUint64Test() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_uint64(userData, "test", 9000000000);

    assert_true(success, "Should successfully set uint64 value");

    mla_uint64_t value = mla_user_data_get_uint64(userData, "test");
    assert_equal(value, (mla_uint64_t)9000000000, "Should retrieve correct uint64 value");
}

// Test setting and getting float values
inline void UserDataSetGetFloatTest() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_float(userData, "test", 3.14f);

    assert_true(success, "Should successfully set float value");

    mla_float_t value = mla_user_data_get_float(userData, "test");
    assert_true(value > 3.13999f && value < 3.14001f, "Should retrieve correct float value");
}

// Test setting and getting double values
inline void UserDataSetGetDoubleTest() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_double(userData, "test", 3.14159265359);

    assert_true(success, "Should successfully set double value");

    mla_double_t value = mla_user_data_get_double(userData, "test");
    assert_true(value > 3.14159265358 && value < 3.14159265360, "Should retrieve correct double value");
}

// Test setting and getting bool values
inline void UserDataSetGetBoolTest() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_bool(userData, "test", true);

    assert_true(success, "Should successfully set bool value");

    mla_bool_t value = mla_user_data_get_bool(userData, "test");
    assert_true(value, "Should retrieve correct bool value (true)");

    success = mla_user_data_set_bool(userData, "test2", false);
    assert_true(success, "Should successfully set bool value to false");

    value = mla_user_data_get_bool(userData, "test2");
    assert_false(value, "Should retrieve correct bool value (false)");
}

// Test setting and getting char values
inline void UserDataSetGetCharTest() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_bool_t success = mla_user_data_set_char(userData, "test", 'A');

    assert_true(success, "Should successfully set char value");

    mla_char_t value = mla_user_data_get_char(userData, "test");
    assert_equal(value, 'A', "Should retrieve correct char value");
}

// Test setting and getting pointer values without ownership
inline void UserDataSetGetPointerWithoutOwnershipTest() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_int32_t testData = 12345;

    mla_bool_t success = mla_user_data_set_pointer_without_ownership(userData, "test", &testData);
    assert_true(success, "Should successfully set pointer without ownership");

    mla_int32_t* value = mla_user_data_get_pointer<mla_int32_t>(userData, "test");
    assert_not_null(value, "Should retrieve non-null pointer");
    assert_equal(*value, 12345, "Should retrieve correct pointer value");
}

// Test setting and getting pointer values with ownership
inline void UserDataSetGetPointerWithOwnershipTest() {
    mla_user_data_t userData = mla_user_data_empty();
    mla_int32_t* testData = (mla_int32_t*)mla_malloc(sizeof(mla_int32_t));

    if (testData == nullptr) {
        assert_fail("Failed to allocate memoryfor test data");
        return;
    }

    *testData = 54321;

    mla_bool_t success = mla_user_data_set_pointer_with_ownership<mla_int32_t, mla_default_init(mla_int32_t)>(userData, "test", testData);
    assert_true(success, "Should successfully set pointer with ownership");

    mla_int32_t* value = mla_user_data_get_pointer<mla_int32_t>(userData, "test");
    assert_not_null(value, "Should retrieve non-null pointer");
    assert_equal(*value, 54321, "Should retrieve correct pointer value");
}

// Test multiple values (should convert to list internally)
inline void UserDataMultipleValuesTest() {
    mla_user_data_t userData = mla_user_data_empty();

    mla_bool_t success = mla_user_data_set_int32(userData, "value1", 100);
    assert_true(success, "Should successfully set first value");

    success = mla_user_data_set_int32(userData, "value2", 200);
    assert_true(success, "Should successfully set second value");

    success = mla_user_data_set_int32(userData, "value3", 300);
    assert_true(success, "Should successfully set third value");

    mla_int32_t val1 = mla_user_data_get_int32(userData, "value1");
    mla_int32_t val2 = mla_user_data_get_int32(userData, "value2");
    mla_int32_t val3 = mla_user_data_get_int32(userData, "value3");

    assert_equal(val1, 100, "Should retrieve correct first value");
    assert_equal(val2, 200, "Should retrieve correct second value");
    assert_equal(val3, 300, "Should retrieve correct third value");
}

// Test overwriting existing value with same name
inline void UserDataOverwriteValueTest() {
    mla_user_data_t userData = mla_user_data_empty();

    mla_bool_t success = mla_user_data_set_int32(userData, "test", 100);
    assert_true(success, "Should successfully set initial value");

    mla_int32_t value = mla_user_data_get_int32(userData, "test");
    assert_equal(value, 100, "Should retrieve correct initial value");

    success = mla_user_data_set_int32(userData, "test", 200);
    assert_true(success, "Should successfully overwrite value");

    value = mla_user_data_get_int32(userData, "test");
    assert_equal(value, 200, "Should retrieve correct overwritten value");
}

// Test mixed data types
inline void UserDataMixedTypesTest() {
    mla_user_data_t userData = mla_user_data_empty();

    mla_user_data_set_int32(userData, "int", 42);
    mla_user_data_set_float(userData, "float", 3.14f);
    mla_user_data_set_bool(userData, "bool", true);
    mla_user_data_set_char(userData, "char", 'X');

    assert_equal(mla_user_data_get_int32(userData, "int"), 42, "Should retrieve int32");
    assert_true(mla_user_data_get_float(userData, "float") > 3.13f, "Should retrieve float");
    assert_true(mla_user_data_get_bool(userData, "bool"), "Should retrieve bool");
    assert_equal(mla_user_data_get_char(userData, "char"), 'X', "Should retrieve char");
}

// Test empty user data
inline void UserDataEmptyTest() {
    mla_user_data_t userData = mla_user_data_empty();

    mla_int32_t value = mla_user_data_get_int32(userData, "missing", 999);
    assert_equal(value, 999, "Should return default value for missing key in empty user data");

    mla_pointer_t ptr = mla_user_data_get_mla_pointer(userData, "missing");
    assert_null(ptr, "Should return null pointer for missing key in empty user data");
}

// Test name size limit (8 characters)
inline void UserDataNameSizeLimitTest() {
    mla_user_data_t userData = mla_user_data_empty();

    // Test with maximum size name (7 chars + null terminator)
    mla_bool_t success = mla_user_data_set_int32(userData, "maxname", 123);
    assert_true(success, "Should successfully set value with 7-char name");

    mla_int32_t value = mla_user_data_get_int32(userData, "maxname");
    assert_equal(value, 123, "Should retrieve value with 7-char name");
}

// Test getting value with different name
inline void UserDataGetNonExistentTest() {
    mla_user_data_t userData = mla_user_data_empty();

    mla_user_data_set_int32(userData, "exists", 100);

    mla_int32_t value = mla_user_data_get_int32(userData, "missing", -1);
    assert_equal(value, -1, "Should return default value for non-existent key");
}

inline void RegisterUserDataTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("UserDataSetGetInt8", test_category, UserDataSetGetInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetUint8", test_category, UserDataSetGetUint8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetInt16", test_category, UserDataSetGetInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetUint16", test_category, UserDataSetGetUint16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetInt32", test_category, UserDataSetGetInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetUint32", test_category, UserDataSetGetUint32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetInt64", test_category, UserDataSetGetInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetUint64", test_category, UserDataSetGetUint64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetFloat", test_category, UserDataSetGetFloatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetDouble", test_category, UserDataSetGetDoubleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetBool", test_category, UserDataSetGetBoolTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetChar", test_category, UserDataSetGetCharTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetPointerWithoutOwnership", test_category, UserDataSetGetPointerWithoutOwnershipTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataSetGetPointerWithOwnership", test_category, UserDataSetGetPointerWithOwnershipTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataMultipleValues", test_category, UserDataMultipleValuesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataOverwriteValue", test_category, UserDataOverwriteValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataMixedTypes", test_category, UserDataMixedTypesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataEmpty", test_category, UserDataEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataNameSizeLimit", test_category, UserDataNameSizeLimitTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UserDataGetNonExistent", test_category, UserDataGetNonExistentTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // COREOS_MLA_USER_DATA_TEST_H
