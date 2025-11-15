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

#define mla_handler_item_array_param mla_http_server_handler_item_t, mla_http_server_handler_item_initializer
#define mla_websocket_handler_item_array_param mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer
#define mla_websocket_connection_array_param mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer

mla_http_server_handler_item_t mla_http_server_handler(const mla_string_t &http_method,
                                                       const mla_callback_userdata &userdata,
                                                       const mla_http_request_handler_checker_t &checker,
                                                       const mla_http_request_handler_t &executor,
                                                       const mla_buffer_reference_t &userDataOwner) {
    return {
        userdata,
        mla_string_to_upper(http_method),
        checker,
        executor,
        userDataOwner
    };
}

mla_bool_t __mla_http_server_handler_starts_with_checker(const mla_callback_userdata &userdata,
                                                         const mla_string_t &url) {
    mla_char_t *pathPrefix = reinterpret_cast<mla_char_t *>(userdata);

    if (pathPrefix == nullptr)
        return false;

    mla_string_t str_prefix = mla_string_from_buffer_without_ownership(pathPrefix, mla_strlen(pathPrefix));

    return mla_string_starts_with(url, str_prefix);
}

mla_bool_t __mla_http_server_handler_equals_checker(const mla_callback_userdata &userdata,
                                                    const mla_string_t &url) {
    mla_char_t *pathPrefix = reinterpret_cast<mla_char_t *>(userdata);

    if (pathPrefix == nullptr)
        return false;

    mla_string_t str_prefix = mla_string_from_buffer_without_ownership(pathPrefix, mla_strlen(pathPrefix));

    return mla_string_equals(url, str_prefix);
}

mla_http_server_handler_item_t mla_http_server_handler_starts_with(const mla_string_t &http_method,
                                                                   mla_string_t pathPrefix,
                                                                   const mla_http_request_handler_t &executor) {
    mla_c_string_t c_string = mla_string_to_cString(pathPrefix, true);

    if (c_string.c_str == nullptr) {
        return mla_http_server_handler_invalid();
    }

    if (!c_string.isOwner) {
        return mla_http_server_handler_invalid();
    }

    return {
        reinterpret_cast<mla_callback_userdata>(c_string.c_str),
        mla_string_to_upper(http_method),
        __mla_http_server_handler_starts_with_checker,
        executor,
        mla_buffer_reference(c_string.c_str, true)
    };
}

mla_bool_t __mla_http_server_handler_all_checker(const mla_callback_userdata &userdata, const mla_string_t &url) {
    (void) userdata;
    (void) url;
    return true;
}

mla_http_server_handler_item_t mla_http_server_handler_all(const mla_string_t &http_method,
                                                           const mla_http_request_handler_t &executor) {
    return {
        0,
        mla_string_to_upper(http_method),
        __mla_http_server_handler_all_checker,
        executor,
        mla_buffer_reference_noOwner()
    };
}

mla_http_server_handler_item_t mla_http_server_handler_invalid() {
    return {
        0,
        mla_string_empty(),
        nullptr,
        nullptr,
        mla_buffer_reference_noOwner()
    };
}

