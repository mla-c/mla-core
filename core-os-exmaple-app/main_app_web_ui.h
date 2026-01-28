//
// Created by chris on 12/19/2025.
//

#ifndef MAIN_APP_WEB_UI_H
#define MAIN_APP_WEB_UI_H

#include "../core-os/http/mla_http_server.h"
#include "../core-os/rpc/mla_rpc_http_server.h"
#include "../core-os/ui/web/mla_ui_http_server.h"
#include "../core-os/log/mla_logger_rpc.h"

static mla_http_server_t g_main_app_web_ui_http_server = mla_http_server_invalid();

inline void main_app_web_ui_init() {

    mla_info("Initializing Web UI...");
    g_main_app_web_ui_http_server = mla_http_server(mla_network_host_ip4(mla_string_const("127.0.0.1"), 8081));
    mla_rpc_http_server_initialize(g_main_app_web_ui_http_server);
    mla_ui_http_server_initialize(g_main_app_web_ui_http_server);
    mla_http_server_start(g_main_app_web_ui_http_server, 4);
    mla_info("Web UI Initialized and running at http://127.0.0.1:8081");

}

#endif