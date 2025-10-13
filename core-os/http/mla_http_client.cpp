//
// Created by christian on 10/6/25.
//

#include "mla_http_client.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../system/mla_number.h"

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

mla_bool_t __mla_http_client_send_body(mla_http_client_response_t& response, const mla_http_request_t & request, const mla_stream_output_t & connection) {

    if (!mla_stream_copy(request.content, connection)) {
        response.status = MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN;
        response.errorMessage = mla_string_const("Failed to send request body");
        return false;
    }

    return true;
}

mla_bool_t __mla_http_client_read_line(const mla_stream_input_t & inputStream, mla_string_t & line) {

    mla_char_t buffer[mla_stream_fast_read_buffer_size];
    mla_size_t bytesRead = 0;

    mla_char_t* finalResultBuffer = nullptr;
    mla_size_t finalResultBufferSize = 0;

    while (true) {
        mla_size_t result = inputStream.read(inputStream, bytesRead, 1, reinterpret_cast<mla_byte_t*>(&buffer[0]));

        if (result == 0) {
            // End of stream
            break;
        }

        bytesRead += result;

        if (bytesRead >= 2 && buffer[bytesRead - 2] == '\r' && buffer[bytesRead - 1] == '\n') {
            // End of line found

            if (finalResultBuffer == nullptr) {
                // Fast path, line is in buffer
                line = mla_string_copy(buffer, bytesRead - 2); // Exclude \r\n
                return true;
            } else {
                // Combine final buffer and current buffer
                mla_char_t* newBuffer = mla_create_char_array(finalResultBufferSize + bytesRead - 2); // Exclude \r\n

                if (newBuffer == nullptr) {
                    // Memory allocation failed
                    mla_free(finalResultBuffer);
                    return false;
                }

                mla_memcpy(newBuffer, finalResultBuffer, finalResultBufferSize);
                mla_free(finalResultBuffer);
                mla_memcpy(newBuffer + finalResultBufferSize, buffer, bytesRead - 2);
                line = mla_string_from_buffer_with_ownership(finalResultBuffer, finalResultBufferSize + bytesRead - 2);
                return true;
            }

            line = mla_string_copy(buffer, bytesRead - 2); // Exclude \r\n
            return true;
        }

        if (bytesRead >= mla_stream_fast_read_buffer_size) {
            // Move to finalbuffer
            if (finalResultBuffer == nullptr) {
                finalResultBuffer = mla_create_char_array(bytesRead);

                if (finalResultBuffer == nullptr) {
                    // Memory allocation failed
                    return false;
                }

                mla_memcpy(finalResultBuffer, buffer, bytesRead);
            } else {
                mla_char_t* newBuffer = mla_create_char_array(finalResultBufferSize + bytesRead);

                if (newBuffer == nullptr) {
                    // Memory allocation failed
                    mla_free(finalResultBuffer);
                    return false;
                }

                mla_memcpy(newBuffer, finalResultBuffer, finalResultBufferSize);
                mla_memcpy(newBuffer + finalResultBufferSize, buffer, bytesRead);
                mla_free(finalResultBuffer);
                finalResultBuffer = newBuffer;
            }
        }

        if ((finalResultBufferSize + bytesRead) >= mla_http_max_header_size) {

            if (finalResultBuffer != nullptr) {
                mla_free(finalResultBuffer);
            }

            // Line too long
            return false;
        }
    }

    if (finalResultBuffer != nullptr) {
        mla_free(finalResultBuffer);
    }

    return false;
}

mla_bool_t __mla_http_client_parse_response_header(const mla_stream_input_t & inputStream, mla_http_response_t & response) {

    mla_string_t statusLine = mla_string_empty();

    if (!__mla_http_client_read_line(inputStream, statusLine)) {
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

    if (mla_string_equals_const(versionStr, "HTTP/1.0")) {
        response.version = MLA_HTTP_VERSION_1_0;
    } else if (mla_string_equals_const(versionStr, "HTTP/1.1")) {
        response.version = MLA_HTTP_VERSION_1_1;
    } else {
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

    mla_string_t doublePoint = mla_string_const(":");

    // Parse the response headers
    while (true) {
        mla_string_t headerLine = mla_string_empty();
        if (!__mla_http_client_read_line(inputStream, headerLine)) {
            response.statusCode = 400;
            response.statusMessage = mla_string_const("Failed to read header line");
            return false;
        }

        if (mla_string_is_empty(headerLine)) {
            // End of headers
            break;
        }

        mla_int32_t colonPos = mla_string_index_of(headerLine, doublePoint);
        if (colonPos < 0) {
            response.statusCode = 400;
            response.statusMessage = mla_string_const("Invalid header line");
            return false;
        }

        mla_string_t headerName = mla_string_trim(mla_string_substr(headerLine, 0, colonPos -1));
        mla_string_t headerValue = mla_string_trim(mla_string_substr(headerLine, colonPos + 1, headerLine.length - 1));

        // Add header to response
        mla_http_headers_add(response.headers, headerName, headerValue);
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
