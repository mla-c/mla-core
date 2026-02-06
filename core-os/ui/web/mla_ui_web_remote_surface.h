//
// Created by chris on 2/5/2026.
//

#ifndef COREOS_APPLICATION_TEST_MLA_UI_WEB_REMOTE_SURFACE_H
#define COREOS_APPLICATION_TEST_MLA_UI_WEB_REMOTE_SURFACE_H

#include "../surfaces/mla_ui_surface.h"
#include "../../http/mla_http_server.h"

mla_ui_surface_t mla_ui_web_remote_surface_create(const mla_http_server_websocket_connection_t& connection);

mla_bool_t mla_ui_web_remote_surface_register_handlers(mla_http_server_t& http_server, );

#endif //COREOS_APPLICATION_TEST_MLA_UI_WEB_REMOTE_SURFACE_H