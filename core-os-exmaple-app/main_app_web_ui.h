//
// Created by chris on 12/19/2025.
//

#ifndef MAIN_APP_WEB_UI_H
#define MAIN_APP_WEB_UI_H

#include "../core-os/http/mla_http_server.h"
#include "../core-os/rpc/mla_rpc_http_server.h"
#include "../core-os/ui/web/mla_ui_http_server.h"
#include "../core-os/log/mla_logger_rpc.h"

#ifndef mla_app_web_ui_http_task_count
#define mla_app_web_ui_http_task_count 2
#endif

static mla_http_server_t g_main_app_web_ui_http_server = mla_http_server_invalid();

inline void main_app_web_ui_init(const mla_ui_control_surface_process_task_t &processTask) {

    mla_info("Initializing Web UI...");
    g_main_app_web_ui_http_server = mla_http_server(mla_network_host_ip4(mla_string_const("0.0.0.0"), 8081));
    mla_rpc_http_server_initialize(g_main_app_web_ui_http_server);
    mla_ui_http_server_initialize(g_main_app_web_ui_http_server);
    mla_ui_http_server_add_web_surface(g_main_app_web_ui_http_server, mla_string_const("Test UI"), mla_string_const("test"), processTask);
    mla_http_server_start(g_main_app_web_ui_http_server, mla_app_web_ui_http_task_count);

    mla_array_list_t<mla_network_ip_address_t, mla_network_ip_address_initializer_t> local_ips = mla_network_get_local_ip_addresses();

    if (mla_array_list_size(local_ips) == 0) {
        mla_warning("No local IP addresses found. Web UI may not be accessible.");
    } else {
        for (mla_size_t i = 0; i < mla_array_list_size(local_ips); i++) {

            mla_network_ip_address_t& address = mla_array_list_get_unsafe(local_ips, i);

            mla_info(mla_string_concat("Web UI accessible at http://", address.address, ":8081"));

        }
    }
}

#endif