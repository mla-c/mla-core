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

struct simple_config {
    mla_uint8_t version;
    mla_float_t value;
};

inline mla_deserializer_read_result_t simple_config_read(mla_deserializer_t& deserializer, mla_pointer_t config, const mla_string_t& property_name) {
    simple_config* obj = static_cast<simple_config*>(config);
    if (mla_string_equals_const(property_name, "version")) {
        mla_deserializer_read_uint8(deserializer, obj->version);
    } else if (mla_string_equals_const(property_name, "value")) {
        mla_deserializer_read_float(deserializer, obj->value);
    } else {
        return MLA_DESERIALIZER_READ_SKIPPED;
    }

}

inline void simple_config_write(mla_serializer_t& serializer, const mla_pointer_t config) {
    const simple_config* obj = static_cast<const simple_config*>(config);
    mla_serializer_write_uint8(serializer, mla_string_const("version"), obj->version);
    mla_serializer_write_float(serializer, mla_string_const("value"), obj->value);
}

inline mla_serialize_definition_t simple_config_serialize_def() {
    return mla_serialize_definition(simple_config_read, simple_config_write);
}

struct complex_config {
    mla_int64_t id;
    mla_string_t name;
    mla_array_list_t<mla_int32_t> values;
};

inline mla_deserializer_read_result_t complex_config_read(mla_deserializer_t& deserializer, mla_pointer_t config, const mla_string_t& property_name) {
    complex_config* obj = static_cast<complex_config*>(config);
    if (mla_string_equals_const(property_name, "id")) {
        mla_deserializer_read_int64(deserializer, obj->id);
    } else if (mla_string_equals_const(property_name, "name")) {
        mla_deserializer_read_string(deserializer, obj->name);
    } else if (mla_string_equals_const(property_name, "values")) {

        if (mla_serializer_read_list(deserializer, obj->values)) {
            return MLA_DESERIALIZER_READ_HANDLED;
        } else {
            return MLA_DESERIALIZER_READ_ERROR;
        }

    } else {
        return MLA_DESERIALIZER_READ_SKIPPED;
    }

}

inline void complex_config_write(mla_serializer_t& serializer, const mla_pointer_t config) {
    const complex_config* obj = static_cast<const complex_config*>(config);
    mla_serializer_write_int64(serializer, mla_string_const("id"), obj->id);
    mla_serializer_write_string(serializer, mla_string_const("name"), obj->name);
    mla_serializer_write_list(serializer, mla_string_const("values"), obj->values);
}

inline mla_serialize_definition_t complex_config_serialize_def() {
    return mla_serialize_definition(complex_config_read, complex_config_write);
}

// Test: Multiple config write and read in same order
inline void MultiConfigSameOrderTest() {
    // Create configs
    test_config_struct config1 = {100, true, mla_string("First")};
    simple_config config2 = {5, 3.14f};

    // Create definitions
    mla_config_definition_t def1 = mla_config_definition<test_config_struct>(
        mla_string_const("test_config1"), test_config_struct_serialize_def());
    mla_config_definition_t def2 = mla_config_definition<simple_config>(
        mla_string_const("simple_config"), simple_config_serialize_def());

    // Write configs
    assert_true(mla_config_manager_write(def1, &config1), "Failed to write config1");
    assert_true(mla_config_manager_write(def2, &config2), "Failed to write config2");

    // Read in same order
    test_config_struct loaded1 = {0, false, mla_string_empty()};
    simple_config loaded2 = {0, 0.0f};

    assert_true(mla_config_manager_read(def1, &loaded1), "Failed to read config1");
    assert_true(mla_config_manager_read(def2, &loaded2), "Failed to read config2");

    // Validate
    assert_equal(config1.intValue, loaded1.intValue, "Config1 intValue mismatch");
    assert_equal(config1.boolValue, loaded1.boolValue, "Config1 boolValue mismatch");
    assert_struct_equal(mla_string_t, config1.strValue, loaded1.strValue, "Config1 strValue mismatch");

    assert_equal(config2.version, loaded2.version, "Config2 version mismatch");
    assert_equal(config2.value, loaded2.value, "Config2 value mismatch");
}

