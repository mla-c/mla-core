//
// Created by chris on 9/16/2025.
//

#ifndef MLA_SERIALIZER_TEST_H
#define MLA_SERIALIZER_TEST_H


#include "../core-os/serializer/mla_binary_serializer.h"
#include "../core-os/serializer/mla_json_serializer.h"
#include "../core-os/serializer/mla_xml_serializer.h"
#include "../core-os/system/mla_array_list.h"
#include "../core-os-test-support/mla_benchmark_executor.h"
#include "../core-os-test-support/mla_test_executor.h"

static mla_byte_t *mla_serializer_buffer = nullptr;

inline void SetupSerializerTest() {
    mla_serializer_buffer = static_cast<mla_byte_t *>(mla_platform_malloc(1024));
}

inline void TearDownSerializerTest() {
    mla_platform_free(mla_serializer_buffer);
}

struct mla_all_types_inner_struct {
    mla_int32_t int32Value;
    mla_bool_t boolValue;

    static mla_bool_t serialize(mla_serializer_t &serializer, const mla_pointer_t& config) {
        const mla_all_types_inner_struct *obj = mla_pointer_get_data<const mla_all_types_inner_struct>(config);

        if (obj == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("int32Value"), obj->int32Value);
        mla_serializer_write_bool(serializer, mla_string_const("boolValue"), obj->boolValue);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t &deserializer, mla_pointer_t& config,
                                                      const mla_string_t &property_name) {
        mla_all_types_inner_struct *obj = mla_pointer_get_data<mla_all_types_inner_struct>(config);

        if (obj == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "int32Value")) {
            mla_deserializer_read_int32(deserializer, obj->int32Value);
        } else if (mla_string_equals_const(property_name, "boolValue")) {
            mla_deserializer_read_bool(deserializer, obj->boolValue);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};


struct mla_all_types_struct {
    mla_bool_t boolValue;
    mla_int8_t int8Value;
    mla_int16_t int16Value;
    mla_int32_t int32Value;
    mla_int64_t int64Value;
    mla_uint8_t uint8Value;
    mla_uint16_t uint16Value;
    mla_uint32_t uint32Value;
    mla_uint64_t uint64Value;
    mla_float_t floatValue;
    mla_double_t doubleValue;
    mla_string_t stringValue;
    mla_bytes_t bytes;
    mla_all_types_inner_struct innerStruct;
    mla_array_list_t<mla_int32_t> intList;
    mla_array_list_t<mla_all_types_inner_struct> innerStructList;


