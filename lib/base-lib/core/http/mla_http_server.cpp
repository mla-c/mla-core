//
// Created by chris on 10/23/2025.
//

#include "mla_http_server.h"

#include "../log/mla_logging.h"
#include "../task/mla_task_manager.h"
#include "mla_http_utils.h"
#include "../system/mla_id.h"
#include "../system/mla_number.h"
#include "mla_websocket_utils.h"
#include "../hash/mla_sha1.h"
#include "../http/mla_http_chunked_stream.h"

#define mla_handler_item_array_param mla_http_server_handler_item_t, mla_http_server_handler_item_initializer
#define mla_websocket_handler_item_array_param mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer
#define mla_websocket_connection_array_param mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer

mla_user_data_id_init(mla_http_task_user_data_server_name)

#define mla_http_server_connection_input_buffer_size mla_global_config_stream_fast_read_buffer_size
#define mla_http_server_connection_output_buffer_size mla_global_config_stream_fast_read_buffer_size

mla_http_server_handler_item_t mla_http_server_handler(const mla_string_t &http_method,
                                                       mla_user_data_t& userdata,
                                                       const mla_http_request_handler_checker_t &checker,
                                                       const mla_http_request_handler_t &executor) {
    return {
        userdata,
        mla_string_to_upper(http_method),
        checker,
        executor
    };
}

struct mla_http_server_path_checker_userdata_t {
    mla_string_t path_data;

    static mla_http_server_path_checker_userdata_t init() {
        return {
            mla_string_empty(),
        };
    }
};

mla_user_data_id_init(mla_http_server_handler_path_data_user_data_name)

mla_bool_t mla_private_http_server_handler_starts_with_checker(const mla_user_data_t& userdata,
                                                         const mla_string_t &url, mla_http_request_handler_checker_compare_mode_t compare_mode) {

    mla_pointer_t checker_userdata = mla_user_data_get_pointer(userdata, mla_http_server_handler_path_data_user_data_name);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return false;
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH) {
        return mla_string_starts_with(url, checker_userdata_ptr->path_data);
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX) {
        return mla_string_starts_with(checker_userdata_ptr->path_data, url);
    }

    return false;

}

mla_bool_t mla_private_http_server_handler_equals_checker(const mla_user_data_t &userdata,
                                                    const mla_string_t &url, mla_http_request_handler_checker_compare_mode_t compare_mode) {

    mla_pointer_t checker_userdata = mla_user_data_get_pointer(userdata, mla_http_server_handler_path_data_user_data_name);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return false;
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH) {
        return mla_string_equals(url, checker_userdata_ptr->path_data);
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX) {
        return mla_string_starts_with(url, checker_userdata_ptr->path_data);
    }

    return false;
}

mla_http_server_handler_item_t mla_http_server_handler_starts_with(const mla_string_t &http_method,
                                                                   const mla_string_t& pathPrefix,
                                                                   const mla_http_request_handler_t &executor) {

    mla_pointer_t checker_userdata = mla_malloc_struct(mla_http_server_path_checker_userdata_t);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return mla_http_server_handler_invalid();
    }

    checker_userdata_ptr->path_data = pathPrefix;

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_pointer(userData, mla_http_server_handler_path_data_user_data_name, checker_userdata);

    return {
        userData,
        mla_string_to_upper(http_method),
        mla_private_http_server_handler_starts_with_checker,
        executor
    };
}

mla_bool_t mla_private_http_server_handler_all_checker(const mla_user_data_t& userdata, const mla_string_t &url, mla_http_request_handler_checker_compare_mode_t compare_mode) {
    (void) userdata;
    (void) url;
    (void) compare_mode;
    return true;
}

mla_http_server_handler_item_t mla_http_server_handler_all(const mla_string_t &http_method,
                                                           const mla_http_request_handler_t &executor) {
    return {
        mla_user_data_empty(),
        mla_string_to_upper(http_method),
        mla_private_http_server_handler_all_checker,
        executor
    };
}

mla_http_server_handler_item_t mla_http_server_handler_invalid() {
    return {
        mla_user_data_empty(),
        mla_string_empty(),
        nullptr,
        nullptr
    };
}

