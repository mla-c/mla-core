//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_HTTP_RESPONSE_H
#define COREOS_MLA_HTTP_RESPONSE_H



struct mla_http_response_t {
    mla_int16_t statusCode; // e.g., 200, 404, etc.
    mla_array_list_t<mla_http_header_t, mla_http_header_initializer> headers;
    mla_stream_input_t content; // Response body content
};

mla_http_response_t mla_http_response_empty();


#endif