    static mla_bool_t serialize(mla_serializer_t &serializer, const mla_pointer_t& config) {
        const mla_all_types_struct *obj = mla_pointer_get_data<const mla_all_types_struct>(config);

        if (obj == nullptr)
            return false;

        mla_serializer_write_bool(serializer, mla_string_const("boolValue"), obj->boolValue);
        mla_serializer_write_int8(serializer, mla_string_const("int8Value"), obj->int8Value);
        mla_serializer_write_int16(serializer, mla_string_const("int16Value"), obj->int16Value);
        mla_serializer_write_int32(serializer, mla_string_const("int32Value"), obj->int32Value);
        mla_serializer_write_int64(serializer, mla_string_const("int64Value"), obj->int64Value);
        mla_serializer_write_uint8(serializer, mla_string_const("uint8Value"), obj->uint8Value);
        mla_serializer_write_uint16(serializer, mla_string_const("uint16Value"), obj->uint16Value);
        mla_serializer_write_uint32(serializer, mla_string_const("uint32Value"), obj->uint32Value);
        mla_serializer_write_uint64(serializer, mla_string_const("uint64Value"), obj->uint64Value);
        mla_serializer_write_float(serializer, mla_string_const("floatValue"), obj->floatValue);
        mla_serializer_write_double(serializer, mla_string_const("doubleValue"), obj->doubleValue);
        mla_serializer_write_string(serializer, mla_string_const("stringValue"), obj->stringValue);
        mla_serializer_write_bytes(serializer, mla_string_const("bytes"), obj->bytes);

        // Inner struct
        mla_serializer_write_struct(serializer, mla_string_const("innerStruct"), obj->innerStruct,
                                    mla_all_types_inner_struct);

        // Int list
        mla_serializer_write_list(serializer, mla_string_const("intList"), obj->intList);

        // Inner struct list
        mla_serializer_write_list_struct(serializer, mla_string_const("innerStructList"), obj->innerStructList,
                                         mla_all_types_inner_struct);

        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t &deserializer, mla_pointer_t& config,
                                                      const mla_string_t &property_name) {
        mla_all_types_struct *obj = mla_pointer_get_data<mla_all_types_struct>(config);

        if (obj == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "boolValue")) {
            mla_deserializer_read_bool(deserializer, obj->boolValue);
        } else if (mla_string_equals_const(property_name, "int8Value")) {
            mla_deserializer_read_int8(deserializer, obj->int8Value);
        } else if (mla_string_equals_const(property_name, "int16Value")) {
            mla_deserializer_read_int16(deserializer, obj->int16Value);
        } else if (mla_string_equals_const(property_name, "int32Value")) {
            mla_deserializer_read_int32(deserializer, obj->int32Value);
        } else if (mla_string_equals_const(property_name, "int64Value")) {
            mla_deserializer_read_int64(deserializer, obj->int64Value);
        } else if (mla_string_equals_const(property_name, "uint8Value")) {
            mla_deserializer_read_uint8(deserializer, obj->uint8Value);
        } else if (mla_string_equals_const(property_name, "uint16Value")) {
            mla_deserializer_read_uint16(deserializer, obj->uint16Value);
        } else if (mla_string_equals_const(property_name, "uint32Value")) {
            mla_deserializer_read_uint32(deserializer, obj->uint32Value);
        } else if (mla_string_equals_const(property_name, "uint64Value")) {
            mla_deserializer_read_uint64(deserializer, obj->uint64Value);
        } else if (mla_string_equals_const(property_name, "floatValue")) {
            mla_deserializer_read_float(deserializer, obj->floatValue);
        } else if (mla_string_equals_const(property_name, "doubleValue")) {
            mla_deserializer_read_double(deserializer, obj->doubleValue);
        } else if (mla_string_equals_const(property_name, "stringValue")) {
            mla_deserializer_read_string(deserializer, obj->stringValue);
        } else if (mla_string_equals_const(property_name, "bytes")) {
            mla_deserializer_read_bytes(deserializer, obj->bytes);
        } else if (mla_string_equals_const(property_name, "innerStruct")) {
            mla_deserializer_read_struct(deserializer, obj->innerStruct, mla_all_types_inner_struct);
        } else if (mla_string_equals_const(property_name, "intList")) {
            if (mla_serializer_read_list(deserializer, obj->intList)) {
                return MLA_DESERIALIZER_READ_HANDLED;
            } else {
                return MLA_DESERIALIZER_READ_ERROR;
            }
        } else if (mla_string_equals_const(property_name, "innerStructList")) {
            mla_deserializer_read_list_struct(deserializer, obj->innerStructList, mla_all_types_inner_struct);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};


inline void AllTypesTest(mla_serializer_t &serializer, mla_deserializer_t &deserializer) {
    if (mla_serializer_is_invalid(serializer)) {
        assert_fail("Serializer is invalid");
        return;
    }

    if (mla_deserializer_is_invalid(deserializer)) {
        assert_fail("Deserializer is invalid");
        return;
    }

    mla_all_types_struct original = {
        true,
        -8,
        -16,
        -32,
        -64,
        8,
        16,
        32,
        64,
        3.14f,
        3.141592653589793,
        mla_string("Test String"),
        mla_bytes(5),
        {0, false},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list<mla_all_types_inner_struct>()
    };

    mla_byte_t *bufferInner = mla_bytes_get_data_for_writing(original.bytes);

    if (bufferInner != nullptr) {
        bufferInner[0] = 1;
        bufferInner[1] = 2;
        bufferInner[2] = 3;
        bufferInner[3] = 4;
        bufferInner[4] = 5;
    } else {
        assert_fail("Failed to get bytes data for writing");
    }

    mla_array_list_add(original.intList, (mla_int32_t) 1);
    mla_array_list_add(original.intList, (mla_int32_t) 2);
    mla_array_list_add(original.intList, (mla_int32_t) 3);

    mla_array_list_add(original.innerStructList, {1, false});
    mla_array_list_add(original.innerStructList, {2, true});
    mla_array_list_add(original.innerStructList, {3, false});

    mla_pointer_t original_ptr = mla_platform_pointer_to_managed_pointer(&original);
    assert_true(mla_serializer_write_data_struct(serializer, original_ptr, mla_all_types_struct::serialize),
                "Failed to serialize data");


    mla_all_types_struct deserialized = {
        false,
        -1,
        -1,
        -1,
        -1,
        1,
        1,
        1,
        1,
        1.0f,
        1.0,
        mla_string_empty(),
        mla_bytes_empty(),
        {6, true},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list_empty<mla_all_types_inner_struct>()
    };

    // Start reading
    assert_true(deserializer.read_next(deserializer), "Failed to read next data from deserializer");

    mla_pointer_t deserialized_ptr = mla_platform_pointer_to_managed_pointer(&deserialized);
    assert_true(
        mla_deserializer_read_struct_read_function(deserializer, deserialized_ptr, mla_all_types_struct::deserialize),
        "Deserialization failed");

    // Compare original and deserialized
    assert_equal(original.boolValue, deserialized.boolValue, "Value 'boolValue' does not match");
    assert_equal(original.int8Value, deserialized.int8Value, "Value 'int8Value' does not match");
    assert_equal(original.int16Value, deserialized.int16Value, "Value 'int16Value' does not match");
    assert_equal(original.int32Value, deserialized.int32Value, "Value 'int32Value' does not match");
    assert_equal(original.int64Value, deserialized.int64Value, "Value 'int64Value' does not match");
    assert_equal(original.uint8Value, deserialized.uint8Value, "Value 'uint8Value' does not match");
    assert_equal(original.uint16Value, deserialized.uint16Value, "Value 'uint16Value' does not match");
    assert_equal(original.uint32Value, deserialized.uint32Value, "Value 'uint32Value' does not match");
    assert_equal(original.uint64Value, deserialized.uint64Value, "Value 'uint64Value' does not match");
    assert_equal(original.floatValue, deserialized.floatValue, "Value 'floatValue' does not match");
    assert_equal(original.doubleValue, deserialized.doubleValue, "Value 'doubleValue' does not match");
    assert_struct_equal(mla_string_t, original.stringValue, deserialized.stringValue,
                        "Value 'stringValue' does not match");

    // Compare bytes
    assert_equal(original.bytes.size, deserialized.bytes.size, "Value 'bytesSize' does not match");

    if (original.bytes.size == deserialized.bytes.size) {
        const mla_byte_t *original_data = mla_bytes_get_data_readonly(original.bytes);
        const mla_byte_t *deserialized_data = mla_bytes_get_data_readonly(deserialized.bytes);

        for (mla_size_t i = 0; i < original.bytes.size; ++i) {
            assert_equal(original_data[i], deserialized_data[i], "Value 'bytes' at index does not match");
        }
    }

    // Compare inner struct
    assert_equal(original.innerStruct.int32Value, deserialized.innerStruct.int32Value,
                 "Value 'innerStruct.int32Value' does not match");
    assert_equal(original.innerStruct.boolValue, deserialized.innerStruct.boolValue,
                 "Value 'innerStruct.boolValue' does not match");

    // Compare int list
    assert_equal(mla_array_list_size(original.intList), mla_array_list_size(deserialized.intList),
                 "Size of 'intList' does not match");

    if (mla_array_list_size(original.intList) == mla_array_list_size(deserialized.intList)) {
        for (mla_size_t i = 0; i < mla_array_list_size(original.intList); ++i) {
            assert_equal(*mla_array_list_get_ref(original.intList, i), *mla_array_list_get_ref(deserialized.intList, i),
                         "Value 'intList' at index does not match");
        }
    }


    // Compare inner struct list
    assert_equal(mla_array_list_size(original.innerStructList), mla_array_list_size(deserialized.innerStructList),
                 "Size of 'innerStructList' does not match");

    if (mla_array_list_size(original.innerStructList) == mla_array_list_size(deserialized.innerStructList)) {
        for (mla_size_t i = 0; i < mla_array_list_size(original.innerStructList); ++i) {
            mla_all_types_inner_struct *originalItem = mla_array_list_get_ref(original.innerStructList, i);
            mla_all_types_inner_struct *deserializedItem = mla_array_list_get_ref(deserialized.innerStructList, i);
            assert_equal(originalItem->int32Value, deserializedItem->int32Value,
                         "Value 'innerStructList.int32Value' at index does not match");
            assert_equal(originalItem->boolValue, deserializedItem->boolValue,
                         "Value 'innerStructList.boolValue' at index does not match");
        }
    }
}

inline void BinarySerializerAllTypesTest() {
    mla_stream_input_t stream_input = mla_stream_input_from_buffer(mla_serializer_buffer, 1024);
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    mla_serializer_t serializer = mla_binary_serializer(stream_output);
    mla_deserializer_t deserializer = mla_binary_deserializer(stream_input);
    AllTypesTest(serializer, deserializer);
}


inline void JsonSerializerAllTypesTest() {
    mla_stream_input_t stream_input = mla_stream_input_from_buffer(mla_serializer_buffer, 1024);
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    mla_serializer_t serializer = mla_json_serializer(stream_output);
    mla_deserializer_t deserializer = mla_json_deserializer(stream_input);
    AllTypesTest(serializer, deserializer);
}

inline void XmlSerializerAllTypesTest() {
    mla_stream_input_t stream_input = mla_stream_input_from_buffer(mla_serializer_buffer, 1024);
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    mla_serializer_t serializer = mla_xml_serializer(stream_output);
    mla_deserializer_t deserializer = mla_xml_deserializer(stream_input);
    AllTypesTest(serializer, deserializer);
}

void RegisterSerializerTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("BinarySerializerAllTypes", test_category, BinarySerializerAllTypesTest,
                               SetupSerializerTest, TearDownSerializerTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("JsonSerializerAllTypes", test_category, JsonSerializerAllTypesTest, SetupSerializerTest,
                    TearDownSerializerTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("XmlSerializerAllTypes", test_category, XmlSerializerAllTypesTest, SetupSerializerTest,
                    TearDownSerializerTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}


//////////////////////////////////////////////////////////////////////////
/// Benchmarks Helpers
//////////////////////////////////////////////////////////////////////////


static mla_all_types_struct g_benchmarkAllTypes = {
    false,
    -1,
    -1,
    -1,
    -1,
    1,
    1,
    1,
    1,
    1.0f,
    1.0,
    mla_string_empty(),
    mla_bytes_empty(),
    {0, false},
    mla_array_list_empty<mla_int32_t>(),
    mla_array_list_empty<mla_all_types_inner_struct>()
};

inline void SetupSerializerBenchmark() {
    mla_serializer_buffer = static_cast<mla_byte_t *>(mla_platform_malloc(1024));
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    g_benchmarkAllTypes = {
        true,
        -8,
        -16,
        -32,
        -64,
        8,
        16,
        32,
        64,
        3.14f,
        3.141592653589793,
        mla_string("Test String"),
        mla_bytes(5),
        {0, false},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list<mla_all_types_inner_struct>()
    };

    mla_byte_t *bufferInner = mla_bytes_get_data_for_writing(g_benchmarkAllTypes.bytes);

    bufferInner[0] = 1;
    bufferInner[1] = 2;
    bufferInner[2] = 3;
    bufferInner[3] = 4;
    bufferInner[4] = 5;

    mla_array_list_add(g_benchmarkAllTypes.intList, (mla_int32_t) 1);
    mla_array_list_add(g_benchmarkAllTypes.intList, (mla_int32_t) 2);
    mla_array_list_add(g_benchmarkAllTypes.intList, (mla_int32_t) 3);

    mla_array_list_add(g_benchmarkAllTypes.innerStructList, {1, false});
    mla_array_list_add(g_benchmarkAllTypes.innerStructList, {2, true});
    mla_array_list_add(g_benchmarkAllTypes.innerStructList, {3, false});
}

inline void TearDownSerializerBenchmark() {
    g_benchmarkAllTypes = {
        false,
        -1,
        -1,
        -1,
        -1,
        1,
        1,
        1,
        1,
        1.0f,
        1.0,
        mla_string_empty(),
        mla_bytes_empty(),
        {0, false},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list_empty<mla_all_types_inner_struct>()
    };

    mla_platform_free(mla_serializer_buffer);
}


inline void SetupDeserializerBenchmark(mla_serializer_t serializer) {
    mla_all_types_struct prepare_benchmarkAllTypes = {
        true,
        -8,
        -16,
        -32,
        -64,
        8,
        16,
        32,
        64,
        3.14f,
        3.141592653589793,
        mla_string("Test String"),
        mla_bytes(5),
        {0, false},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list<mla_all_types_inner_struct>()
    };

    mla_byte_t *bufferInner = mla_bytes_get_data_for_writing(prepare_benchmarkAllTypes.bytes);

    bufferInner[0] = 1;
    bufferInner[1] = 2;
    bufferInner[2] = 3;
    bufferInner[3] = 4;
    bufferInner[4] = 5;

    mla_array_list_add(prepare_benchmarkAllTypes.intList, (mla_int32_t) 1);
    mla_array_list_add(prepare_benchmarkAllTypes.intList, (mla_int32_t) 2);
    mla_array_list_add(prepare_benchmarkAllTypes.intList, (mla_int32_t) 3);

    mla_array_list_add(prepare_benchmarkAllTypes.innerStructList, {1, false});
    mla_array_list_add(prepare_benchmarkAllTypes.innerStructList, {2, true});
    mla_array_list_add(prepare_benchmarkAllTypes.innerStructList, {3, false});


    mla_pointer_t prepare_benchmarkAllTypes_ptr = mla_platform_pointer_to_managed_pointer(&prepare_benchmarkAllTypes);
    mla_serializer_write_data_struct(serializer, prepare_benchmarkAllTypes_ptr, mla_all_types_struct::serialize);
}

inline void TearDownDeserializerBenchmark() {
    g_benchmarkAllTypes = {
        false,
        -1,
        -1,
        -1,
        -1,
        1,
        1,
        1,
        1,
        1.0f,
        1.0,
        mla_string_empty(),
        mla_bytes_empty(),
        {0, false},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list_empty<mla_all_types_inner_struct>()
    };
    mla_platform_free(mla_serializer_buffer);
}


inline void AllTypesSerializerBenchmark(mla_serializer_t serializer) {
    mla_pointer_t data_ptr = mla_platform_pointer_to_managed_pointer(&g_benchmarkAllTypes);
    mla_serializer_write_data_struct(serializer, data_ptr, mla_all_types_struct::serialize);
}

inline void AllTypesDeserializerBenchmark(mla_deserializer_t deserializer) {
    deserializer.read_next(deserializer);
    mla_pointer_t data_ptr = mla_platform_pointer_to_managed_pointer(&g_benchmarkAllTypes);
    mla_deserializer_read_struct_read_function(deserializer, data_ptr,
                                               mla_all_types_struct::deserialize);
    g_benchmarkAllTypes = {
        false,
        -1,
        -1,
        -1,
        -1,
        1,
        1,
        1,
        1,
        1.0f,
        1.0,
        mla_string_empty(),
        mla_bytes_empty(),
        {0, false},
        mla_array_list_empty<mla_int32_t>(),
        mla_array_list_empty<mla_all_types_inner_struct>()
    };
}

//////////////////////////////////////////////////////////////////////////
/// Benchmarks
//////////////////////////////////////////////////////////////////////////


inline void BinarySerializerBenchmark() {
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    mla_serializer_t serializer = mla_binary_serializer(stream_output);
    AllTypesSerializerBenchmark(serializer);
}


inline void SetupBinaryDeserializerBenchmark() {
    mla_serializer_buffer = static_cast<mla_byte_t *>(mla_platform_malloc(1024));
    mla_stream_output_t prepare_stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);
    mla_serializer_t serializer = mla_binary_serializer(prepare_stream_output);
    SetupDeserializerBenchmark(serializer);
}


inline void BinaryDeserializerBenchmark() {
    mla_stream_input_t stream_input = mla_stream_input_from_buffer(mla_serializer_buffer, 1024);

    mla_deserializer_t deserializer = mla_binary_deserializer(stream_input);
    AllTypesDeserializerBenchmark(deserializer);
}

inline void JsonSerializerBenchmark() {
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    mla_serializer_t serializer = mla_json_serializer(stream_output);
    AllTypesSerializerBenchmark(serializer);
}

inline void SetupJsonDeserializerBenchmark() {
    mla_serializer_buffer = static_cast<mla_byte_t *>(mla_platform_malloc(1024));
    mla_stream_output_t prepare_stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);
    mla_serializer_t serializer = mla_json_serializer(prepare_stream_output);
    SetupDeserializerBenchmark(serializer);
}

inline void JsonDeserializerBenchmark() {
    mla_stream_input_t stream_input = mla_stream_input_from_buffer(mla_serializer_buffer, 1024);

    mla_deserializer_t deserializer = mla_json_deserializer(stream_input);
    AllTypesDeserializerBenchmark(deserializer);
}

inline void XmlSerializerBenchmark() {
    mla_stream_output_t stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);

