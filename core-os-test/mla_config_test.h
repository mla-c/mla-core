// mla_config_test.h
// Tests for mla_config system, following the style of mla_serializer_test.h

#ifndef COREOS_MLA_CONFIG_TEST_H
#define COREOS_MLA_CONFIG_TEST_H

#include "../core-os/config/mla_config.h"
#include "../core-os/system/mla_array_list.h"
#include "../core-os-test-support/mla_test_executor.h"
#include <cstring>

// Test struct for config
struct test_config_struct {
    mla_int32_t intValue;
    mla_bool_t boolValue;
    mla_string_t strValue;
};

inline mla_deserializer_read_result_t test_config_struct_read(mla_deserializer_t& deserializer, mla_pointer_t config, const mla_string_t& property_name) {

    test_config_struct* obj = static_cast<test_config_struct*>(config);
    if (mla_string_equals_const(property_name, "intValue")) {
        mla_deserializer_read_int32(deserializer, obj->intValue);
    } else if (mla_string_equals_const(property_name, "boolValue")) {
        mla_deserializer_read_bool(deserializer, obj->boolValue);
    } else if (mla_string_equals_const(property_name, "strValue")) {
        mla_deserializer_read_string(deserializer, obj->strValue);
    } else {
        return MLA_DESERIALIZER_READ_SKIPPED;
    }

}

inline void test_config_struct_write(mla_serializer_t& serializer, const mla_pointer_t config) {

    const test_config_struct* obj = static_cast<const test_config_struct*>(config);
    mla_serializer_write_int32(serializer, mla_string_const("intValue"), obj->intValue);
    mla_serializer_write_bool(serializer, mla_string_const("boolValue"), obj->boolValue);
    mla_serializer_write_string(serializer, mla_string_const("strValue"), obj->strValue);
}

// Helper to create a config definition
inline mla_serialize_definition_t test_config_struct_serialize_def() {
    return mla_serialize_definition(test_config_struct_read, test_config_struct_write);
}


// Test: Write and Read config
inline void ConfigWriteReadTest() {
    // Set up config
    test_config_struct original = {42, true, mla_string("ConfigTest")};
    mla_config_definition_t def = mla_config_definition<test_config_struct>(mla_string_const("test_config"), test_config_struct_serialize_def());

    // Write config
    assert_true(mla_config_manager_write(def, &original), "Failed to write config");

    // Simulate reading back
    test_config_struct loaded = {0, false, mla_string_empty()};
    assert_true(mla_config_manager_read(def, &loaded), "Failed to read config");

    // Validate
    assert_equal(original.intValue, loaded.intValue, "Config intValue mismatch");
    assert_equal(original.boolValue, loaded.boolValue, "Config boolValue mismatch");
    assert_struct_equal(mla_string_t, original.strValue, loaded.strValue, "Config strValue mismatch");
}


void RegisterConfigTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("ConfigWriteRead", test_category, ConfigWriteReadTest, nullptr, nullptr);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif

