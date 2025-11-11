//
// Created by chris on 10/23/2025.
//

#ifndef COREOS_MLA_HTTP_SERVER_TEST_H
#define COREOS_MLA_HTTP_SERVER_TEST_H

#include "../core-os/system/mla_stream.h"
#include "../core-os/http/mla_http_client.h"
#include "../core-os/http/mla_http_server.h"
#include "../core-os/system/mla_string_concat.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

static mla_network_host_t test_server_host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 41258);
static mla_string_t test_server_url = mla_string_const("http://127.0.0.1:41258");

inline mla_bool_t mla_http_server_request_hello_world_handler(const mla_http_request_t &request, mla_http_response_t &response) {

    (void)request;
    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");

    return true;
}

inline mla_bool_t mla_http_server_request_echo_handler(const mla_http_request_t &request, mla_http_response_t &response) {

    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    response.content = request.content;

    return true;
}

inline mla_bool_t mla_http_server_request_test_handler(const mla_http_request_t &request, mla_http_response_t &response) {

    (void)request;
    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    const mla_char_t* body = "test";
    response.content = mla_stream_input_from_buffer((mla_byte_t*)body, sizeof(mla_char_t) * 4);

    return true;
}

inline void StartSimpleHttpServerTest() {



    mla_http_server_t server = mla_http_server(test_server_host);
    mla_http_server_handler_item_t handlerItem = mla_http_server_handler_all(mla_http_method_get, mla_http_server_request_hello_world_handler);
    assert_true(mla_http_server_register_handler(server, handlerItem), "Should register hello world handler");
    assert_true(mla_http_server_start(server, 1), "Should start simple HTTP server");

    mla_http_request_t request = mla_http_get_request(test_server_url);
    mla_http_client_response_t response = mla_http_client_send_request(request);
    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP request to simple server should succeed");
    assert_equal(response.response.statusCode, mla_http_status_ok, "Should receive 200 OK from simple server");

    server = mla_http_server_invalid();
}

inline void HttpServerMultiHandlerTest() {

    mla_http_server_t server = mla_http_server(test_server_host);
    mla_http_server_handler_item_t handlerItem1 = mla_http_server_handler_starts_with(mla_http_method_get, mla_string_const("/test"), mla_http_server_request_test_handler);
    assert_true(mla_http_server_register_handler(server, handlerItem1), "Should register test handler");
    mla_http_server_handler_item_t handlerItem2 = mla_http_server_handler_starts_with(mla_http_method_post, mla_string_const("/echo"), mla_http_server_request_echo_handler);
    assert_true(mla_http_server_register_handler(server, handlerItem2), "Should register echo handler");
    assert_true(mla_http_server_start(server, 2), "Should start multi handler HTTP server");

    // Test GET request
    mla_string_t test_url = mla_string_concat(test_server_url, mla_string_const("/test"));
    mla_http_request_t request1 = mla_http_get_request(test_url);
    mla_http_client_response_t response1 = mla_http_client_send_request(request1);
    assert_equal(response1.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP GET request to multi handler server should succeed");
    assert_equal(response1.response.statusCode, mla_http_status_ok, "Should receive 200 OK from multi handler server");

    mla_char_t body_buffer[5] = {0};
    response1.response.content.read(response1.response.content,0, 4, (mla_byte_t*)body_buffer);
    mla_string_t body_string = mla_string_from_buffer_without_ownership(body_buffer, 4);
    assert_struct_equal(mla_string_t, mla_string_const("test"), body_string, "Should receive 'test' body from multi handler server");

    // Test POST request
    mla_string_t echo_url = mla_string_concat(test_server_url, mla_string_const("/echo"));
    mla_http_request_t request2 = mla_http_post_request(echo_url);
    request2.content = mla_stream_input_from_buffer((mla_byte_t*)"hello world", 12);
    mla_http_client_response_t response2 = mla_http_client_send_request(request2);
    assert_equal(response2.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP POST request to multi handler server should succeed");
    assert_equal(response2.response.statusCode, mla_http_status_ok, "Should receive 200 OK from multi handler server");

    mla_char_t body_2_buffer[12] = {0};
    response2.response.content.read(response2.response.content,0, 12, (mla_byte_t*)body_2_buffer);
    mla_string_t body_2_string = mla_string_from_buffer_without_ownership(body_2_buffer, 11);

    assert_struct_equal(mla_string_t, mla_string_const("hello world"), body_2_string, "Should receive echoed body from multi handler server");

    // Test Not Found
    mla_string_t not_found_url = mla_string_concat(test_server_url, mla_string_const("/not_found"));
    mla_http_request_t request3 = mla_http_get_request(not_found_url);
    mla_http_client_response_t response3 = mla_http_client_send_request(request3);
    assert_equal(response3.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP GET request to non-existent path should succeed");
    assert_equal(response3.response.statusCode, mla_http_status_not_found, "Should receive 404 Not Found from multi handler server");

    server = mla_http_server_invalid();
}

void RegisterHttpServerTests(mla_test_executor_t &p_TestExecutor) {

    // Only run HTTP server tests in native multi-tasking environments
    // Because it need two threads to run the server and the client simultaneously
    if (mla_is_native_multi_tasking) {


        mla_test_t test = mla_test("StartSimpleHttpServer", test_category, StartSimpleHttpServerTest);
        mla_test_executor_register_test(p_TestExecutor, test);

        test = mla_test("HttpServerMultiHandler", test_category, HttpServerMultiHandlerTest);
        mla_test_executor_register_test(p_TestExecutor, test);
    }


}



static mla_http_server_t test_server = mla_http_server_invalid();

void StartSimpleHttpServerTest_Setup() {

    test_server = mla_http_server(test_server_host);
    mla_http_server_handler_item_t handlerItem = mla_http_server_handler_all(mla_http_method_get, mla_http_server_request_hello_world_handler);
    mla_http_server_register_handler(test_server, handlerItem);
    mla_http_server_start(test_server, 1);
}

void StartSimpleHttpServerTest_TearDown() {

    mla_http_server_stop(test_server);
    test_server = mla_http_server_invalid();
    mla_task_manager_cleanup();
}

void SimpleHttpServerBenchmark() {

    mla_http_request_t request = mla_http_get_request(test_server_url);
    mla_http_client_response_t response = mla_http_client_send_request(request);

    if (response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
        mla_error(mla_string_concat("HTTP request to simple server failed with status: ",
                                    mla_string_from_uint32((mla_uint32_t)response.status)));
    }
}

void RegisterHttpServerBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    // Concat Benchmarks
    ////////////////////////////////////////////
    mla_benchmark_t benchmark = mla_benchmark("SimpleHttpServer", benchmark_category, SimpleHttpServerBenchmark,
                                             StartSimpleHttpServerTest_Setup,
                                             StartSimpleHttpServerTest_TearDown);
    mla_benchmark_set_iteration_division(benchmark, 1000);

    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}

#endif