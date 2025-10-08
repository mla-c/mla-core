//
// Created by christian on 10/6/25.
//

#include "mla_http_client.h"

#include "../log/mla_logging.h"
#include "../network/mla_network.h"
#include "../system/mla_string_concat.h"
#include "../url/mla_url.h"

mla_http_client_t mla_http_client() {
    return {};
}


mla_bool_t __mla_http_client_resolve_host(mla_http_client_response& response, const mla_url_t& url, mla_network_host_t & host) {

}

mla_bool_t __mla_http_client_connect(mla_http_client_response& response, const mla_network_host_t & host, mla_network_connection_t & connection) {

}

mla_bool_t __mla_http_client_send_header(mla_http_client_response& response, const mla_url_t& url, const mla_http_request_t & request, mla_stream_output_t & connection) {

    if (mla_string_is_empty(request.method)) {
        mla_error("Request has no method");
        return false;
    }

    if (!mla_stream_output_write_string(connection, request.method)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const(" "))) {
        return false;
    }

    mla_string_t pathAndQuery = mla_url_to_string_pathAndQuery(url);

    if (!mla_stream_output_write_string(connection, pathAndQuery)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const(" "))) {
        return false;
    }

    switch (request.version) {
        case MLA_HTTP_VERSION_1_0:
            if (!mla_stream_output_write_string(connection, mla_string_const("HTTP/1.0\r\n"))) {
                return false;
            }
            break;
        case MLA_HTTP_VERSION_1_1:
            if (!mla_stream_output_write_string(connection, mla_string_const("HTTP/1.1\r\n"))) {
                return false;
            }
            break;
        default:
            response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL;
            response.errorMessage = mla_string_concat("Unsupported HTTP version: ", request.version);
            return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("Host: "))) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, url.host)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
        return false;
    }

    // Write Headers
    for (mla_size_t i = 0; i < mla_array_list_size(request.headers); i++) {
        const mla_http_header_t* header = mla_array_list_get_ref(request.headers, i);
        if (!mla_stream_output_write_string(connection, header->name)) {
            return false;
        }
        if (!mla_stream_output_write_string(connection, mla_string_const(": "))) {
            return false;
        }
        if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
            if (!mla_stream_output_write_string(connection, header->value)) {
                return false;
            }
        } else {
            for (mla_size_t j = 0; j < mla_array_list_size(header->values); j++) {
                const mla_string_t* value = mla_array_list_get_ref(header->values, j);
                if (!mla_stream_output_write_string(connection, *value)) {
                    return false;
                }
                if (j < mla_array_list_size(header->values) - 1) {
                    if (!mla_stream_output_write_string(connection, mla_string_const(", "))) {
                        return false;
                    }
                }
            }
        }
        if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
            return false;
        }
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
        return false;
    }

    return true;

}

mla_bool_t __mla_http_client_send_body(mla_http_client_response& response, const mla_http_request_t & request, mla_stream_output_t & connection) {
    return mla_stream_copy(request.content, connection);
}

mla_bool_t __mla_http_client_parse_response_header(mla_stream_input_t & inputStream, mla_http_response_t & response) {

}

mla_bool_t __mla_http_client_handle_response_body(mla_http_client_response& response, const mla_http_request_t & request, mla_network_connection_t & connection) {

}

mla_bool_t __mla_http_client_parse_url(mla_http_client_response& response, const mla_string_t & urlString, mla_url_t & url) {

    if (!mla_url_parse(urlString, url)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL;
        response.errorMessage = mla_string_concat("Failed to parse URL: ", urlString);
        return false;
    }
    return true;
}

void __mla_http_client_close_connection(mla_network_connection_t & connection) {
    mla_network_connection_disconnect(connection);
}

mla_http_client_response mla_http_client_send_request(mla_http_client_t &p_Client, const mla_http_request_t &p_Request) {

    mla_http_client_response response = { MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, mla_string_empty(), mla_http_response_empty() };

    /////////////
    /// CONNECT
    /////////////

    // Resolve host
    mla_network_host_t host = mla_network_host_invalid();

    mla_url_t url = mla_url_empty();

    if (!__mla_http_client_parse_url(response, p_Request.url, url)) {
        return response;
    }

    if (!__mla_http_client_resolve_host(response, url, host)) {
        return response;
    }

    // Open Connection
    mla_network_connection_t connection = mla_network_connection_disconnected();

    if (!__mla_http_client_connect(response, host, connection)) {
        return response;
    }

    ////////////
    /// REQUEST
    ////////////

    // Send Header
    if (!__mla_http_client_send_header(response, url, p_Request, connection.outputStream)) {
        __mla_http_client_close_connection(connection);
        return response;
    }

    // Send Body
    if (!__mla_http_client_send_body(response, p_Request, connection.outputStream)) {
        __mla_http_client_close_connection(connection);
        return response;
    }

    ///////////
    /// RESPONSE
    ////////////

    // Parse Response Header
    if (!__mla_http_client_parse_response_header(connection.inputStream, response.response)) {
        __mla_http_client_close_connection(connection);
        return response;
    }

    // Handle Response Body
    if (!__mla_http_client_handle_response_body(response, p_Request, connection)) {
        __mla_http_client_close_connection(connection);
        return response;
    }

    return response;
}
