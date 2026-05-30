//
// Created by christian on 10/8/25.
//

#ifndef MLA_URL_TEST_H
#define MLA_URL_TEST_H

#include "../lib/base-lib/core/url/mla_url.h"
#include "../lib/base-lib/test-support/mla_test_executor.h"


void BasicUrlParseTest() {
    mla_string_t urlString = mla_string("http://www.example.com/path");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse basic URL");
    assert_struct_equal(mla_string_t, url.scheme, mla_string("http"), "Scheme should be 'http'");
    assert_struct_equal(mla_string_t, url.host, mla_string("www.example.com"), "Host should be 'www.example.com'");
    assert_equal(url.port, (mla_uint16_t)80, "Port should default to 80");
    assert_struct_equal(mla_string_t, url.path, mla_string("/path"), "Path should be '/path'");
}

void UrlWithExplicitPortTest() {
    mla_string_t urlString = mla_string("https://example.com:8443/secure");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse URL with explicit port");
    assert_struct_equal(mla_string_t, url.scheme, mla_string("https"), "Scheme should be 'https'");
    assert_equal(url.port, (mla_uint16_t)8443, "Port should be 8443");
    assert_struct_equal(mla_string_t, url.path, mla_string("/secure"), "Path should be '/secure'");
}

void UrlWithQueryParamsTest() {
    mla_string_t urlString = mla_string("http://example.com/search?q=test&page=1");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse URL with query params");
    assert_struct_equal(mla_string_t, url.path, mla_string("/search"), "Path should be '/search'");
    assert_equal(mla_array_list_size(url.query), (mla_size_t)2, "Should have 2 query parameters");

    if (mla_array_list_size(url.query) == 2) {
        mla_url_query_param_t* param1 = mla_array_list_get_ref(url.query, 0);
        assert_struct_equal(mla_string_t, param1->key, mla_string("q"), "First param key should be 'q'");
        assert_struct_equal(mla_string_t, param1->value, mla_string("test"), "First param value should be 'test'");

        mla_url_query_param_t* param2 = mla_array_list_get_ref(url.query, 1);
        assert_struct_equal(mla_string_t, param2->key, mla_string("page"), "Second param key should be 'page'");
        assert_struct_equal(mla_string_t, param2->value, mla_string("1"), "Second param value should be '1'");
    } else {
        assert_true(false, "Should have 2 query parameters");
    }

}

void UrlWithFragmentTest() {
    mla_string_t urlString = mla_string("https://example.com/page#section");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse URL with fragment");
    assert_struct_equal(mla_string_t, url.path, mla_string("/page"), "Path should be '/page'");
    assert_struct_equal(mla_string_t, url.fragment, mla_string("section"), "Fragment should be 'section'");
}

void UrlWithQueryAndFragmentTest() {
    mla_string_t urlString = mla_string("https://example.com/path?key=value#anchor");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse URL with query and fragment");
    assert_struct_equal(mla_string_t, url.path, mla_string("/path"), "Path should be '/path'");
    assert_equal(mla_array_list_size(url.query), (mla_size_t)1, "Should have 1 query parameter");
    assert_struct_equal(mla_string_t, url.fragment, mla_string("anchor"), "Fragment should be 'anchor'");
}

void UrlWithoutPathTest() {
    mla_string_t urlString = mla_string("http://example.com");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse URL without path");
    assert_struct_equal(mla_string_t, url.host, mla_string("example.com"), "Host should be 'example.com'");
    assert_true(mla_string_is_empty(url.path), "Path should be empty");
}

void WebSocketUrlTest() {
    mla_string_t urlString = mla_string("ws://example.com:3000/socket");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse WebSocket URL");
    assert_struct_equal(mla_string_t, url.scheme, mla_string("ws"), "Scheme should be 'ws'");
    assert_equal(url.port, (mla_uint16_t)3000, "Port should be 3000");
}

void SecureWebSocketUrlTest() {
    mla_string_t urlString = mla_string("wss://example.com/socket");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse secure WebSocket URL");
    assert_struct_equal(mla_string_t, url.scheme, mla_string("wss"), "Scheme should be 'wss'");
    assert_equal(url.port, (mla_uint16_t)443, "Port should default to 443 for wss");
}

void InvalidUrlNoSchemeTest() {
    mla_string_t urlString = mla_string("example.com/path");
    mla_url_t url = mla_url_empty();

    assert_false(mla_url_parse(urlString, url), "Should fail to parse URL without scheme");
}

void EmptyUrlTest() {
    mla_string_t urlString = mla_string_empty();
    mla_url_t url = mla_url_empty();

    assert_false(mla_url_parse(urlString, url), "Should fail to parse empty URL");
}

void InvalidPortTest() {
    mla_string_t urlString = mla_string("http://example.com:99999/path");
    mla_url_t url = mla_url_empty();

    assert_false(mla_url_parse(urlString, url), "Should fail to parse URL with invalid port");
}

void QueryParamWithoutValueTest() {
    mla_string_t urlString = mla_string("http://example.com?flag&key=value");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(urlString, url), "Should parse query param without value");

    if (mla_array_list_size(url.query) == 2) {
        mla_url_query_param_t* param1 = mla_array_list_get_ref(url.query, 0);
        assert_struct_equal(mla_string_t, param1->key, mla_string("flag"), "First param key should be 'flag'");
        assert_true(mla_string_is_empty(param1->value), "First param value should be empty");
    } else {
        assert_true(false, "Should have 2 query parameters");
    }

}

void UrlToStringBasicTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("https");
    url.host = mla_string("example.com");
    url.port = 443;
    url.path = mla_string("/path");

    mla_string_t result = mla_url_to_string(url);
    assert_struct_equal(mla_string_t, result, mla_string("https://example.com:443/path"),
                "URL string should match expected format");
    mla_string_destroy(result);
}

void UrlToStringWithQueryTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("http");
    url.host = mla_string("example.com");
    url.port = 80;
    url.path = mla_string("/search");

    mla_url_query_param_t param1 = mla_url_query_param_empty();
    param1.key = mla_string("q");
    param1.value = mla_string("test");
    mla_array_list_add(url.query, param1);

    mla_url_query_param_t param2 = mla_url_query_param_empty();
    param2.key = mla_string("page");
    param2.value = mla_string("1");
    mla_array_list_add(url.query, param2);

    mla_string_t result = mla_url_to_string(url);
    assert_struct_equal(mla_string_t, result, mla_string("http://example.com:80/search?q=test&page=1"),
                "URL string with query should match expected format");
    mla_string_destroy(result);
}

void UrlToStringWithFragmentTest() {

    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("https");
    url.host = mla_string("example.com");
    url.port = 443;
    url.path = mla_string("/page");
    url.fragment = mla_string("section");

    mla_string_t result = mla_url_to_string(url);
    assert_struct_equal(mla_string_t, result, mla_string("https://example.com:443/page#section"),
                "URL string with fragment should match expected format");
    mla_string_destroy(result);
}

void RoundTripParseAndToStringTest() {
    mla_string_t original = mla_string("https://example.com:8443/path?key=value#anchor");
    mla_url_t url = mla_url_empty();

    assert_true(mla_url_parse(original, url), "Should parse original URL");
    mla_string_t reconstructed = mla_url_to_string(url);
    assert_struct_equal(mla_string_t, original, reconstructed,
                "Reconstructed URL should match original");
    mla_string_destroy(reconstructed);
}

void UrlToStringPathAndQueryBasicTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("https");
    url.host = mla_string("example.com");
    url.port = 443;
    url.path = mla_string("/path");

    mla_string_t result = mla_url_to_string_pathAndQuery(url);
    assert_struct_equal(mla_string_t, result, mla_string("/path"),
                "Path only should return '/path'");
    mla_string_destroy(result);
}

void UrlToStringPathAndQueryWithQueryTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("http");
    url.host = mla_string("example.com");
    url.port = 80;
    url.path = mla_string("/search");

    mla_url_query_param_t param1 = mla_url_query_param_empty();
    param1.key = mla_string("q");
    param1.value = mla_string("test");
    mla_array_list_add(url.query, param1);

    mla_url_query_param_t param2 = mla_url_query_param_empty();
    param2.key = mla_string("page");
    param2.value = mla_string("1");
    mla_array_list_add(url.query, param2);

    mla_string_t result = mla_url_to_string_pathAndQuery(url);
    assert_struct_equal(mla_string_t, result, mla_string("/search?q=test&page=1"),
                "Path with query should match expected format");
    mla_string_destroy(result);
}

void UrlToStringPathAndQueryNoPathTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("https");
    url.host = mla_string("example.com");
    url.port = 443;

    mla_string_t result = mla_url_to_string_pathAndQuery(url);
    assert_struct_equal(mla_string_t, result, mla_string("/"),
                "Empty path should default to '/'");
    mla_string_destroy(result);
}

void UrlToStringPathAndQueryIgnoreFragmentTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("https");
    url.host = mla_string("example.com");
    url.port = 443;
    url.path = mla_string("/page");
    url.fragment = mla_string("section");

    mla_string_t result = mla_url_to_string_pathAndQuery(url);
    assert_struct_equal(mla_string_t, result, mla_string("/page"),
                "Fragment should be ignored");
    mla_string_destroy(result);
}

void UrlToStringPathAndQueryWithQueryNoPathTest() {
    mla_url_t url = mla_url_empty();
    url.scheme = mla_string("http");
    url.host = mla_string("example.com");
    url.port = 80;

    mla_url_query_param_t param = mla_url_query_param_empty();
    param.key = mla_string("search");
    param.value = mla_string("value");
    mla_array_list_add(url.query, param);

    mla_string_t result = mla_url_to_string_pathAndQuery(url);
    assert_struct_equal(mla_string_t, result, mla_string("/?search=value"),
                "Query without path should include default '/'");
    mla_string_destroy(result);
}



void RegisterUrlTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("BasicUrlParse", test_category, BasicUrlParseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlWithExplicitPort", test_category, UrlWithExplicitPortTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlWithQueryParams", test_category, UrlWithQueryParamsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlWithFragment", test_category, UrlWithFragmentTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlWithQueryAndFragment", test_category, UrlWithQueryAndFragmentTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlWithoutPath", test_category, UrlWithoutPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WebSocketUrl", test_category, WebSocketUrlTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SecureWebSocketUrl", test_category, SecureWebSocketUrlTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InvalidUrlNoScheme", test_category, InvalidUrlNoSchemeTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EmptyUrl", test_category, EmptyUrlTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InvalidPort", test_category, InvalidPortTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("QueryParamWithoutValue", test_category, QueryParamWithoutValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringBasic", test_category, UrlToStringBasicTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringWithQuery", test_category, UrlToStringWithQueryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringWithFragment", test_category, UrlToStringWithFragmentTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("RoundTripParseAndToString", test_category, RoundTripParseAndToStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringPathAndQueryBasic", test_category, UrlToStringPathAndQueryBasicTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringPathAndQueryWithQuery", test_category, UrlToStringPathAndQueryWithQueryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringPathAndQueryNoPath", test_category, UrlToStringPathAndQueryNoPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringPathAndQueryIgnoreFragment", test_category, UrlToStringPathAndQueryIgnoreFragmentTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UrlToStringPathAndQueryWithQueryNoPath", test_category, UrlToStringPathAndQueryWithQueryNoPathTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
