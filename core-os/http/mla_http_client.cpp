//
// Created by christian on 10/6/25.
//

#include "mla_http_client.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../system/mla_number.h"
#include "mla_http_utils.h"

mla_bool_t __mla_http_client_default_resolve_host(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host) {

    (void)client;

    if (!mla_network_host_resolve(host, url.host, url.port)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST;
        response.errorMessage = mla_string_concat("Failed to resolve host: ", url.host);
        return false;
    }

    return true;
}

mla_bool_t __mla_http_client_default_connect(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_network_host_t & host, mla_network_connection_t & connection) {

    if (!mla_network_connection_connect(connection, host, mla_connection_type_tcp, client.timeout_ms)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED;
        response.errorMessage = mla_string_concat("Failed to connect to host: ", host.address.address);
        return false;
    }

    return true;
}

mla_http_client_t mla_http_client() {
    return {
        30000, // Default timeout 30 seconds
        __mla_http_client_default_resolve_host,
        __mla_http_client_default_connect
    };
}


mla_bool_t __mla_http_client_resolve_host(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_url_t& url, mla_network_host_t & host) {

    if (client.resolve_host == nullptr) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("HTTP client has no resolve_host function");
        return false;
    }

    return client.resolve_host(client, response, url, host);
}

mla_bool_t __mla_http_client_connect(const mla_http_client_t &client, mla_http_client_response_t& response, const mla_network_host_t & host, mla_network_connection_t & connection) {

    if (client.connect == nullptr) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("HTTP client has no connect function");
        return false;
    }

    return client.connect(client, response, host, connection);
}

mla_bool_t __mla_http_client_send_header(mla_http_client_response_t& response, const mla_url_t& url, const mla_http_request_t & request, const mla_stream_output_t & connection) {

    if (mla_string_is_empty(request.method)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("HTTP request has no method");
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
            response.errorMessage = mla_string_concat("Unsupported HTTP version: ", mla_string_from_int32(request.version));
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
    if (!mla_http_utils_write_headers(request.headers, connection)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection, mla_string_const("\r\n"))) {
        return false;
    }

    return true;

}

mla_bool_t __mla_http_client_send_body(mla_http_client_response_t& response, const mla_http_request_t & request, const mla_stream_output_t & connection) {

    if (!mla_stream_copy(request.content, connection)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("Failed to send request body");
        return false;
    }

    return true;
}

mla_bool_t __mla_http_client_parse_response_header(const mla_stream_input_t & inputStream, mla_http_response_t & response) {

    mla_string_t statusLine = mla_string_empty();

    if (!mla_http_utils_read_line(inputStream, statusLine)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Failed to read status line");
        return false;
    }

    // Parse status line
    mla_array_list_t<mla_string_t, mla_string_initializer> parts = mla_string_split(statusLine, mla_string_const(" "));

    if (mla_array_list_size(parts) < 2) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Invalid status line");
        return false;
    }
    // HTTP version is parts[0]
    mla_string_t versionStr = *mla_array_list_get_ref(parts, 0);

    if (!mla_http_utils_parse_http_version(versionStr, response.version)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_concat("Unsupported HTTP version: ", versionStr);
        return false;
    }

    // HTTP status code is parts[1]
    mla_string_t statusCodeStr = *mla_array_list_get_ref(parts, 1);

    if (!mla_parse_uint16(statusCodeStr, response.statusCode)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Invalid status code");
        return false;
    }

    if (!mla_http_utils_read_headers(response.headers, inputStream)) {
        response.statusCode = 400;
        response.statusMessage = mla_string_const("Failed to read header line");
        return false;
    }

    return true;

}

mla_bool_t __mla_http_client_handle_response_body(mla_http_response_t& response, const mla_network_connection_t & connection) {

    // Check for Content-Length header
    mla_string_t contentLengthStr = mla_http_headers_get_value(response.headers, mla_string_const("Content-Length"));

    if (mla_string_equals_const(contentLengthStr, "0")) {
        response.content = mla_stream_noop_input();
        return false;
    }

    if (connection.inputStream.remaining_bytes != nullptr) {

        mla_size_t remaining = connection.inputStream.remaining_bytes(connection.inputStream);
        if (remaining == 0) {
            return false;
        }
    }

    response.content = connection.inputStream;
    return true;
}

mla_bool_t __mla_http_client_parse_url(mla_http_client_response_t& response, const mla_string_t & urlString, mla_url_t & url) {

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

mla_http_client_response_t mla_http_client_send_request(const mla_http_client_t &client, const mla_http_request_t &p_Request) {

    mla_http_client_response_t response = { MLA_HTTP_CLIENT_RESPONSE_STATUS_OK, mla_string_empty(), mla_http_response_empty() };

    /////////////
    /// CONNECT
    /////////////

    // Resolve host
    mla_network_host_t host = mla_network_host_invalid();

    mla_url_t url = mla_url_empty();

    if (!__mla_http_client_parse_url(response, p_Request.url, url)) {
        return response;
    }

    if (!__mla_http_client_resolve_host(client, response, url, host)) {
        return response;
    }

    // Open Connection
    mla_network_connection_t connection = mla_network_connection_disconnected();

    if (!__mla_http_client_connect(client, response, host, connection)) {
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
    if (!__mla_http_client_handle_response_body(response.response, connection)) {
        __mla_http_client_close_connection(connection);
        return response;
    }

    return response;
}
