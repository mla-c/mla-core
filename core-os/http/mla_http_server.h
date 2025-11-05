//
// Created by chris on 10/23/2025.
//

#ifndef COREOS_MLA_HTTP_SERVER_H
#define COREOS_MLA_HTTP_SERVER_H

#include "mla_http_request.h"
#include "mla_http_response.h"
#include "../network/mla_network.h"
#include "../task/mla_mutx.h"

enum mla_http_server_status_t: mla_uint8_t {
    MLA_HTTP_SERVER_STATUS_STOPPED,
    MLA_HTTP_SERVER_STATUS_RUNNING,
    MLA_HTTP_SERVER_STATUS_ERROR
};

typedef mla_bool_t (*mla_http_request_handler_t)(const mla_http_request_t &request, mla_http_response_t &response);
typedef mla_bool_t (*mla_http_request_handler_checker_t)(const mla_callback_userdata &userdata, const mla_string_t& url);

struct mla_http_server_handler_item_t {
    mla_callback_userdata userdata;
    mla_string_t method;
    mla_http_request_handler_checker_t checker;
    mla_http_request_handler_t executor;
    mla_buffer_reference_t userDataOwner;
};

mla_http_server_handler_item_t mla_http_server_handler(const mla_string_t &http_method, const mla_callback_userdata& userdata, const mla_http_request_handler_checker_t& checker, const mla_http_request_handler_t& executor, const mla_buffer_reference_t& userDataOwner);
mla_http_server_handler_item_t mla_http_server_handler_starts_with(const mla_string_t &http_method, mla_string_t pathPrefix, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_all(const mla_string_t &http_method, const mla_http_request_handler_t& executor);
mla_http_server_handler_item_t mla_http_server_handler_invalid();

struct mla_http_server_handler_item_initializer {
    static mla_http_server_handler_item_t init() {
        return {
            0,
            mla_string_empty(),
            nullptr,
            nullptr,
            mla_buffer_reference_noOwner()
        };
    }
};

struct mla_http_server_t {
    mla_array_list_t<mla_http_server_handler_item_t, mla_http_server_handler_item_initializer> handlers;
    mla_network_host_t host;
    mla_network_listener_t listener;
    mla_mutex_t listenerLock;
    mla_buffer_reference_t serverOwner;
    mla_http_server_status_t status;
    mla_int32_t timeout_ms;
};


mla_http_server_t mla_http_server_invalid();
mla_http_server_t mla_http_server(const mla_network_host_t &host);

mla_bool_t mla_http_server_register_handler(mla_http_server_t &server, const mla_http_server_handler_item_t& handlerItem);

mla_bool_t mla_http_server_start(mla_http_server_t &server, mla_uint8_t number_of_tasks);
mla_bool_t mla_http_server_stop(mla_http_server_t &server);


////////////////////////////////////////////////////////////////
/// Helpers
////////////////////////////////////////////////////////////////

inline void mla_http_server_destroy(mla_http_server_t& server) {
    mla_http_server_stop(server);
    server = mla_http_server_invalid();
}

#endif