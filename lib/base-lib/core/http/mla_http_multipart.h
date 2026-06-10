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

mla_http_multipart_part_t mla_http_multipart_part_empty();

struct mla_http_server_multipart_parse_context_t {
    mla_user_data_t userdata;
};

mla_http_server_multipart_parse_context_t mla_http_server_multipart_parse_context_empty();

// Creates a multipart parsing context for the given HTTP request. Returns true if the context was successfully created, false otherwise (e.g. if the request is not a multipart request).
mla_bool_t mla_http_server_parse_multipart_create_context(const mla_http_request_t& request, mla_http_server_multipart_parse_context_t& context);
// Parses the next multipart item from the request content. Returns true if an item was successfully parsed, false if there are no more items or if an error occurred. The parsed item is stored in the result parameter.
mla_bool_t mla_http_server_parse_multipart_next_item(mla_http_server_multipart_parse_context_t& context, mla_http_multipart_part_t& result);
// Finishes processing the current multipart item. This should be called after processing the content of a multipart item to ensure that the parser is ready for the next item. Returns true if the item was successfully finished, false if an error occurred.
mla_bool_t mla_http_server_parse_multipart_finish_item(mla_http_server_multipart_parse_context_t& context);

#endif
