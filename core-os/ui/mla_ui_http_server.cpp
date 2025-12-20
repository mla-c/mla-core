//
// Created by christian on 12/5/25.
//

#include "mla_ui_http_server.h"
#include "mla_ui_web_embedded.h"

mla_bool_t __mla_ui_http_server_handler(const mla_http_request_t &request, mla_http_response_t &response) {

    // Remove "/ui/" prefix
    mla_string_t resource_name = mla_string_substr(request.url, 4, request.url.length - 4);

    if (mla_string_is_empty(resource_name)) {
        resource_name = mla_string_const("index.html");
    }

    if (mla_string_equals_const(resource_name, "index.html")) {

        // Serve index.html
        response.statusCode = mla_http_status_ok;
        response.statusMessage = mla_string_const("OK");
        // Add headers
        mla_http_headers_add(response.headers, mla_string_const("Content-Type"), mla_string_const("text/html; charset=UTF-8"));
        mla_http_headers_add(response.headers, mla_string_const("Content-Length"), mla_string_from_uint32(mla_ui_web_embedded_index_html_len));

        mla_string_t content_encoding = mla_string(mla_ui_web_embedded_index_html_content_encoding);

        if (!mla_string_is_empty(content_encoding)) {
            mla_http_headers_add(response.headers, mla_string_const("Content-Encoding"), content_encoding);
        }

        // Set content
        response.content = mla_stream_input_from_buffer(mla_ui_web_embedded_index_html, mla_ui_web_embedded_index_html_len);
        return true;

    } else {
        // Not found
        response.statusCode = mla_http_status_not_found;
        response.statusMessage = mla_string_const("Not Found");
        return true;
    }
}

mla_bool_t mla_ui_http_server_initialize(mla_http_server_t &server) {

    mla_http_server_handler_item_t handler = mla_http_server_handler_starts_with(mla_http_method_get, mla_string_const("/ui/"), __mla_ui_http_server_handler);
    return mla_http_server_register_handler(server, handler);

}