mla_http_server_websocket_connection_t mla_http_server_websocket_connection_invalid() {
    return {
        mla_network_connection_disconnected(),
        mla_string_empty(),
        mla_mutex_invalid()
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler(
    const mla_callback_userdata &userdata, const mla_http_request_handler_checker_t &checker,
    const mla_http_websocket_text_message_handler_t &text_message_handler,
    const mla_http_websocket_binary_message_handler_t &binary_message_handler,
    const mla_buffer_reference_t &userDataOwner) {
    return {
        userdata,
        checker,
        text_message_handler,
        binary_message_handler,
        userDataOwner
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_path_equals(
    mla_string_t path, const mla_http_websocket_text_message_handler_t &text_message_handler,
    const mla_http_websocket_binary_message_handler_t &binary_message_handler) {
    mla_c_string_t c_string = mla_string_to_cString(path, true);

    if (c_string.c_str == nullptr) {
        return mla_http_server_websocket_handler_invalid();
    }

    if (!c_string.isOwner) {
        return mla_http_server_websocket_handler_invalid();
    }

    return {
        reinterpret_cast<mla_callback_userdata>(c_string.c_str),
        __mla_http_server_handler_equals_checker,
        text_message_handler,
        binary_message_handler,
        mla_buffer_reference(c_string.c_str, true)
    };
}

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_invalid() {
    return {
        0,
        nullptr,
        nullptr,
        nullptr,
        mla_buffer_reference_noOwner()
    };
}


struct mla_http_server_internal_resource_cleanup_userdata {
    mla_array_list_t<mla_string_t, mla_string_initializer> active_tasks;
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
        mla_buffer_reference_noOwner(),
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
                                    mla_string_from_uint16(host.port))),
        host,
        mla_network_listener_invalid(),
        mla_mutex(mla_string_concat("HttpServerListenerLock_", host.address.address, ":",
                                    mla_string_from_uint16(host.port))),
        mla_buffer_reference_noOwner(),
        MLA_HTTP_SERVER_STATUS_STOPPED,
        mla_default_http_timeout_ms
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

    mla_bool_t result = mla_array_list_add(server.handlers, handlerItem);

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

mla_bool_t __mla_http_server_request_read(const mla_network_connection_t &connection, mla_http_request_t &request,
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
        return false;
    }

    mla_size_t content_size = 0;

    if (mla_parse_uint32(contentLengthStr, content_size)) {
        request.content = mla_http_content_input_stream(connection.inputStream, timeout_ms, content_size);
        return true;
    }

    // No Content-Length, use chunked or until close
    mla_string_t transferEncoding = mla_http_headers_get_value(request.headers, mla_string_const("Transfer-Encoding"));

    if (mla_string_equals_const(transferEncoding, "chunked")) {
        mla_warning(mla_string_const("Chunked transfer encoding is not supported"));
        return false;
    }

    request.content = mla_stream_noop_input();
    return true;
}

mla_bool_t __mla_http_server_response_send(const mla_network_connection_t &connection,
                                           const mla_http_response_t &response) {
    // Send status line
    mla_string_t httpVersion = (response.version == MLA_HTTP_VERSION_1_1)
                                   ? mla_string_const("HTTP/1.1 ")
                                   : mla_string_const("HTTP/1.0 ");

    if (!mla_stream_output_write_string(connection.outputStream, httpVersion)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_from_uint16(response.statusCode))) {
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const(" "))) {
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, response.statusMessage)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        return false;
    }

    // Send headers
    if (!mla_http_utils_write_headers(response.headers, connection.outputStream)) {
        return false;
    }

    // Write Content Length if body is present
    if (!mla_http_utils_write_content_headers(response.headers, response.content, connection.outputStream)) {
        return false;
    }

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        return false;
    }

    // Send body
    if (!mla_stream_copy(response.content, connection.outputStream)) {
        return false;
    }

    return true;
}

