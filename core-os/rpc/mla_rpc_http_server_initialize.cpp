//
// Created by christian on 11/18/25.
//

#include "mla_rpc_http_server.h"
#include "mla_rpc.h"
#include "../url/mla_url.h"
#include "../serializer/mla_serializer.h"
#include "../serializer/mla_json_serializer.h"
#include "../serializer/mla_binary_serializer.h"

mla_bool_t __mla_rpc_http_server_find_serializer(const mla_rpc_procedure_unsafe_t& procedure, const mla_http_response_t& response, mla_serializer_t& out_serializer) {

    // Based on Content-Type, create appropriate deserializer
    if (mla_string_equals(mla_http_headers_get_value(request.headers, mla_string_const("Content-Type")), mla_string_const("application/json"))) {
        out_serializer = mla_json_serializer(response.content);
        return true;
    } else if (mla_string_equals(mla_http_headers_get_value(request.headers, mla_string_const("Content-Type")), mla_string_const("application/octet-stream"))) {
        out_serializer = mla_binary_serializer(request.content);
        return true;
    }

    return false;
}


mla_bool_t __mla_rpc_http_server_find_deserializer(const mla_rpc_procedure_unsafe_t& procedure, const mla_http_request_t& request, mla_deserializer_t& out_deserializer) {

    // Based on Content-Type, create appropriate deserializer
    if (mla_string_equals(mla_http_headers_get_value(request.headers, mla_string_const("Content-Type")), mla_string_const("application/json"))) {
        out_deserializer = mla_json_deserializer(request.content);
        return true;
    } else if (mla_string_equals(mla_http_headers_get_value(request.headers, mla_string_const("Content-Type")), mla_string_const("application/octet-stream"))) {
        out_deserializer = mla_binary_deserializer(request.content);
        return true;
    }

    return false;
}

mla_bool_t __mla_rpc_http_server_handler(const mla_http_request_t &request, mla_http_response_t &response) {

    mla_url_t url = mla_url_empty();

    if (!mla_url_parse(request.url, url)) {
        mla_debug(mla_string_concat("Failed to parse URL ", request.url));
        return false;
    }

    // Remove "/rpc/" prefix
    mla_string_t procedure_name = mla_string_substr(url.path, 5, url.path.length - 5);

    mla_rpc_procedure_unsafe_t procedure = mla_rpc_procedure_unsafe_invalid();

    if (!mla_rpc_find_procedure(procedure_name, procedure)) {
        response.statusCode = mla_http_status_not_found;
        mla_debug(mla_string_concat("Procedure not found ", procedure_name));
        return false;
    }

    if (procedure.execute == nullptr) {
        mla_error(mla_string_concat("Procedure has no execute handler ", procedure_name));
        return false;
    }

    mla_deserializer_t deserializer = mla_deserializer_invalid();

    if (!__mla_rpc_http_server_find_deserializer(procedure, request, deserializer)) {
        mla_error(mla_string_concat("Failed to create deserializer for procedure ", procedure_name));
        return false;
    }

    // Prepare Input data
    mla_pointer_t input = nullptr;

    if (procedure.inputDefinition.data_size > 0) {
        input = mla_malloc(procedure.inputDefinition.data_size);

        if (input == nullptr) {
            return false;
        }

        mla_memset(input, 0, procedure.inputDefinition.data_size);
    }

    // Prepare Output data
    mla_pointer_t output = nullptr;

    if (procedure.outputDefinition.data_size > 0) {
        output = mla_malloc(procedure.outputDefinition.data_size);

        if (output == nullptr) {

            if (input != nullptr) {
                mla_free(input);
            }
            return false;
        }

        mla_memset(output, 0, procedure.outputDefinition.data_size);
    }

    if (input != nullptr && procedure.inputDefinition.read_function != nullptr) {

        if (!mla_deserializer_read_struct(deserializer, input, procedure.inputDefinition.read_function)) {
            response.statusCode = mla_http_status_bad_request;
            mla_error(mla_string_concat("Failed to deserialize input for procedure ", procedure_name));
            mla_free(output);
            mla_free(input);
            return false;
        }
    }

    if (procedure.execute(input, output)) {
        response.statusCode = mla_http_status_ok;

        // Serialize output
        mla_serializer_t serializer = mla_serializer_invalid();

        if (!__mla_rpc_http_server_find_serializer(procedure, response, serializer)) {
            mla_error(mla_string_concat("Failed to create serializer for procedure ", procedure_name));
            mla_free(output);
            mla_free(input);
            return false;
        }

        if (output != nullptr && procedure.outputDefinition.write_function != nullptr) {
            mla_serializer_write_struct(serializer, output, procedure.outputDefinition.write_function);
        }

    } else {
        response.statusCode = mla_http_status_internal_server_error;
    }

    mla_free(output);
    mla_free(input);

    return true;

}

mla_bool_t mla_rpc_http_server_initialize(mla_http_server_t &server) {

    mla_http_server_handler_item_t handler = mla_http_server_handler_starts_with(mla_http_method_post, mla_string_const("/rpc/"), __mla_rpc_http_server_handler);
    return mla_http_server_register_handler(server, handler);
}