// Test: Multiple config write and read in different order
inline void MultiConfigDifferentOrderTest() {
    // Create configs
    test_config_struct config1 = {200, false, mla_string("Alpha")};
    simple_config config2 = {10, 2.718f};
    complex_config config3 = {9876543210, mla_string("Gamma"), mla_array_list<mla_int32_t>()};
    mla_array_list_add(config3.values, (mla_int32_t)1);
    mla_array_list_add(config3.values, (mla_int32_t)2);
    mla_array_list_add(config3.values, (mla_int32_t)3);


    // Create definitions
    mla_config_definition_t def1 = mla_config_definition<test_config_struct>(
        mla_string_const("test_config1"), test_config_struct_serialize_def());
    mla_config_definition_t def2 = mla_config_definition<simple_config>(
        mla_string_const("simple_config"), simple_config_serialize_def());
    mla_config_definition_t def3 = mla_config_definition<complex_config>(
        mla_string_const("complex_config"), complex_config_serialize_def());

    // Write configs in one order
    assert_true(mla_config_manager_write(def1, &config1), "Failed to write config1");
    assert_true(mla_config_manager_write(def2, &config2), "Failed to write config2");
    assert_true(mla_config_manager_write(def3, &config3), "Failed to write config3");

    // Read in different order
    simple_config loaded2 = {0, 0.0f};
    complex_config loaded3 = {0, mla_string_empty(), mla_array_list<mla_int32_t>()};
    test_config_struct loaded1 = {0, false, mla_string_empty()};

    assert_true(mla_config_manager_read(def2, &loaded2), "Failed to read config2");
    assert_true(mla_config_manager_read(def3, &loaded3), "Failed to read config3");
    assert_true(mla_config_manager_read(def1, &loaded1), "Failed to read config1");

    // Validate
    assert_equal(config2.version, loaded2.version, "Config2 version mismatch");
    assert_equal(config2.value, loaded2.value, "Config2 value mismatch");

    assert_equal(config3.id, loaded3.id, "Config3 id mismatch");
    assert_struct_equal(mla_string_t, config3.name, loaded3.name, "Config3 name mismatch");
    assert_equal(mla_array_list_size(config3.values), mla_array_list_size(loaded3.values), "Config3 values size mismatch");
    for (mla_size_t i = 0; i < mla_array_list_size(config3.values); i++) {

        assert_equal(mla_array_list_get_ref(config3.values, i), mla_array_list_get_ref(loaded3.values, i), "Config3 values element mismatch");
    }

    assert_equal(config1.intValue, loaded1.intValue, "Config1 intValue mismatch");
    assert_equal(config1.boolValue, loaded1.boolValue, "Config1 boolValue mismatch");
    assert_struct_equal(mla_string_t, config1.strValue, loaded1.strValue, "Config1 strValue mismatch");
}


inline void MultiConfigUpdateTest() {
    // Create configs
    test_config_struct config1 = {300, true, mla_string("Original")};
    simple_config config2 = {20, 1.618f};

    // Create definitions
    mla_config_definition_t def1 = mla_config_definition<test_config_struct>(
        mla_string_const("test_config1"), test_config_struct_serialize_def());
    mla_config_definition_t def2 = mla_config_definition<simple_config>(
        mla_string_const("simple_config"), simple_config_serialize_def());

    // Write both configs
    assert_true(mla_config_manager_write(def1, &config1), "Failed to write config1");
    assert_true(mla_config_manager_write(def2, &config2), "Failed to write config2");

    // Update just one config
    test_config_struct updated1 = {400, false, mla_string("Updated")};
    assert_true(mla_config_manager_write(def1, &updated1), "Failed to update config1");

    // Read both back
    test_config_struct loaded1 = {0, false, mla_string_empty()};
    simple_config loaded2 = {0, 0.0f};

    assert_true(mla_config_manager_read(def1, &loaded1), "Failed to read config1");
    assert_true(mla_config_manager_read(def2, &loaded2), "Failed to read config2");

    // Validate updated values
    assert_equal(updated1.intValue, loaded1.intValue, "Updated config1 intValue mismatch");
    assert_equal(updated1.boolValue, loaded1.boolValue, "Updated config1 boolValue mismatch");
    assert_struct_equal(mla_string_t, updated1.strValue, loaded1.strValue, "Updated config1 strValue mismatch");

    // Validate unchanged values
    assert_equal(config2.version, loaded2.version, "Config2 version mismatch");
    assert_equal(config2.value, loaded2.value, "Config2 value mismatch");
}


void RegisterConfigTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("ConfigWriteRead", test_category, ConfigWriteReadTest, nullptr, nullptr);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultiConfigSameOrder", test_category, MultiConfigSameOrderTest, nullptr, nullptr);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultiConfigDifferentOrder", test_category, MultiConfigDifferentOrderTest, nullptr, nullptr);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultiConfigUpdate", test_category, MultiConfigUpdateTest, nullptr, nullptr);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif

