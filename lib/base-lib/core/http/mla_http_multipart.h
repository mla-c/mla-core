#ifndef MLA_HTTP_MULTIPART_H
#define MLA_HTTP_MULTIPART_H

#include "mla_http_request.h"
#include "mla_http_response.h"
#include "mla_http_client.h"

// Client Helper

mla_http_client_response_t mla_http_client_upload_file(
    const mla_http_client_t &client, 
    const mla_string_t& url, 
    const mla_string_t& field_name, 
    const mla_string_t& file_name, 
    const mla_string_t& content_type, 
    mla_stream_input_t file_content);

inline mla_http_client_response_t mla_http_client_upload_file(
    const mla_string_t& url, 
    const mla_string_t& field_name, 
    const mla_string_t& file_name, 
    const mla_string_t& content_type, 
    mla_stream_input_t file_content) 
{
    mla_http_client_t client = mla_http_client();
    return mla_http_client_upload_file(client, url, field_name, file_name, content_type, file_content);
}

// Server Helper

struct mla_http_multipart_part_t {
    mla_string_t field_name;
    mla_string_t file_name;
    mla_string_t content_type;
    mla_stream_input_t content;
};

// Return true to continue, false to abort.
typedef mla_bool_t (*mla_http_multipart_part_handler_t)(const mla_user_data_t& userdata, const mla_http_multipart_part_t& part);

mla_bool_t mla_http_server_parse_multipart(const mla_http_request_t& request, mla_user_data_t userdata, mla_http_multipart_part_handler_t handler);

#endif
