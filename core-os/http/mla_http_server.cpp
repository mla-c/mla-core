//
// Created by chris on 10/23/2025.
//

#include "mla_http_server.h"

#include "../log/mla_logging.h"
#include "../task/mla_task_manager.h"
#include "mla_http_utils.h"

#define mla_handler_item_array_param mla_http_server_handler_item_t, mla_http_server_handler_item_initializer

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


struct mla_http_server_internal_resource_cleanup_userdata {
    mla_array_list_t<mla_string_t, mla_string_initializer> active_tasks;
};

mla_http_server_t mla_http_server_invalid() {
    return {
        mla_array_list_empty<mla_handler_item_array_param>(),
        mla_network_host_invalid(),
        mla_network_listener_invalid(),
        mla_mutex_invalid(),
        mla_buffer_reference_noOwner(),
        MLA_HTTP_SERVER_STATUS_STOPPED
    };
}

mla_http_server_t mla_http_server(const mla_network_host_t &host) {
    return {
        mla_array_list_empty<mla_handler_item_array_param>(),
        host,
        mla_network_listener_invalid(),
        mla_mutex(mla_string_concat("HttpServerListenerLock_", host.address.address, ":",
                                    mla_string_from_uint16(host.port))),
        mla_buffer_reference_noOwner(),
        MLA_HTTP_SERVER_STATUS_STOPPED
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

mla_bool_t __mla_http_server_request_read(const mla_network_connection_t &connection, mla_http_request_t &request) {
    // Read request line
    mla_string_t requestLine = mla_string_empty();
    if (!mla_http_utils_read_line(connection.inputStream, requestLine)) {
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
    if (!mla_http_utils_read_headers(request.headers, connection.inputStream)) {
        return false;
    }

    // Body (if any)
    // Check for Content-Length header
    mla_string_t contentLengthStr = mla_http_headers_get_value(request.headers, mla_string_const("Content-Length"));

    if (mla_string_equals_const(contentLengthStr, "0")) {
        request.content = mla_stream_noop_input();
        return false;
    }

    if (connection.inputStream.remaining_bytes != nullptr) {
        mla_size_t remaining = connection.inputStream.remaining_bytes(connection.inputStream);
        if (remaining == 0) {
            return false;
        }
    }

    request.content = connection.inputStream;
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

    if (!mla_stream_output_write_string(connection.outputStream, mla_string_const("\r\n"))) {
        return false;
    }

    // Send body
    if (!mla_stream_copy(response.content, connection.outputStream)) {
        return false;
    }

    return true;
}

mla_task_process_result_state __mla_http_server_handler_task(mla_callback_userdata userData) {
    mla_http_server_t &server = *reinterpret_cast<mla_http_server_t *>(userData);

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        return TASK_PROCESS_RESULT_DONE; // Server is not running, exit task
    }

    // Accept a new connection

    if (!mla_mutex_lock(server.listenerLock)) {
        return TASK_PROCESS_RESULT_CONTINUE; // No connection, yield and try again
    }

    mla_network_connection_t clientConnection = mla_network_connection_disconnected();

    if (!mla_network_listener_accept_connection(server.listener, clientConnection)) {
        mla_mutex_unlock(server.listenerLock);
        return TASK_PROCESS_RESULT_CONTINUE; // No connection, yield and try again
    }

    mla_mutex_unlock(server.listenerLock);

    mla_http_request_t request = mla_http_request_empty();

    if (!__mla_http_server_request_read(clientConnection, request)) {
        mla_warning(
            mla_string_concat("Invalid http request from client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port)));
        clientConnection = mla_network_connection_disconnected();
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    mla_http_response_t response = mla_http_response_empty();
    response.version = request.version;
    response.statusCode = 404; // Default to Not Found
    response.statusMessage = mla_string_const("Not Found");

    // Find a handler for the request
    for (mla_size_t i = 0; i < mla_array_list_size(server.handlers); i++) {
        mla_http_server_handler_item_t &handlerItem = mla_array_list_get_unsafe(server.handlers, i);

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

    if (__mla_http_server_response_send(clientConnection, response)) {
        mla_warning(
            mla_string_concat("Failed to send HTTP response to client ", clientConnection.host.address.address, ":",
                mla_string_from_uint16(clientConnection.host.port), " for URL: ", request.url));
    }
    clientConnection = mla_network_connection_disconnected();

    return TASK_PROCESS_RESULT_CONTINUE; // Yield to allow other tasks to run
}

mla_buffer_cleanup_mode __mla_http_server_cleanup_hook(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;

    mla_http_server_internal_resource_cleanup_userdata *cleanup_userdata = static_cast<
        mla_http_server_internal_resource_cleanup_userdata *>(data);

    for (mla_size_t i = 0; i < mla_array_list_size(cleanup_userdata->active_tasks); ++i) {
        mla_string_t task_name = mla_array_list_get_unsafe(cleanup_userdata->active_tasks, i);
        if (!mla_task_manager_abort_task(task_name)) {
            mla_warning(mla_string_concat("Failed to abort HTTP server task ", task_name));
        }
    }

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

    server.serverOwner = mla_buffer_reference(cleanup_userdata, true, __mla_http_server_cleanup_hook, 0);

    for (mla_uint8_t i = 0; i < number_of_tasks; i++) {
        mla_string_t task_name = mla_string_concat("HttpServerTask_", server.host.address.address, ":",
                                                   mla_string_from_uint16(server.host.port), "_",
                                                   mla_string_from_uint8(i));

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

    if (server.status != MLA_HTTP_SERVER_STATUS_RUNNING) {
        mla_mutex_unlock(server.listenerLock);
        return true;
    }

    if (!mla_network_listener_close(server.listener)) {
        mla_mutex_unlock(server.listenerLock);
        mla_error("Failed to close HTTP server listener");
        return false;
    }

    server.status = MLA_HTTP_SERVER_STATUS_STOPPED;

    mla_mutex_unlock(server.listenerLock);
    mla_info(
        mla_string_concat("HTTP server stopped on ", server.host.address.address, ":", mla_string_from_uint16(server.
            host.port)));

    return true;
}
