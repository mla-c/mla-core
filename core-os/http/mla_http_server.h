//
// Created by chris on 10/23/2025.
//

#ifndef COREOS_MLA_HTTP_SERVER_H
#define COREOS_MLA_HTTP_SERVER_H

#include "mla_http_request.h"
#include "mla_http_response.h"
#include "../network/mla_network.h"
#include "../task/mla_mutx.h"
#include "../task/mla_rw_lock.h"

enum mla_http_server_status_t: mla_uint8_t {
    MLA_HTTP_SERVER_STATUS_STOPPED,
    MLA_HTTP_SERVER_STATUS_RUNNING,
    MLA_HTTP_SERVER_STATUS_ERROR
};

typedef mla_bool_t (*mla_http_request_handler_checker_t)(const mla_user_data_t &userdata, const mla_string_t& url);

///////////////////////////////////
/// HTTP Server Handler Item
//////////////////////////////////

typedef mla_bool_t (*mla_http_request_handler_t)(const mla_http_request_t &request, mla_http_response_t &response);

struct mla_http_server_handler_item_t {
    mla_user_data_t userdata;
    mla_string_t method;
    mla_http_request_handler_checker_t checker;
    mla_http_request_handler_t executor;
};

mla_http_server_handler_item_t mla_http_server_handler(const mla_string_t &http_method, mla_user_data_t& userdata, const mla_http_request_handler_checker_t& checker, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_starts_with(const mla_string_t &http_method, const mla_string_t& pathPrefix, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_all(const mla_string_t &http_method, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_invalid();

struct mla_http_server_handler_item_initializer {
    static mla_http_server_handler_item_t init() {
        return {
            mla_user_data_empty(),
            mla_string_empty(),
            nullptr,
            nullptr
        };
    }
};

///////////////////////////////////
/// HTTP Server WebSocket Handler Item
//////////////////////////////////

struct mla_http_server_t;
struct mla_http_server_websocket_connection_t;

typedef mla_bool_t (*mla_http_websocket_open_handler_t)(mla_http_server_websocket_connection_t& connection);
typedef mla_bool_t (*mla_http_websocket_text_message_handler_t)(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_bool_t isFinalFragment);
typedef mla_bool_t (*mla_http_websocket_binary_message_handler_t)(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message, mla_bool_t isFinalFragment);
typedef void (*mla_http_websocket_closed_handler_t)(const mla_http_server_websocket_connection_t& connection);

struct mla_http_server_websocket_connection_t {
    mla_http_server_t* server;
    mla_network_connection_t connection;
    mla_string_t id;
    mla_mutex_t lock;
    mla_user_data_t userdata;
    mla_http_websocket_open_handler_t open_executor;
    mla_http_websocket_text_message_handler_t text_executor;
    mla_http_websocket_binary_message_handler_t binary_executor;
    mla_http_websocket_closed_handler_t closed_executor;
};

mla_http_server_websocket_connection_t mla_http_server_websocket_connection_invalid();

struct mla_http_server_websocket_connection_initializer {
    static mla_http_server_websocket_connection_t init() {
        return mla_http_server_websocket_connection_invalid();
    }
};

struct mla_http_server_websocket_handler_item_t {
    mla_user_data_t userdata;
    mla_http_request_handler_checker_t checker;
    mla_http_websocket_open_handler_t open_executor;
    mla_http_websocket_text_message_handler_t text_executor;
    mla_http_websocket_binary_message_handler_t binary_executor;
    mla_http_websocket_closed_handler_t close_executor;
};

mla_http_server_websocket_connection_t mla_http_server_websocket_connection(mla_http_server_t& http_server, mla_network_connection_t& connection, const mla_http_server_websocket_handler_item_t& handlerItem);

mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler(mla_user_data_t& userdata, const mla_http_request_handler_checker_t& checker, const mla_http_websocket_text_message_handler_t& text_message_handler, const mla_http_websocket_binary_message_handler_t& binary_message_handler);
mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_path_equals(const mla_string_t& path, const mla_http_websocket_text_message_handler_t& text_message_handler, const mla_http_websocket_binary_message_handler_t& binary_message_handler);
mla_http_server_websocket_handler_item_t mla_http_server_websocket_handler_invalid();

struct mla_http_server_websocket_handler_item_initializer {
    static mla_http_server_websocket_handler_item_t init() {
        return {
            mla_user_data_empty(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        };
    }
};

////////////////////////////////////
/// HTTP Server
///////////////////////////////////

struct mla_http_server_t {
    mla_array_list_t<mla_http_server_handler_item_t, mla_http_server_handler_item_initializer> httpHandlers;
    mla_array_list_t<mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer> websocketHandlers;
    mla_array_list_t<mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer> websocketConnections;
    mla_rw_lock_t websocketConnectionsLock;
    mla_network_host_t host;
    mla_network_listener_t listener;
    mla_mutex_t listenerLock;
    mla_buffer_reference_t serverOwner;
    mla_http_server_status_t status;
    mla_int32_t timeout_ms;
};

// Lifecycle Methods
mla_http_server_t mla_http_server_invalid();
mla_http_server_t mla_http_server(const mla_network_host_t &host);
mla_bool_t mla_http_server_start(mla_http_server_t &server, mla_uint8_t number_of_tasks);
mla_bool_t mla_http_server_stop(mla_http_server_t &server);

// Registration Methods
mla_bool_t mla_http_server_register_handler(mla_http_server_t &server, const mla_http_server_handler_item_t& handlerItem);
mla_bool_t mla_http_server_register_websocket_handler(mla_http_server_t &server, const mla_http_server_websocket_handler_item_t& handlerItem);

// WebSocket Methods
mla_bool_t mla_http_server_find_websocket_connection(mla_http_server_t &server, const mla_string_t& connectionId, mla_http_server_websocket_connection_t& outConnection);
mla_bool_t mla_http_server_is_websocket_connection_open(const mla_http_server_websocket_connection_t& connection);

mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_websocket_connection_t& connection, mla_uint16_t closeCode, const mla_string_t& reason);
mla_bool_t mla_http_server_close_websocket_connection(mla_http_server_t &server, const mla_string_t& connectionId, mla_uint16_t closeCode, const mla_string_t& reason);

mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_bool_t is_final);
mla_bool_t mla_http_server_send_websocket_text_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_string_t& message, mla_bool_t is_final);

mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_websocket_connection_t& connection, const mla_bytes_t& message, mla_bool_t is_final);
mla_bool_t mla_http_server_send_websocket_binary_message(mla_http_server_t &server, const mla_string_t& connectionId, const mla_bytes_t& message, mla_bool_t is_final);


////////////////////////////////////////////////////////////////
/// Helpers
////////////////////////////////////////////////////////////////

inline void mla_http_server_destroy(mla_http_server_t& server) {
    mla_http_server_stop(server);
    server = mla_http_server_invalid();
}

#endif