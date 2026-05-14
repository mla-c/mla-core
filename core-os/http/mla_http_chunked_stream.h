//
// Created by chris on 11/27/2025.
//

#ifndef MLA_C_HTTP_CHUNKED_STREAM_H
#define MLA_C_HTTP_CHUNKED_STREAM_H

#include "../system/mla_stream.h"

mla_stream_input_t mla_http_chunked_stream_input(const mla_stream_input_t &baseStream, mla_int32_t timeout_ms);
mla_stream_input_t mla_http_chunked_stream_input_deflate(const mla_stream_input_t &baseStream, mla_int32_t timeout_ms);

struct mla_http_chunked_stream_output_t {
    mla_stream_output_t output;
    mla_stream_output_t chunkedStream;
    mla_stream_output_t baseStream;
};

mla_bool_t mla_http_chunked_stream_output_finished(mla_http_chunked_stream_output_t &chunkedOutput);
mla_http_chunked_stream_output_t mla_http_chunked_stream_output_invalid();
mla_http_chunked_stream_output_t mla_http_chunked_stream_output(const mla_stream_output_t &baseStream);
mla_http_chunked_stream_output_t mla_http_chunked_stream_output_deflate(const mla_stream_output_t &baseStream);

#endif