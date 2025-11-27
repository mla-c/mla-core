//
// Created by christian on 11/24/25.
//

#ifndef COREOS_MLA_RPC_HTTP_DATA_TYPES_H
#define COREOS_MLA_RPC_HTTP_DATA_TYPES_H

#include "../mla_data_types.h"

#define mla_rpc_stream_small_buffer_size 1024 // 1 KB

enum mla_http_rpc_content_type: mla_uint8_t {
    mla_http_rpc_content_type_unknown,
    mla_http_rpc_content_type_json,
    mla_http_rpc_content_type_binary
};


#endif