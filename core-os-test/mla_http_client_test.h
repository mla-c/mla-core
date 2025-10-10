//
// Created by chris on 10/10/2025.
//

#ifndef COREOS_MLA_HTTP_CLIENT_TEST_H
#define COREOS_MLA_HTTP_CLIENT_TEST_H

#include "../core-os/http/mla_http_client.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

void SimpleGetRequestTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Send the request
    mla_http_client_response_t response = mla_http_client_send_request(request);

    // Verify the response status
    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP request should succeed");

    // Verify we got a valid HTTP response
    assert_equal(response.response.statusCode, mla_http_status_ok,
                "Should receive a valid HTTP status code");

    // Verify content type header
    mla_string_t contentType = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Type"));
    assert_struct_equal(mla_string_t, contentType, mla_string_const("text/html"),
                "Content-Type should be text/html");

    // Check that we received some content
    mla_byte_t buffer[64];

    if (response.response.content.read != nullptr) {
        mla_size_t readed = response.response.content.read(response.response.content, 0, sizeof(buffer), buffer);
        assert_true(readed > 0, "Should read some bytes from response content");

        mla_string_t content = mla_string_from_buffer_without_ownership((mla_char_t *)buffer, 64);
        assert_true(mla_string_contains(content, mla_string_const("<html")),"Response content should contain HTML");

        mla_size_t totalRead = readed;
        // Read all content (for cleanup)
        while (readed > 0) {
            readed = response.response.content.read(response.response.content, 0, sizeof(buffer), buffer);
            totalRead += readed;
        }

        // Get total content length from headers
        mla_string_t contentLengthStr = mla_http_headers_get_value(response.response.headers, mla_string_const("Content-Length"));

        mla_size_t contentLength = 0;

        mla_parse_uint32(contentLengthStr, contentLength);
        assert_equal(totalRead, contentLength, "Total read bytes should match Content-Length header");

    } else {
        assert_fail("Response content read function is null");
    }

    // Clean up
    mla_http_client_response_destroy(response);
}

void GetRequestWithHeadersTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Add custom headers
    mla_http_headers_add(request.headers, mla_string("User-Agent"), mla_string("MLA-HTTP-Client/1.0"));
    mla_http_headers_add(request.headers, mla_string("Accept"), mla_string("text/html"));

    // Send the request
    mla_http_client_response_t response = mla_http_client_send_request(request);

    // Verify the response status
    assert_equal(response.status, MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "HTTP request should succeed");

    // Verify we got a valid HTTP response
    assert_equal(response.response.statusCode, mla_http_status_ok,
                "Should receive a valid HTTP status code");

    // Clean up
    mla_http_client_response_destroy(response);
}

void RequestTimeoutTest() {

    // Create a simple GET request to a test site
    mla_http_request_t request = mla_http_request(mla_string_const("http://www.testingmcafeesites.com"), mla_http_method_get);

    // Create client with very short timeout
    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 1; // 1ms timeout - should fail

    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    // Should timeout or fail
    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
                "Request with 1ms timeout should fail");

    mla_http_client_response_destroy(response);
}

void RegisterHttpClientTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleGetRequest", test_category, SimpleGetRequestTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("GetRequestWithHeaders", test_category, GetRequestWithHeadersTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RequestTimeout", test_category, RequestTimeoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}


#endif