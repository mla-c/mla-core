//
// Created by chris on 10/23/2025.
//

#ifndef COREOS_MLA_HTTP_UTILS_H
#define COREOS_MLA_HTTP_UTILS_H

#include "../system/mla_stream.h"
#include "mla_http_header.h"
#include "mla_http_data_types.h"

mla_bool_t mla_http_utils_read_line(const mla_stream_input_t & inputStream, mla_string_t & line);
mla_bool_t mla_http_utils_write_headers(const mla_array_list_t<mla_http_header_t, mla_http_header_initializer> &headers, const mla_stream_output_t & connection);
mla_bool_t mla_http_utils_read_headers(mla_array_list_t<mla_http_header_t, mla_http_header_initializer> &headers, const mla_stream_input_t & connection);
mla_bool_t mla_http_utils_parse_http_version(const mla_string_t &versionStr, mla_http_version &version);

#endif