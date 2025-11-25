//
// Created by christian on 11/24/25.
//

#ifndef COREOS_MLA_HTTP_RPC_TEST_H
#define COREOS_MLA_HTTP_RPC_TEST_H

#include "../core-os/rpc/mla_rpc.h"
#include "../core-os/rpc/mla_rpc_http_server.h"
#include "../core-os/rpc/mla_rpc_http_client.h"
#include "../core-os-test-support/mla_test_executor.h"

static mla_network_host_t rpc_test_server_host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 41258);
static mla_string_t rpc_test_server_url = mla_string_const("http://127.0.0.1:41258");


struct my_http_rpc_test_input_t {
    mla_int32_t a;
    mla_int32_t b;

    static void serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {

        const my_http_rpc_test_input_t* input = static_cast<const my_http_rpc_test_input_t*>(obj);
        mla_serializer_write_int32(serializer, mla_string_const("a"), input->a);
        mla_serializer_write_int32(serializer, mla_string_const("b"), input->b);

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {

        my_http_rpc_test_input_t* input = static_cast<my_http_rpc_test_input_t*>(obj);

        if (mla_string_equals_const(property_name, "a")) {
            mla_deserializer_read_int32(deserializer, input->a);
        } else if (mla_string_equals_const(property_name, "b")) {
            mla_deserializer_read_int32(deserializer, input->b);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }

    }
};


struct my_http_rpc_test_output_t {
    mla_int32_t sum;

    static void serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {

        const my_http_rpc_test_output_t* output = static_cast<const my_http_rpc_test_output_t*>(obj);
        mla_serializer_write_int32(serializer, mla_string_const("sum"), output->sum);

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {

        my_http_rpc_test_output_t* output = static_cast<my_http_rpc_test_output_t*>(obj);

        if (mla_string_equals_const(property_name, "sum")) {
            mla_deserializer_read_int32(deserializer, output->sum);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }
};

mla_bool_t my_http_rpc_test_procedure_handler(const my_http_rpc_test_input_t* input, my_http_rpc_test_output_t* output) {

    output->sum = input->a + input->b;
    return true;
}

#define mla_http_rpc_procedure_math_sum_name "http/math/sum"
#define mla_http_rpc_procedure_math_sum_signature my_rpc_test_input_t, my_rpc_test_output_t

mla_rpc_auto_register_procedure(mla_http_rpc_procedure_math_sum_name, my_http_rpc_test_input_t, my_http_rpc_test_output_t, my_http_rpc_test_procedure_handler)

inline void ExecuteHttpRpcProcedureTest() {

    mla_http_server_t server = mla_http_server(rpc_test_server_host);
    assert_true(mla_rpc_http_server_initialize(server), "Should initialize HTTP RPC server");
    assert_true(mla_http_server_start(server, 1), "Should start simple HTTP server");
    mla_rpc_remote_endpoint_t endpoint = mla_rpc_http_register_endpoint(mla_string_concat(rpc_test_server_url, "/rpc/"), mla_http_rpc_content_type_binary);
    assert_true(mla_rpc_remote_endpoint_valid(endpoint), "Should create valid HTTP RPC remote endpoint");
    assert_true(mla_rpc_register_remote_endpoint(endpoint), "Should register HTTP RPC remote endpoint");

    my_http_rpc_test_input_t input = { 3, 4 };
    my_http_rpc_test_output_t output = { 0 };

    assert_true(mla_rpc_execute_procedure_remote(mla_string_const(mla_http_rpc_procedure_math_sum_name), &input, &output), "Failed to execute RPC procedure");
    assert_equal(output.sum, 7, "RPC procedure returned incorrect sum");

    assert_true(mla_rpc_unregister_remote_endpoint(endpoint), "Should unregister HTTP RPC remote endpoint");

    server = mla_http_server_invalid();

}

void RegisterHttpRpcTests(mla_test_executor_t &p_TestExecutor) {

    (void)p_TestExecutor;
    //mla_test_t test = mla_test("ExecuteRpcProcedure", test_category, ExecuteHttpRpcProcedureTest);
    //mla_test_executor_register_test(p_TestExecutor, test);

}

#endif