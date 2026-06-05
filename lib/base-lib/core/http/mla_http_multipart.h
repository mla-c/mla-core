//
// Created by Copilot.
//

#ifndef MLA_HTTP_MULTIPART_H
#define MLA_HTTP_MULTIPART_H

#include "mla_http_request.h"
#include "mla_http_response.h"
#include "mla_http_client.h"

////////////////////////////////////////////////////////////////
/// Client Multipart Helper
////////////////////////////////////////////////////////////////

// Helper method for the http client to upload a file using multipart/form-data.
// This takes a base URL, field name, file name, content type, and a stream for the binary content.
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

////////////////////////////////////////////////////////////////
/// Server Multipart Helper
////////////////////////////////////////////////////////////////

struct mla_http_multipart_part_t {
    mla_string_t field_name;
    mla_string_t file_name;
    mla_string_t content_type;
    mla_stream_input_t content;
};

// Callback to handle each part. Must return true to continue parsing, false to abort.
// The `part.content` stream must be fully consumed by the handler if it wants to proceed to the next part,
// OR the parser will automatically consume the remaining bytes of the part.
typedef mla_bool_t (*mla_http_multipart_part_handler_t)(mla_user_data_t userdata, mla_http_multipart_part_t& part);

// Parses a multipart/form-data request body and invokes the handler for each part.
// Returns true on successful parsing of all parts, false on error.
mla_bool_t mla_http_server_parse_multipart(const mla_http_request_t& request, mla_user_data_t userdata, mla_http_multipart_part_handler_t handler);

#endif