mla_http_server_websocket_connection_t mla_http_server_websocket_connection_invalid() {
    return {
        nullptr,
        mla_network_connection_disconnected(),
        mla_string_empty(),
        mla_string_empty(),
        mla_mutex_invalid(),
        mla_user_data_empty(),
        false,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
}

mla_http_server_websocket_connection_t mla_private_http_server_websocket_connection(mla_http_server_t& http_server, mla_network_connection_t& connection, const mla_http_server_websocket_handler_item_t& handlerItem, const mla_string_t& endpoint, mla_bool_t supports_deflate_compression) {

    const mla_string_t id = mla_generate_runtime_id();

#if mla_global_feature_flag_http_server_use_deflate_compression == 1
    mla_bool_t final_supports_deflate_compression = supports_deflate_compression;
#else
    (void)supports_deflate_compression;
    mla_bool_t final_supports_deflate_compression = false;
#endif


    return {
        &http_server,
        connection,
        id,
        endpoint,
        mla_mutex(mla_string_concat("WebSocketConn_", id), true),
        handlerItem.userdata,
        final_supports_deflate_compression,
        handlerItem.open_executor,
        handlerItem.text_executor,
        handlerItem.binary_executor,
        handlerItem.close_executor,
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler(
    mla_user_data_t &userdata, const mla_http_request_handler_checker_t &checker,
    const mla_http_websocket_text_message_handler_t &text_message_handler,
    const mla_http_websocket_binary_message_handler_t &binary_message_handler) {
    return {
        userdata,
        checker,
        nullptr,
        text_message_handler,
        binary_message_handler,
        nullptr
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_path_equals(
    const mla_string_t& path, const mla_http_websocket_text_message_handler_t &text_message_handler,
    const mla_http_websocket_binary_message_handler_t &binary_message_handler) {

    mla_pointer_t checker_userdata = mla_malloc_struct(mla_http_server_path_checker_userdata_t);

    mla_http_server_path_checker_userdata_t *checker_userdata_ptr = mla_pointer_get_data<mla_http_server_path_checker_userdata_t>(checker_userdata);

    if (checker_userdata_ptr == nullptr) {
        return mla_http_server_websocket_handler_invalid();
    }

    checker_userdata_ptr->path_data = path;

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_pointer(userData, mla_http_server_handler_path_data_user_data_name, checker_userdata);


    return {
        userData,
        mla_private_http_server_handler_equals_checker,
        nullptr,
        text_message_handler,
        binary_message_handler,
        nullptr
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_invalid() {
    return {
        mla_user_data_empty(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
}


struct mla_http_server_internal_resource_cleanup_userdata {
    mla_array_list_t<mla_string_t, mla_string_initializer> active_tasks;

    static mla_http_server_internal_resource_cleanup_userdata init() {
        return {
            mla_array_list_empty<mla_string_t, mla_string_initializer>()
        };
    }
};

mla_http_server_t mla_http_server_invalid() {
    return {
        mla_array_list_empty<mla_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_connection_array_param>(),
        mla_rw_lock_invalid(),
        mla_network_host_invalid(),
        mla_network_listener_invalid(),
        mla_mutex_invalid(),
        mla_pointer_null(),
        MLA_HTTP_SERVER_STATUS_STOPPED,
        0
    };
}

mla_http_server_t mla_http_server(const mla_network_host_t &host) {
    return {
        mla_array_list_empty<mla_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_handler_item_array_param>(),
        mla_array_list_empty<mla_websocket_connection_array_param>(),
        mla_rw_lock(mla_string_concat("HttpServerWebsocketConnections_", host.address.address, ":",
                                    mla_string_from_uint16(host.port)), true),
        host,
        mla_network_listener_invalid(),
        mla_mutex(mla_string_concat("HttpServerListenerLock_", host.address.address, ":",
                                    mla_string_from_uint16(host.port)), true),
        mla_pointer_null(),
        MLA_HTTP_SERVER_STATUS_STOPPED,
        mla_global_config_default_http_timeout_ms
    };
}

mla_bool_t mla_http_server_register_handler(mla_http_server_t &server,
                                            const mla_http_server_handler_item_t &handlerItem) {
    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (server.status == MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        mla_error("Cannot register handler while server is running");
        return false;
    }

    mla_bool_t result = mla_array_list_add(server.httpHandlers, handlerItem);

    mla_mutex_unlock(server.listenerLock);
    return result;
}

mla_bool_t mla_http_server_register_websocket_handler(mla_http_server_t &server,
                                                      const mla_http_server_websocket_handler_item_t &handlerItem) {
    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (server.status == MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        mla_error("Cannot register websocket handler while server is running");
        return false;
    }

    mla_bool_t result = mla_array_list_add(server.websocketHandlers, handlerItem);

    mla_mutex_unlock(server.listenerLock);
    return result;
}

mla_bool_t mla_private_http_server_request_read(mla_network_connection_t &connection, mla_http_request_t &request,
                                          mla_int32_t timeout_ms) {

    // Read request line
    mla_string_t requestLine = mla_string_empty();
    if (!mla_http_utils_read_line(connection.inputStream, requestLine, timeout_ms)) {
        return false;
    }

    // Parse request line
    mla_array_list_t<mla_string_t, mla_string_initializer> parts = mla_string_split(requestLine, mla_string_const(" "));

    if (mla_array_list_size(parts) < 3) {
        return false;
    }

    // Method
    request.method = mla_string_to_upper(*mla_array_list_get_ref(parts, 0));
    // URL
    request.url = *mla_array_list_get_ref(parts, 1);
    // HTTP Version
    mla_string_t versionStr = *mla_array_list_get_ref(parts, 2);

    if (!mla_http_utils_parse_http_version(versionStr, request.version)) {
        return false; // Unsupported HTTP version
    }

    // Read headers
    if (!mla_http_utils_read_headers(request.headers, connection.inputStream, timeout_ms)) {
        return false;
    }

    // Body (if any)
    // Check for Content-Length header
    mla_string_t contentLengthStr = mla_http_headers_get_value(request.headers, mla_string_const("Content-Length"));

    if (mla_string_equals_const(contentLengthStr, "0")) {
        request.content = mla_stream_noop_input();
        return true;
    }

    mla_size_t content_size = 0;

    if (mla_parse_uint32(contentLengthStr, content_size)) {
        request.content = mla_http_content_fixed_size_input_stream(connection.inputStream, timeout_ms, content_size);
        return true;
    }

    // No Content-Length, use chunked or until close
    mla_string_t transferEncoding = mla_http_headers_get_value(request.headers, mla_string_const("Transfer-Encoding"));

    if (mla_string_equals_const(transferEncoding, "chunked")) {

        // Check if we need switch to deflate
        if (mla_http_headers_has_header_value(request.headers, mla_string_const("Content-Encoding"), mla_string_const("deflate"))) {
            request.content = mla_http_chunked_stream_input_deflate(connection.inputStream, timeout_ms);
        } else {
            request.content = mla_http_chunked_stream_input(connection.inputStream, timeout_ms);
        }

        return true;
    }

    request.content = mla_stream_noop_input();
    return true;
}

mla_bool_t mla_private_http_server_response_send(mla_network_connection_t &connection,
                                           mla_http_response_t &response) {


    // Send status line
    mla_string_t httpVersion = (response.version == MLA_HTTP_VERSION_1_1)
                                   ? mla_string_const("HTTP/1.1 ")
                                   : mla_string_const("HTTP/1.0 ");

    if (!mla_stream_output_write_string(connection.outputStream, httpVersion)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_from_uint16(response.statusCode))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const(" "))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, response.statusMessage)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    // Send headers
    if (!mla_http_utils_write_headers(response.headers, connection.outputStream)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    // Write Content Length if body is present
    if (!mla_http_utils_write_content_headers(response.headers, response.content, connection.outputStream)) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        mla_stream_output_flush_buffered_wrapper(connection.outputStream);
        return false;
    }

    // Send body
    if (mla_http_response_content_writer_is_valid(response.contentWriter)) {

        if (!response.contentWriter.writeTo(response.contentWriter, connection.outputStream)) {
            mla_stream_output_flush_buffered_wrapper(connection.outputStream);
            return false;
        }

    } else {
        // No content writer, send content stream
        if (!mla_stream_copy(response.content, connection.outputStream)) {
            mla_stream_output_flush_buffered_wrapper(connection.outputStream);
            return false;
        }
    }

    if (!mla_stream_output_flush_buffered_wrapper(connection.outputStream)) {
        return false;
    }

    return true;
}

mla_string_t mla_private_http_server_compute_websocket_accept(const mla_string_t& secWebSocketKey) {
    const mla_string_t guid = mla_string_const("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    mla_string_t input = mla_string_concat(secWebSocketKey, guid);

    mla_bytes_t inputBytes = mla_bytes_from_string(input);
    mla_bytes_t digest = mla_sha1(inputBytes);
    mla_string_t accept = mla_bytes_to_base64(digest);
    return accept;
}


mla_task_process_result_state mla_private_http_server_handler_new_request(mla_user_data_t& userData) {

    mla_pointer_t http_server = mla_user_data_get_pointer(userData, mla_http_task_user_data_server_name);

    mla_http_server_t* http_server_ptr = mla_pointer_get_data<mla_http_server_t>(http_server);

    if (http_server_ptr == nullptr) {
        return TASK_PROCESS_RESULT_DONE; // Server reference lost, exit task
    }

    mla_http_server_t& server = *http_server_ptr;

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    // Accept a new connection

    if (!mla_mutex_trylock(server.listenerLock, 100)) {
        return TASK_PROCESS_RESULT_CONTINUE; // No connection, yield and try again
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    mla_network_connection_t clientConnection = mla_network_connection_disconnected();

    if (!mla_network_listener_accept_connection(server.listener, clientConnection)) {
        mla_mutex_unlock(server.listenerLock);
        return TASK_PROCESS_RESULT_CONTINUE; // No connection, yield and try again
    }

    mla_mutex_unlock(server.listenerLock);

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    // apply buffers to the connection
    clientConnection.inputStream = mla_stream_input_buffered_wrapper(clientConnection.inputStream, mla_http_server_connection_input_buffer_size);
    clientConnection.outputStream = mla_stream_output_buffered_wrapper(clientConnection.outputStream, mla_http_server_connection_output_buffer_size);

    mla_http_request_t request = mla_http_request_empty();

    if (!mla_private_http_server_request_read(clientConnection, request, server.timeout_ms)) {
        mla_warning(
            mla_string_concat("Invalid http request from client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port)));
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    mla_array_list_t<mla_handler_item_array_param> copyHandlers = server.
            httpHandlers;

    mla_http_response_t response = mla_http_response_empty();
    response.version = request.version;
    response.statusCode = 404; // Default to Not Found
    response.statusMessage = mla_string_const("Not Found");

    mla_bool_t processed = false;

    // Find a handler for the request
    for (mla_size_t i = 0; i < mla_array_list_size(copyHandlers); i++) {
        mla_http_server_handler_item_t &handlerItem = mla_array_list_get_unsafe(copyHandlers, i);

        if (handlerItem.checker == nullptr) {
            continue; // No checker function, skip
        }

        if (!mla_string_equals(request.method, handlerItem.method)) {
            continue;
        }

        if (!handlerItem.checker(handlerItem.userdata, request.url, MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH)) {
            continue; // Checker returned false, skip
        }

        if (handlerItem.executor != nullptr) {
            if (!handlerItem.executor(server, request, response)) {
                mla_warning(mla_string_concat("Handler executor failed for URL: ", request.url));
                response.statusCode = 500; // Internal Server Error
                response.statusMessage = mla_string_const("Internal Server Error");
            }
        }
        processed = true;

        break;
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    if (!processed) {

        // Check for websocket handlers
        mla_array_list_t<mla_websocket_handler_item_array_param> copyWebsocketHandlers = server.
            websocketHandlers;

        // Find a handler for the request
        for (mla_size_t i = 0; i < mla_array_list_size(copyWebsocketHandlers); i++) {

            mla_http_server_websocket_handler_item_t &handlerItem = mla_array_list_get_unsafe(copyWebsocketHandlers, i);

            if (handlerItem.checker == nullptr) {
                continue; // No checker function, skip
            }

            if (!handlerItem.checker(handlerItem.userdata, request.url, MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH)) {
                continue; // Checker returned false, skip
            }

            // ... inside the websocket handler match block ...
            // 1) Validate and compute accept
            mla_string_t wsKey = mla_http_headers_get_value(request.headers, mla_string_const("Sec-WebSocket-Key"));
            if (mla_string_is_empty(wsKey)) {
                // Reject bad handshake
                response.statusCode = 400;
                response.statusMessage = mla_string_const("Bad Request");
                break;
            }

            mla_string_t accept = mla_private_http_server_compute_websocket_accept(wsKey);

            mla_bool_t supports_deflate_compression = mla_http_headers_has_header_value(request.headers, mla_string_const("Sec-WebSocket-Extensions"), mla_string_const("permessage-deflate"), mla_string_const(","));

            // 2) Build 101 response (no body)
            response.statusCode = mla_http_status_switching_protocols; // 101
            response.statusMessage = mla_string_const("Switching Protocols");
            mla_http_headers_add(response.headers, mla_string_const("Connection"), mla_string_const("Upgrade"));
            mla_http_headers_add(response.headers, mla_string_const("Upgrade"), mla_string_const("websocket"));
            mla_http_headers_add(response.headers, mla_string_const("Sec-WebSocket-Accept"), accept);

#if mla_global_feature_flag_http_server_use_deflate_compression == 1

            if (supports_deflate_compression) {
                // Advertise client_no_context_takeover and server_no_context_takeover because
                // the server creates a fresh DEFLATE compressor/decompressor for every message.
                // Without these parameters third-party clients may reuse their compression
                // window across messages, causing the server's per-message decompressor to
                // produce corrupt output.
                mla_http_headers_add(response.headers, mla_string_const("Sec-WebSocket-Extensions"), mla_string_const("permessage-deflate; client_no_context_takeover; server_no_context_takeover"));
            }
#endif
            response.content = mla_stream_noop_input(); // ensure no body

            // 3) Register the WebSocket connection and keep it open
            mla_http_server_websocket_connection_t ws_connection = mla_private_http_server_websocket_connection(server, clientConnection, handlerItem, request.url, supports_deflate_compression);


            if (ws_connection.open_executor != nullptr) {
                if (!ws_connection.open_executor(ws_connection)) {
                    ws_connection = mla_http_server_websocket_connection_invalid();
                    break;
                }
            }

            if (mla_rw_lock_write(server.websocketConnectionsLock)) {
                if (mla_array_list_add(server.websocketConnections, ws_connection)) {
                    mla_info(mla_string_concat("Registered new WebSocket connection with id ", ws_connection.id, " for endpoint ", request.url));
                } else {
                    ws_connection = mla_http_server_websocket_connection_invalid(); // Mark as invalid to avoid processing
                }
                mla_rw_unlock_write(server.websocketConnectionsLock);
            } else {
                ws_connection = mla_http_server_websocket_connection_invalid(); // Mark as invalid to avoid processing
            }

            break;

        }
    }

    // Regular Http response
    if (!mla_private_http_server_response_send(clientConnection, response)) {

        mla_warning(
            mla_string_concat("Failed to send HTTP response to client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port), " for URL: ", request.url));
    }

    clientConnection = mla_network_connection_disconnected();
    return TASK_PROCESS_RESULT_CONTINUE;

}

mla_bool_t mla_private_http_server_remove_websocket_connection(mla_http_server_t &server,
                                                     const mla_http_server_websocket_connection_t &connection) {

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return false;
    }

    mla_int32_t index = -1;

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t& current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_equals(current_connection.id, connection.id)) {
            index = static_cast<mla_int32_t>(i);
            break;
        }

    }

    mla_rw_unlock_read(server.websocketConnectionsLock);

    if (connection.closed_executor != nullptr) {
        connection.closed_executor(connection);
    }

    if (index < 0) {
        return false;
    }

    if (!mla_rw_lock_write(server.websocketConnectionsLock)) {
        return false;
    }

    // Hot Path
    if (static_cast<mla_size_t>(index) < mla_array_list_size(server.websocketConnections)) {

        mla_http_server_websocket_connection_t& current_connection = mla_array_list_get_unsafe(server.websocketConnections, index);

        if (mla_string_equals(current_connection.id, connection.id)) {

            if (mla_array_list_remove(server.websocketConnections, index)) {
                mla_rw_unlock_write(server.websocketConnectionsLock);
                mla_info(mla_string_concat("Removed WebSocket connection with id ", connection.id, " from server "));
                return true;
            }
        }

    }

    // Fallback to linear search remove if the index is out of bounds or the id doesn't match (concurrent modification case)
    index = -1;

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t& current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_equals(current_connection.id, connection.id)) {
            index = static_cast<mla_int32_t>(i);
            break;
        }

    }

    if (index < 0) {
        mla_rw_unlock_write(server.websocketConnectionsLock);
        return false;
    }

    if (mla_array_list_remove(server.websocketConnections, index)) {
        mla_rw_unlock_write(server.websocketConnectionsLock);
        mla_info(mla_string_concat("Removed WebSocket connection with id ", connection.id, " from server "));
        return true;
    }
    mla_rw_unlock_write(server.websocketConnectionsLock);
    mla_error(mla_string_concat("Failed to remove WebSocket connection with id ", connection.id, " from server "));
    return false;
}

mla_task_process_result_state mla_private_http_server_handler_websocket_messages(mla_user_data_t& userData) {

    mla_pointer_t http_server = mla_user_data_get_pointer(userData, mla_http_task_user_data_server_name);

    mla_http_server_t* http_server_ptr = mla_pointer_get_data<mla_http_server_t>(http_server);

    if (http_server_ptr == nullptr) {
        return TASK_PROCESS_RESULT_DONE; // Server reference lost, exit task
    }

    mla_http_server_t& server = *http_server_ptr;

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    mla_size_t connectionCount = mla_array_list_size(server.websocketConnections);

    if (mla_array_list_size(server.websocketConnections) == 0) {
        mla_rw_unlock_read(server.websocketConnectionsLock);
        return TASK_PROCESS_RESULT_CONTINUE; // No connections, yield and try again
    }

    mla_array_list_t<mla_websocket_connection_array_param> copyConnections = mla_array_list<mla_websocket_connection_array_param>(connectionCount);
    mla_array_list_add_all(copyConnections, server.websocketConnections);

    mla_rw_unlock_read(server.websocketConnectionsLock);

    for (mla_size_t i = 0; i < mla_array_list_size(copyConnections); ++i) {

        if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
            return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
        }

        mla_http_server_websocket_connection_t connection = mla_array_list_get_unsafe(copyConnections, i);

        if (!mla_http_server_is_websocket_connection_open(connection)) {
            continue;
        }

        // We lock the connection for the whole processing time because
        // if multiple task are processing messages for the same connection at the same time,
        // it can cause issues with the order of messages and the state of the connection.
        // For example, if one task is processing a text message and another task receives a close message for the same connection
        if (!mla_mutex_trylock(connection.lock, 25)) {
            continue;
        }

        mla_string_t textMessage = mla_string_empty();
        mla_bytes_t binaryMessage = mla_bytes_empty();

        mla_websocket_transport_message_receive_type_t receive_type = mla_websocket_transport_receive_message(connection.connection, server.timeout_ms, textMessage, binaryMessage, false);

        enum mla_websocket_connection_action_t: mla_uint8_t {
            MLA_WEBSOCKET_CONNECTION_NONE,
            MLA_WEBSOCKET_CONNECTION_CLOSE,
            MLA_WEBSOCKET_CONNECTION_REMOVE,
        };

        mla_websocket_connection_action_t action = MLA_WEBSOCKET_CONNECTION_NONE;

        switch (receive_type) {
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TEXT:
                if (connection.text_executor != nullptr) {
                    if (!connection.text_executor(connection, textMessage)) {
                        mla_warning(mla_string_concat("WebSocket text message handler failed for connection ", connection.id, ". Closing connection."));
                        action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                    }
                } else {
                    mla_warning(mla_string_concat("No WebSocket text message handler registered for connection ", connection.id, ". Closing connection."));
                    action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                }
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_BINARY:
                if (connection.binary_executor != nullptr) {
                    if (!connection.binary_executor(connection, binaryMessage)) {
                        mla_warning(mla_string_concat("WebSocket binary message handler failed for connection ", connection.id, ". Closing connection."));
                        action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                    }
                } else {
                    mla_warning(mla_string_concat("No WebSocket binary message handler registered for connection ", connection.id, ". Closing connection."));
                    action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                }
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_CLOSED:
                action = MLA_WEBSOCKET_CONNECTION_REMOVE;
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_PONG:
                // Ignore pong messages
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT:
                action = MLA_WEBSOCKET_CONNECTION_CLOSE;
                break;
            case MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE:
                // No message received -> Nothing to do
                break;
        }

        if (action == MLA_WEBSOCKET_CONNECTION_NONE) {

            if (!mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream)) {
                mla_warning(mla_string_concat("Failed to flush WebSocket connection ", connection.id, " output stream. Closing connection."));
                action = MLA_WEBSOCKET_CONNECTION_CLOSE;
            }
        }
        mla_mutex_unlock(connection.lock);

        if (action == MLA_WEBSOCKET_CONNECTION_CLOSE) {
            mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Closing connection"));
        } else if (action == MLA_WEBSOCKET_CONNECTION_REMOVE) {
            mla_private_http_server_remove_websocket_connection(server, connection);
        }

    }


    return TASK_PROCESS_RESULT_CONTINUE;

}

mla_task_process_result_state mla_private_http_server_handler_task(mla_user_data_t& userData) {

    // Process new http request
    mla_task_process_result_state state = mla_private_http_server_handler_new_request(userData);

    if (state == TASK_PROCESS_RESULT_DONE) {
        return TASK_PROCESS_RESULT_DONE;
    }

    // Process websocket messages
    return mla_private_http_server_handler_websocket_messages(userData);
}

void mla_private_http_server_cleanup_hook(mla_http_server_internal_resource_cleanup_userdata& cleanup_userdata) {

    for (mla_size_t i = 0; i < mla_array_list_size(cleanup_userdata.active_tasks); ++i) {
        mla_string_t &task_name = mla_array_list_get_unsafe(cleanup_userdata.active_tasks, i);
        if (!mla_task_manager_abort_task(task_name)) {
            mla_warning(mla_string_concat("Failed to abort HTTP server task ", task_name));
        }
    }

    mla_sleep(100); // Give some time for tasks to abort


}

mla_bool_t mla_http_server_start(mla_http_server_t &server, mla_uint8_t number_of_tasks, mla_task_stack_size task_stack_size) {
    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (server.status == MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        return true;
    }

    if (number_of_tasks == 0) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("Http server must have at least one task to run");
        return false;
    }

    if (mla_array_list_size(server.httpHandlers) == 0 && mla_array_list_size(server.websocketHandlers) == 0) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("No handlers registered for HTTP server");
        return false;
    }

    if (!mla_network_listener_bind_and_listen(server.listener, server.host, mla_connection_type_tcp)) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error(
            mla_string_concat("Failed to bind and listen on ", server.host.address.address, ":", mla_string_from_uint16(
                server.host.port)));
        return false;
    }

    server.status = MLA_HTTP_SERVER_STATUS_RUNNING;

    server.serverOwner = mla_malloc_struct_cleanup_hook(mla_http_server_internal_resource_cleanup_userdata, mla_private_http_server_cleanup_hook);

    mla_http_server_internal_resource_cleanup_userdata *cleanup_userdata = mla_pointer_get_data<mla_http_server_internal_resource_cleanup_userdata>(server.serverOwner);

    if (cleanup_userdata == nullptr) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("Failed to allocate memory for HTTP server cleanup userdata");
        return false;
    }

    mla_memset(cleanup_userdata, 0, sizeof(mla_http_server_internal_resource_cleanup_userdata));
    cleanup_userdata->active_tasks = mla_array_list_empty<mla_string_t, mla_string_initializer>();

    mla_string_t runtime_id = mla_generate_runtime_id();

    mla_user_data_t taskUserData = mla_user_data_empty();
    mla_pointer_t http_server_ptr = mla_platform_pointer_to_managed_pointer(&server);

    mla_user_data_set_pointer(taskUserData, mla_http_task_user_data_server_name, http_server_ptr);

    for (mla_uint8_t i = 0; i < number_of_tasks; i++) {
        mla_string_t task_name = mla_string_concat("HttpServerTask_", server.host.address.address, ":",
                                                   mla_string_from_uint16(server.host.port), "_",
                                                   mla_string_from_uint8(i), "_", runtime_id);

        mla_task_t http_task = mla_task_repeating(
            task_name,
            mla_private_http_server_handler_task,
            taskUserData
        );

        mla_task_update_stack_size(http_task, task_stack_size);

        if (!mla_task_manager_register_task(http_task)) {
            server.status = MLA_HTTP_SERVER_STATUS_ERROR;
            mla_mutex_unlock(server.listenerLock);
            server.serverOwner = mla_pointer_null();
            mla_error(mla_string_concat("Failed to register HTTP server task ", http_task.name));
            return false;
        }

        mla_array_list_add(cleanup_userdata->active_tasks, task_name);
    }

    mla_mutex_unlock(server.listenerLock);
    mla_info(
        mla_string_concat("Started HTTP server on ", server.host.address.address, ":", mla_string_from_uint16(server.
            host.port), " with " , mla_string_from_uint8(number_of_tasks), " tasks"));

    return true;
}

mla_bool_t mla_http_server_running(const mla_http_server_t &server) {
    return (server.status == MLA_HTTP_SERVER_STATUS_RUNNING);
}

void mla_http_server_set_timeout(mla_http_server_t &server, mla_int32_t timeout_ms) {
    server.timeout_ms = timeout_ms;
}

mla_bool_t mla_http_server_stop(mla_http_server_t &server) {

    if (!mla_mutex_lock(server.listenerLock)) {
        return false;
    }

    if (!mla_rw_lock_write(server.websocketConnectionsLock)) {
        mla_mutex_unlock(server.listenerLock);
        return false;
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        mla_rw_unlock_write(server.websocketConnectionsLock);
        return true;
    }

    mla_bool_t all_closed = true;

    if (!mla_network_listener_close(server.listener)) {
        mla_error("Failed to close HTTP server listener");
        all_closed = false;
    }

    // Close all active websocket connections
    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {
        mla_http_server_websocket_connection_t &connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_http_server_is_websocket_connection_open(connection)) {
            mla_websocket_transport_send_close_frame(connection.connection.outputStream, mla_websocket_close_normal, mla_string_const("Server shutting down"), false);
            mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        }
    }

    mla_array_list_clear(server.websocketConnections);

    if (!all_closed) {
        mla_mutex_unlock(server.listenerLock);
        mla_rw_unlock_write(server.websocketConnectionsLock);
        // Some resources failed to close
        // Keep the server in error state
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        return false;
    }

    server.status = MLA_HTTP_SERVER_STATUS_STOPPED;
    mla_mutex_unlock(server.listenerLock);
    mla_rw_unlock_write(server.websocketConnectionsLock);
    server.serverOwner = mla_pointer_null();

    mla_info(
        mla_string_concat("HTTP server stopped on ", server.host.address.address, ":", mla_string_from_uint16(server.
            host.port)));

    return true;
}


mla_bool_t mla_http_server_find_websocket_connection(mla_http_server_t &server, const mla_string_t &connectionId,
                                                     mla_http_server_websocket_connection_t &outConnection) {
    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return false;
    }


    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {
        mla_http_server_websocket_connection_t &current_connection = mla_array_list_get_unsafe(
            server.websocketConnections, i);

        if (mla_string_equals(current_connection.id, connectionId)) {
            outConnection = current_connection;
            mla_rw_unlock_read(server.websocketConnectionsLock);
            return true;
        }
    }

    mla_rw_unlock_read(server.websocketConnectionsLock);
    return false;
}

mla_array_list_t<mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer> mla_http_server_find_websocket_connections(mla_http_server_t &server, const mla_string_t &path_prefix) {
    mla_array_list_t<mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer> result = mla_array_list_empty<mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer>();

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return result;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t &current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_starts_with(current_connection.endpoint, path_prefix)) {
            mla_array_list_add(result, current_connection);
        }
    }

    mla_rw_unlock_read(server.websocketConnectionsLock);
    return result;
}

