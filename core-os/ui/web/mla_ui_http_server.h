//
// Created by christian on 12/5/25.
//

#ifndef MLA_C_MLA_UI_HTTP_H
#define MLA_C_MLA_UI_HTTP_H

#include "../../http/mla_http_server.h"
#include "../controls/mla_ui_control_surface.h"

mla_bool_t mla_ui_http_server_initialize(mla_http_server_t &server);
mla_bool_t mla_ui_http_server_add_web_surface(mla_http_server_t& http_server, const mla_string_t& display_name, const mla_string_t &surface_name, mla_ui_control_surface_process_task_t processTask);

#endif