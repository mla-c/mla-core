//
// Created by christian on 11/24/25.
//

#ifndef MLA_HTTP_RPC_TEST_H
#define MLA_HTTP_RPC_TEST_H

#include "../core-os/rpc/mla_rpc.h"
#include "../core-os/rpc/mla_rpc_http_server.h"
#include "../core-os/rpc/mla_rpc_http_client.h"
#include "../core-os-test-support/mla_test_executor.h"

static mla_network_host_t rpc_test_server_host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 41258);
static mla_string_t rpc_test_server_url = mla_string_const("http://127.0.0.1:41258");

struct my_http_rpc_test_large_data_t {
    mla_string_t large_string;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const my_http_rpc_test_large_data_t* input = mla_pointer_get_data<const my_http_rpc_test_large_data_t>(obj);

        if (input == nullptr)
            return false;

        mla_serializer_write_string(serializer, mla_string_const("large_string"), input->large_string);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        my_http_rpc_test_large_data_t* input = mla_pointer_get_data<my_http_rpc_test_large_data_t>(obj);

        if (input == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "large_string")) {
            mla_deserializer_read_string(deserializer, input->large_string);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_http_rpc_test_large_data_t);
        mla_reflection_struct_field_string(meta, my_http_rpc_test_large_data_t, large_string);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct my_http_rpc_test_input_t {
    mla_int32_t a;
    mla_int32_t b;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const my_http_rpc_test_input_t* input = mla_pointer_get_data<const my_http_rpc_test_input_t>(obj);

        if (input == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("a"), input->a);
        mla_serializer_write_int32(serializer, mla_string_const("b"), input->b);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        my_http_rpc_test_input_t* input = mla_pointer_get_data<my_http_rpc_test_input_t>(obj);

        if (input == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "a")) {
            mla_deserializer_read_int32(deserializer, input->a);
        } else if (mla_string_equals_const(property_name, "b")) {
            mla_deserializer_read_int32(deserializer, input->b);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }

    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_http_rpc_test_input_t);
        mla_reflection_struct_field_int32(meta, my_http_rpc_test_input_t, a);
        mla_reflection_struct_field_int32(meta, my_http_rpc_test_input_t, b);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};


struct my_http_rpc_test_output_t {
    mla_int32_t sum;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const my_http_rpc_test_output_t* output = mla_pointer_get_data<const my_http_rpc_test_output_t>(obj);

        if (output == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("sum"), output->sum);
        return true;

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        my_http_rpc_test_output_t* output = mla_pointer_get_data<my_http_rpc_test_output_t>(obj);

        if (output == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "sum")) {
            mla_deserializer_read_int32(deserializer, output->sum);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_http_rpc_test_output_t);
        mla_reflection_struct_field_int32(meta, my_http_rpc_test_output_t, sum);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

mla_bool_t my_http_rpc_test_procedure_handler(const my_http_rpc_test_input_t* input, my_http_rpc_test_output_t* output) {

    output->sum = input->a + input->b;
    return true;
}

mla_bool_t my_http_rpc_test_large_procedure_handler(const my_http_rpc_test_large_data_t* input, my_http_rpc_test_large_data_t* output) {

    output->large_string = input->large_string;
    return true;
}

#define mla_http_rpc_procedure_math_sum_name "http/math/sum"
#define mla_http_rpc_procedure_math_sum_signature my_rpc_test_input_t, my_rpc_test_output_t
mla_rpc_auto_register_procedure(mla_http_rpc_procedure_math_sum_name, my_http_rpc_test_input_t, my_http_rpc_test_output_t, my_http_rpc_test_procedure_handler)

#define mla_http_rpc_procedure_large_ping_name "http/large/ping"
#define mla_http_rpc_procedure_large_ping_signature my_http_rpc_test_large_data_t, my_http_rpc_test_large_data_t
mla_rpc_auto_register_procedure(mla_http_rpc_procedure_large_ping_name, my_http_rpc_test_large_data_t, my_http_rpc_test_large_data_t, my_http_rpc_test_large_procedure_handler)


