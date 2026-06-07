#ifndef MLA_HTTP_MULTIPART_TEST_H
#define MLA_HTTP_MULTIPART_TEST_H

#include "../../lib/base-lib/core/http/mla_http_multipart.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

void HttpMultipartClientTest() {
    mla_string_t file_data = mla_string_const("dummy file data");
    mla_stream_input_t stream = mla_stream_input_from_string(file_data);
    
    mla_http_client_t client = mla_http_client();
    client.timeout_ms = 1; 

    mla_http_client_response_t response = mla_http_client_upload_file(
        client,
        mla_string_const("http://example.com/upload"),
        mla_string_const("file"),
        mla_string_const("test.txt"),
        mla_string_const("text/plain"),
        stream
    );

    assert_true(response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, "Expected connection failure with 1ms timeout for unreachable URL");
    
    mla_http_client_response_destroy(response);
}

void RegisterHttpMultipartTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("HttpMultipartClientTest", test_category, HttpMultipartClientTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
