//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_HTTP_REQUEST_H
#define COREOS_MLA_HTTP_REQUEST_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "mla_http_header.h"
#include "../system/mla_stream.h"
#include "mla_http_data_types.h"



struct mla_http_request_t {
    mla_http_version version;
    mla_string_t url;
    mla_string_t method; // GET, POST, PUT, DELETE, etc.
    mla_array_list_t<mla_http_header_t, mla_http_header_initializer> headers;
    mla_stream_input_t  content; // Request body content
};

inline mla_http_request_t mla_http_request_empty() {
    return {
        MLA_HTTP_VERSION_1_0,
            mla_string_empty(),
            mla_string_empty(),
            mla_array_list_empty<mla_http_header_t, mla_http_header_initializer>(),
            mla_stream_noop_input()
    };
}

inline mla_http_request_t mla_http_request(const mla_string_t &p_Url, const mla_string_t &p_Method) {
    return {
        MLA_HTTP_VERSION_1_1,
            p_Url,
            p_Method,
            mla_array_list_empty<mla_http_header_t, mla_http_header_initializer>(),
            mla_stream_noop_input()
    };
}

////////////////////////////////////////////////////////////////
/// Helpers
////////////////////////////////////////////////////////////////

#define mla_http_method_get mla_string_const("GET")
#define mla_http_method_post mla_string_const("POST")
#define mla_http_method_put mla_string_const("PUT")
#define mla_http_method_delete mla_string_const("DELETE")

inline mla_http_request_t mla_http_get_request(const mla_string_t &p_Url) {
    return mla_http_request(p_Url, mla_http_method_get);
}

inline mla_http_request_t mla_http_post_request(const mla_string_t &p_Url) {
    return mla_http_request(p_Url, mla_http_method_post);
}

#endif