inline void ExecuteHttpRpcProcedureTest(mla_http_rpc_content_type content_type) {

    mla_http_server_t server = mla_http_server(rpc_test_server_host);
    assert_true(mla_rpc_http_server_initialize(server), "Should initialize HTTP RPC server");
    assert_true(mla_http_server_start(server, 1), "Should start simple HTTP server");
    mla_rpc_remote_endpoint_t endpoint = mla_rpc_http_register_endpoint(mla_string_concat(rpc_test_server_url, "/rpc/"), content_type);
    assert_true(mla_rpc_remote_endpoint_valid(endpoint), "Should create valid HTTP RPC remote endpoint");

    my_http_rpc_test_input_t input = { 3, 4 };
    my_http_rpc_test_output_t output = { 0 };

    assert_true(mla_rpc_execute_procedure_remote(mla_string_const(mla_http_rpc_procedure_math_sum_name), &input, &output), "Failed to execute RPC procedure");
    assert_equal(output.sum, 7, "RPC procedure returned incorrect sum");

    assert_true(mla_rpc_unregister_remote_endpoint(endpoint), "Should unregister HTTP RPC remote endpoint");

    server = mla_http_server_invalid();

}

inline void ExecuteHttpRpcProcedureBinaryModeTest() {
    ExecuteHttpRpcProcedureTest(mla_http_rpc_content_type_binary);
}

inline void ExecuteHttpRpcProcedureJsonModeTest() {
    ExecuteHttpRpcProcedureTest(mla_http_rpc_content_type_json);
}

inline void ExecuteHttpRpcLargeDataProcedureTest(mla_http_rpc_content_type content_type) {

    mla_http_server_t server = mla_http_server(rpc_test_server_host);
    assert_true(mla_rpc_http_server_initialize(server), "Should initialize HTTP RPC server");
    assert_true(mla_http_server_start(server, 1), "Should start simple HTTP server");
    mla_rpc_remote_endpoint_t endpoint = mla_rpc_http_register_endpoint(mla_string_concat(rpc_test_server_url, "/rpc/"), content_type);
    assert_true(mla_rpc_remote_endpoint_valid(endpoint), "Should create valid HTTP RPC remote endpoint");

    my_http_rpc_test_large_data_t input = {
        mla_string_const("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. "
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. "
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. ")
    };
    my_http_rpc_test_large_data_t output = {
        mla_string_empty()
    };

    assert_true(mla_string_length(input.large_string) > mla_global_config_rpc_stream_small_buffer_size, "Test input large string is not large enough for large data test");

    assert_true(mla_rpc_execute_procedure_remote(mla_string_const(mla_http_rpc_procedure_large_ping_name), &input, &output), "Failed to execute large data RPC procedure");
    assert_equal(mla_string_length(output.large_string), mla_string_length(input.large_string), "RPC large data procedure returned incorrect data length");
    assert_struct_equal(mla_string_t, output.large_string, input.large_string, "RPC large data procedure returned incorrect data");

    assert_true(mla_rpc_unregister_remote_endpoint(endpoint), "Should unregister HTTP RPC remote endpoint");

    server = mla_http_server_invalid();

}

inline void ExecuteHttpRpcLargeDataProcedureBinaryModeTest() {
    ExecuteHttpRpcLargeDataProcedureTest(mla_http_rpc_content_type_binary);
}

inline void ExecuteHttpRpcLargeDataProcedureJsonModeTest() {
    ExecuteHttpRpcLargeDataProcedureTest(mla_http_rpc_content_type_json);
}

void RegisterHttpRpcTests(mla_test_executor_t &p_TestExecutor) {

    // Only run HTTP server tests in native multi-tasking environments
    // Because it need two threads to run the server and the client simultaneously
    if (mla_is_native_multi_tasking) {

        mla_test_t test = mla_test("ExecuteRpcProcedureBinary", test_category, ExecuteHttpRpcProcedureBinaryModeTest);
        mla_test_executor_register_test(p_TestExecutor, test);

        test = mla_test("ExecuteRpcProcedureJson", test_category, ExecuteHttpRpcProcedureJsonModeTest);
        mla_test_executor_register_test(p_TestExecutor, test);

        test = mla_test("ExecuteRpcProcedureLargeDataBinary", test_category, ExecuteHttpRpcLargeDataProcedureBinaryModeTest);
        mla_test_executor_register_test(p_TestExecutor, test);

        test = mla_test("ExecuteRpcProcedureLargeDataJson", test_category, ExecuteHttpRpcLargeDataProcedureJsonModeTest);
        mla_test_executor_register_test(p_TestExecutor, test);
    }

}

#endif