mla_size_t mla_http_server_get_websocket_connection_count(mla_http_server_t &server, const mla_string_t &path_prefix) {
    mla_size_t count = 0;

    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return count;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketConnections); ++i) {

        mla_http_server_websocket_connection_t &current_connection = mla_array_list_get_unsafe(server.websocketConnections, i);

        if (mla_string_starts_with(current_connection.endpoint, path_prefix)) {
            count++;
        }
    }

    mla_rw_unlock_read(server.websocketConnectionsLock);
    return count;
}

mla_bool_t mla_http_server_is_websocket_connection_open(const mla_http_server_websocket_connection_t &connection) {
    if (mla_string_is_empty(connection.id)) {
        return false;
    }

    return mla_network_connection_is_connected(connection.connection);
}

mla_array_list_t<mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer> mla_http_server_get_websocket_handler_for_path(mla_http_server_t &server, const mla_string_t& path) {

    mla_array_list_t<mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer> result = mla_array_list_empty<mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer>();

    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketHandlers); ++i) {

        mla_http_server_websocket_handler_item_t &handlerItem = mla_array_list_get_unsafe(server.websocketHandlers, i);

        if (handlerItem.checker != nullptr && handlerItem.checker(handlerItem.userdata, path, MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX)) {
            mla_array_list_add(result, handlerItem);
        }
    }

    return result;
}

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_websocket_connection_t &connection,
                                                      mla_uint16_t closeCode, const mla_string_t &reason) {

    if (mla_http_server_is_websocket_connection_open(connection)) {

        if (mla_mutex_lock(connection.lock)) {

            mla_websocket_transport_send_close_frame(connection.connection.outputStream, closeCode, reason, false);
            mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
            mla_mutex_unlock(connection.lock);
        }

    }

    if (connection.server == nullptr) {
        return true;
    }

    mla_private_http_server_remove_websocket_connection(*connection.server, connection);
    connection = mla_http_server_websocket_connection_invalid();

    return true;
}

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_t &server, const mla_string_t &connectionId,
                                                      mla_uint16_t closeCode, const mla_string_t &reason) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_close_websocket_connection(connection, closeCode, reason);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t use_deflate_compression_if_supported) {
    return mla_http_server_try_send_websocket_text_message(connection, userData, message_generator, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t &connection, const mla_string_t &message, mla_bool_t use_deflate_compression_if_supported) {
    return mla_http_server_try_send_websocket_text_message(connection, message, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_t &server, const mla_string_t &connectionId, const mla_string_t &message, mla_bool_t use_deflate_compression_if_supported) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_send_websocket_text_message(connection, message, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, ((mla_int32_t)connection_lock_timeout))) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_text_with_generator(connection.connection.outputStream, userData, message_generator, false, use_deflate_compression)) {

        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }
    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    // Something is wrong with the connection, close it to avoid further issues
    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send text message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, connection_lock_timeout)) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_text_frame(connection.connection.outputStream, message, false, use_deflate_compression)) {

        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }
    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send text message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_text_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_string_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_try_send_websocket_text_message(connection, message, connection_lock_timeout, use_deflate_compression_if_supported);

}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t use_deflate_compression_if_supported) {

    return mla_http_server_try_send_websocket_binary_message(connection, userData, message_generator, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t &connection, const mla_bytes_t &message, mla_bool_t use_deflate_compression_if_supported) {

    return mla_http_server_try_send_websocket_binary_message(connection, message, -1, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t &connectionId,
                                                         const mla_bytes_t &message, mla_bool_t use_deflate_compression_if_supported) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_send_websocket_binary_message(connection, message, use_deflate_compression_if_supported);
}

mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, connection_lock_timeout)) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_binary_with_generator(connection.connection.outputStream, userData, message_generator, false, use_deflate_compression)) {
        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }
    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send binary message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    if (!mla_http_server_is_websocket_connection_open(connection)) {
        return false;
    }

    if (connection_lock_timeout <= 0) {
        if (!mla_mutex_lock(connection.lock)) {
            return false;
        }
    } else {
        if (!mla_mutex_trylock(connection.lock, connection_lock_timeout)) {
            return false;
        }
    }

    mla_bool_t use_deflate_compression = connection.supports_deflate_compression && use_deflate_compression_if_supported;

    if (mla_websocket_transport_send_binary_frame(connection.connection.outputStream, message, false, use_deflate_compression)) {
        mla_bool_t flush_successful = mla_stream_output_flush_buffered_wrapper(connection.connection.outputStream);
        mla_mutex_unlock(connection.lock);
        return flush_successful;
    }

    mla_mutex_unlock(connection.lock);

    if (connection.server == nullptr) {
        return false;
    }

    mla_http_server_close_websocket_connection(connection, mla_websocket_close_abnormal, mla_string_const("Failed to send binary message"));
    return false;

}

mla_bool_t mla_http_server_try_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_bytes_t& message, mla_int32_t connection_lock_timeout, mla_bool_t use_deflate_compression_if_supported) {

    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_try_send_websocket_binary_message(connection, message, connection_lock_timeout, use_deflate_compression_if_supported);

}