mla_task_process_result_state __mla_http_server_handler_new_request(mla_callback_userdata userData) {

    mla_http_server_t& server = *reinterpret_cast<mla_http_server_t *>(userData);

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    // Accept a new connection

    if (!mla_mutex_lock_timeout(server.listenerLock, 100)) {
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

    mla_http_request_t request = mla_http_request_empty();

    if (!__mla_http_server_request_read(clientConnection, request, server.timeout_ms)) {
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

    mla_array_list_t<mla_http_server_handler_item_t, mla_http_server_handler_item_initializer> copyHandlers = server.
            handlers;

    mla_http_response_t response = mla_http_response_empty();
    response.version = request.version;
    response.statusCode = 404; // Default to Not Found
    response.statusMessage = mla_string_const("Not Found");

    // Find a handler for the request
    for (mla_size_t i = 0; i < mla_array_list_size(copyHandlers); i++) {
        mla_http_server_handler_item_t &handlerItem = mla_array_list_get_unsafe(copyHandlers, i);

        if (handlerItem.checker == nullptr) {
            continue; // No checker function, skip
        }

        if (!mla_string_equals(request.method, handlerItem.method)) {
            continue;
        }

        if (!handlerItem.checker(handlerItem.userdata, request.url)) {
            continue; // Checker returned false, skip
        }

        if (handlerItem.executor != nullptr) {
            if (!handlerItem.executor(request, response)) {
                mla_warning(mla_string_concat("Handler executor failed for URL: ", request.url));
                response.statusCode = 500; // Internal Server Error
                response.statusMessage = mla_string_const("Internal Server Error");
            }
        }

        break;
    }

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_DONE; // Server stopped while accepting, exit task
    }

    if (!__mla_http_server_response_send(clientConnection, response)) {
        mla_warning(
            mla_string_concat("Failed to send HTTP response to client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port), " for URL: ", request.url));
    }
    clientConnection = mla_network_connection_disconnected();

    return TASK_PROCESS_RESULT_CONTINUE; // Yield to allow other tasks to run

}

mla_task_process_result_state __mla_http_server_handler_websocket_messages(mla_callback_userdata userData) {
    mla_http_server_t& server = *reinterpret_cast<mla_http_server_t *>(userData);

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    mla_array_list_t<mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer> copyConnections = server.
            websocketConnections;



}

mla_task_process_result_state __mla_http_server_handler_task(mla_callback_userdata userData) {

    // Process new http request
    mla_task_process_result_state state = __mla_http_server_handler_new_request(userData);

    if (state == TASK_PROCESS_RESULT_DONE) {
        return TASK_PROCESS_RESULT_DONE;
    }

    // Process websocket messages
    return __mla_http_server_handler_websocket_messages(userData);
}

mla_buffer_cleanup_mode __mla_http_server_cleanup_hook(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;

    mla_http_server_internal_resource_cleanup_userdata *cleanup_userdata = static_cast<
        mla_http_server_internal_resource_cleanup_userdata *>(data);

    for (mla_size_t i = 0; i < mla_array_list_size(cleanup_userdata->active_tasks); ++i) {
        mla_string_t &task_name = mla_array_list_get_unsafe(cleanup_userdata->active_tasks, i);
        if (!mla_task_manager_abort_task(task_name)) {
            mla_warning(mla_string_concat("Failed to abort HTTP server task ", task_name));
        }
    }

    mla_sleep(100); // Give some time for tasks to abort

    // Free active tasks list
    cleanup_userdata->active_tasks = mla_array_list_empty<mla_string_t, mla_string_initializer>();

    return CLEAN_UP_NEEDED;
}

mla_bool_t mla_http_server_start(mla_http_server_t &server, mla_uint8_t number_of_tasks) {
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

    if (mla_array_list_size(server.handlers) == 0) {
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
    mla_http_server_internal_resource_cleanup_userdata *cleanup_userdata = static_cast<
        mla_http_server_internal_resource_cleanup_userdata *>(mla_malloc(
        sizeof(mla_http_server_internal_resource_cleanup_userdata)));

    if (cleanup_userdata == nullptr) {
        server.status = MLA_HTTP_SERVER_STATUS_ERROR;
        mla_mutex_unlock(server.listenerLock);
        mla_error("Failed to allocate memory for HTTP server cleanup userdata");
        return false;
    }

    mla_memset(cleanup_userdata, 0, sizeof(mla_http_server_internal_resource_cleanup_userdata));
    cleanup_userdata->active_tasks = mla_array_list_empty<mla_string_t, mla_string_initializer>();

    server.serverOwner = mla_buffer_reference(cleanup_userdata, true, __mla_http_server_cleanup_hook, 0);

    mla_string_t runtime_id = mla_generate_runtime_id();

    for (mla_uint8_t i = 0; i < number_of_tasks; i++) {
        mla_string_t task_name = mla_string_concat("HttpServerTask_", server.host.address.address, ":",
                                                   mla_string_from_uint16(server.host.port), "_",
                                                   mla_string_from_uint8(i), "_", runtime_id);

        mla_task_t http_task = mla_task_repeating(
            task_name,
            __mla_http_server_handler_task,
            reinterpret_cast<mla_callback_userdata>(&server)
        );

        if (!mla_task_manager_register_task(http_task)) {
            server.status = MLA_HTTP_SERVER_STATUS_ERROR;
            mla_mutex_unlock(server.listenerLock);
            server.serverOwner = mla_buffer_reference_noOwner();
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
            mla_websocket_transport_send_close_frame(connection.connection.outputStream, mla_websocket_close_normal, mla_string_const("Server shutting down"));
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
    server.serverOwner = mla_buffer_reference_noOwner();

    mla_info(
        mla_string_concat("HTTP server stopped on ", server.host.address.address, ":", mla_string_from_uint16(server.
            host.port)));

    return true;
}

mla_bool_t __http_server_remove_websocket_connection(mla_http_server_t &server,
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

    if (index < 0) {
        return false;
    }

    mla_rw_lock_write(server.websocketConnectionsLock);
    mla_bool_t removed = mla_array_list_remove(server.websocketConnections, index);
    mla_rw_unlock_write(server.websocketConnectionsLock);
    return removed;
}

mla_bool_t mla_http_server_find_websocket_connection(mla_http_server_t &server, const mla_string_t &connectionId,
                                                     mla_http_server_websocket_connection_t &outConnection) {
    if (!mla_rw_lock_read(server.websocketConnectionsLock)) {
        return false;
    }


    for (mla_size_t i = 0; i < mla_array_list_size(server.websocketHandlers); ++i) {
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

mla_bool_t mla_http_server_is_websocket_connection_open(const mla_http_server_websocket_connection_t &connection) {
    if (mla_string_is_empty(connection.id))
        return false;

    return mla_network_connection_is_connected(connection.connection);
}

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_t &server,
                                                      mla_http_server_websocket_connection_t &connection,
                                                      mla_uint16_t closeCode, const mla_string_t &reason) {

    if (mla_http_server_is_websocket_connection_open(connection)) {

        if (mla_mutex_lock(connection.lock)) {

            mla_websocket_transport_send_close_frame(connection.connection.outputStream, closeCode, reason);
            mla_mutex_unlock(connection.lock);
        }

        connection = mla_http_server_websocket_connection_invalid();
    }

    __http_server_remove_websocket_connection(server, connection);

    return true;
}

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_t &server, const mla_string_t &connectionId,
                                                      mla_uint16_t closeCode, const mla_string_t &reason) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_close_websocket_connection(server, connection, closeCode, reason);
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_t &server,
                                                       mla_http_server_websocket_connection_t &connection,
                                                       const mla_string_t &message, mla_bool_t is_final) {

    if (!mla_http_server_is_websocket_connection_open(connection))
        return false;

    if (!mla_mutex_lock(connection.lock))
        return false;

    if (mla_websocket_transport_send_text_frame(connection.connection.outputStream, message, is_final)) {
        mla_mutex_unlock(connection.lock);
        return true;
    }
    mla_mutex_unlock(connection.lock);

    mla_http_server_close_websocket_connection(server, connection, mla_websocket_close_abnormal, mla_string_const("Failed to send text message"));
    return false;
}

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_t &server, const mla_string_t &connectionId,
                                                       const mla_string_t &message, mla_bool_t is_final) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_send_websocket_text_message(server, connection, message, is_final);
}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_t &server,
                                                         mla_http_server_websocket_connection_t &connection,
                                                         const mla_bytes_t &message, mla_bool_t is_final) {

    if (!mla_http_server_is_websocket_connection_open(connection))
        return false;

    if (!mla_mutex_lock(connection.lock))
        return false;

    if (mla_websocket_transport_send_binary_frame(connection.connection.outputStream, message, is_final)) {
        mla_mutex_unlock(connection.lock);
        return true;
    }

    mla_mutex_unlock(connection.lock);

    mla_http_server_close_websocket_connection(server, connection, mla_websocket_close_abnormal, mla_string_const("Failed to send binary message"));
    return false;

}

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t &connectionId,
                                                         const mla_bytes_t &message, mla_bool_t is_final) {
    mla_http_server_websocket_connection_t connection = mla_http_server_websocket_connection_invalid();

    if (!mla_http_server_find_websocket_connection(server, connectionId, connection)) {
        return false;
    }

    return mla_http_server_send_websocket_binary_message(server, connection, message, is_final);
}
