//
// Created by christian on 11/24/25.
//

#ifndef MLA_MLA_RPC_HTTP_CLIENT_H
#define MLA_MLA_RPC_HTTP_CLIENT_H
#include "mla_rpc.h"
#include "mla_rpc_http_data_types.h"

mla_rpc_remote_endpoint_t mla_rpc_http_register_endpoint(const mla_string_t& server_url, mla_http_rpc_content_type content_type);

#endif
