//
// Created by christian on 11/18/25.
//

#include "mla_rpc_http_server.h"
#include "mla_rpc.h"
#include "../url/mla_url.h"
#include "../serializer/mla_serializer.h"

mla_bool_t __mla_rpc_http_server_find_deserializer(const mla_rpc_procedure_unsafe_t& procedure, const mla_http_request_t& request, mla_deserializer_t& out_deserializer) {

    // For simplicity, we assume JSON deserializer for all procedures
    //mla_stream_input_t input_stream = mla_stream_input_from_buffer_reference(request.body);
    //out_deserializer = mla_json_deserializer(input_stream);

    return false;
}

mla_bool_t __mla_rpc_http_server_handler(const mla_http_request_t &request, mla_http_response_t &response) {

    mla_url_t url = mla_url_empty();

    if (!mla_url_parse(request.url, url)) {
        mla_debug(mla_string_concat("RPC HTTP Server: Failed to parse URL ", request.url));
        return false;
    }

    // Remove "/rpc/" prefix
    mla_string_t procedure_name = mla_string_substr(url.path, 5, url.path.length - 5);

    mla_rpc_procedure_unsafe_t procedure = mla_rpc_procedure_unsafe_invalid();

    if (!mla_rpc_find_procedure(procedure_name, procedure)) {
        mla_debug(mla_string_concat("RPC HTTP Server: Procedure not found ", procedure_name));
        return false;
    }

    mla_deserializer_t deserializer = mla_deserializer_invalid();

    if (!__mla_rpc_http_server_find_deserializer(procedure, request, deserializer)) {
        mla_error(mla_string_concat("RPC HTTP Server: Failed to create deserializer for procedure ", procedure_name));
        return false;
    }




}

mla_bool_t mla_rpc_http_server_initialize(mla_http_server_t &server) {

    mla_http_server_handler_item_t handler = mla_http_server_handler_starts_with(mla_http_method_post, mla_string_const("/rpc/"), __mla_rpc_http_server_handler);
    return mla_http_server_register_handler(server, handler);
}