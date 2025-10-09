//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_HTTP_RESPONSE_H
#define COREOS_MLA_HTTP_RESPONSE_H

#include "../mla_data_types.h"
#include "mla_http_header.h"
#include "../system/mla_stream.h"
#include "../system/mla_string.h"
#include "mla_http_data_types.h"

struct mla_http_response_t {
    mla_http_version version; // e.g., HTTP/1.1
    mla_uint16_t statusCode; // e.g., 200, 404, etc.
    mla_string_t statusMessage; // e.g., "OK", "Not Found", etc.
    mla_array_list_t<mla_http_header_t, mla_http_header_initializer> headers;
    mla_stream_input_t content; // Response body content
};

inline mla_http_response_t mla_http_response_empty() {
    return {MLA_HTTP_VERSION_1_0, 0, mla_string_empty(), mla_array_list_empty<mla_http_header_t, mla_http_header_initializer>(), mla_stream_noop_input()};
}


#endif
