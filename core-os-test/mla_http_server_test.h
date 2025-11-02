//
// Created by chris on 10/23/2025.
//

#ifndef COREOS_MLA_HTTP_SERVER_TEST_H
#define COREOS_MLA_HTTP_SERVER_TEST_H

#include "../core-os/system/mla_stream.h"
#include "../core-os/http/mla_http_client.h"
#include "../core-os/http/mla_http_server.h"
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

void RegisterHttpServerTests(mla_test_executor_t &p_TestExecutor) {

    if (mla_is_native_multi_tasking) {

        // Only run HTTP server tests in native multi-tasking environments
        // Because it need two threads to run the server and the client simultaneously
        mla_test_t test = mla_test("StartSimpleHttpServer", test_category, StartSimpleHttpServerTest);
        mla_test_executor_register_test(p_TestExecutor, test);
    }


}

#endif