    mla_serializer_t serializer = mla_xml_serializer(stream_output);
    AllTypesSerializerBenchmark(serializer);
}

inline void SetupXmlDeserializerBenchmark() {
    mla_serializer_buffer = static_cast<mla_byte_t *>(mla_platform_malloc(1024));
    mla_stream_output_t prepare_stream_output = mla_stream_output_to_buffer(mla_serializer_buffer, 1024);
    mla_serializer_t serializer = mla_xml_serializer(prepare_stream_output);
    SetupDeserializerBenchmark(serializer);
}

inline void XmlDeserializerBenchmark() {
    mla_stream_input_t stream_input = mla_stream_input_from_buffer(mla_serializer_buffer, 1024);

    mla_deserializer_t deserializer = mla_xml_deserializer(stream_input);
    AllTypesDeserializerBenchmark(deserializer);
}

void RegisterSerializerBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    mla_benchmark_t benchmark = mla_benchmark("BinarySerializer", benchmark_category, BinarySerializerBenchmark,
                                              SetupSerializerBenchmark, TearDownSerializerBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("BinaryDeserializer", benchmark_category, BinaryDeserializerBenchmark,
                              SetupBinaryDeserializerBenchmark, TearDownDeserializerBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);


    benchmark = mla_benchmark("JsonSerializer", benchmark_category, JsonSerializerBenchmark, SetupSerializerBenchmark,
                              TearDownSerializerBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("JsonDeserializer", benchmark_category, JsonDeserializerBenchmark,
                              SetupJsonDeserializerBenchmark, TearDownDeserializerBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("XmlSerializer", benchmark_category, XmlSerializerBenchmark, SetupSerializerBenchmark,
                              TearDownSerializerBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("XmlDeserializer", benchmark_category, XmlDeserializerBenchmark,
                              SetupXmlDeserializerBenchmark, TearDownDeserializerBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif
