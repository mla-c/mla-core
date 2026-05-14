//
// Created by christian on 11/24/25.
//

#ifndef MLA_MLA_RPC_HTTP_DATA_TYPES_H
#define MLA_MLA_RPC_HTTP_DATA_TYPES_H

#include "../mla_data_types.h"

enum mla_http_rpc_content_type: mla_uint8_t {
    mla_http_rpc_content_type_unknown,
    mla_http_rpc_content_type_json,
    mla_http_rpc_content_type_binary
};


#endif