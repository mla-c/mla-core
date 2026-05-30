//
// Created by christian on 11/24/25.
//

#include "mla_rpc_http_client.h"
#include "../http/mla_http_client.h"
#include "../system/mla_string_concat.h"
#include "../log/mla_logging.h"
#include "../serializer/mla_serializer.h"
#include "../serializer/mla_binary_serializer.h"
#include "../serializer/mla_json_serializer.h"
#include "../http/mla_http_chunked_stream.h"

struct mla_rpc_http_client_config {
    mla_string_t server_url;
    mla_http_rpc_content_type content_type;

    static mla_rpc_http_client_config init() {
        return {
            mla_string_empty(),
            mla_http_rpc_content_type_unknown
        };
    }
};

struct mla_rpc_http_request_body_config {
    mla_serialize_definition_t input_definition;
    mla_pointer_t input_data;
    mla_http_rpc_content_type content_type;

    static mla_rpc_http_request_body_config init() {
        return {
            mla_serialize_definition_invalid(),
            mla_pointer_null(),
            mla_http_rpc_content_type_unknown
        };
    }
};


mla_string_t __mla_http_rpc_content_type_to_string(mla_http_rpc_content_type content_type) {

    switch (content_type) {
        case mla_http_rpc_content_type_json:
            return mla_string_const("application/json");
        case mla_http_rpc_content_type_binary:
            return mla_string_const("application/octet-stream");
        default:
            return mla_string_const("Invalid-Content-Type");
    }
}

mla_bool_t __mla_http_rpc_request_content_write(mla_http_rpc_content_type content_type, const mla_stream_output_t &outputStream, const mla_pointer_t& input_data, const mla_serialize_definition_write_function_t &write_function) {

    mla_serializer_t serializer = mla_serializer_invalid();

    if (content_type == mla_http_rpc_content_type_json) {
        serializer = mla_json_serializer(outputStream);
    } else if (content_type == mla_http_rpc_content_type_binary) {
        serializer = mla_binary_serializer(outputStream);
    } else {
        mla_error(mla_string_const("Unsupported Content-Type in HTTP RPC Request"));
        return false;
    }


    if (!mla_serializer_write_data_struct(serializer, input_data, write_function)) {
        return false;
    }

    return true;

}

mla_user_data_id_init(mla_http_rpc_request_content_writer_body_config_user_data_name)

mla_bool_t __mla_http_rpc_request_content_writer(const mla_http_request_content_writer_t& writer, const mla_stream_output_t &outputStream) {

    mla_pointer_t body_config_ptr = mla_user_data_get_pointer(writer.userData, mla_http_rpc_request_content_writer_body_config_user_data_name);

    mla_rpc_http_request_body_config* body_config = mla_pointer_get_data<mla_rpc_http_request_body_config>(body_config_ptr);

    if (body_config == nullptr) {
        return false;
    }

    mla_http_chunked_stream_output_t chunked_output = mla_http_chunked_stream_output(outputStream);

    mla_bool_t result = __mla_http_rpc_request_content_write(body_config->content_type, chunked_output.output, body_config->input_data, body_config->input_definition.write_function);

    // Finalize chunked output
    if (!mla_http_chunked_stream_output_finished(chunked_output)) {
        return false;
    }

    return result;

}

mla_user_data_id_init(mla_rpc_http_client_config_user_data_name)

mla_bool_t __mla_rpc_http_execute(const mla_user_data_t &userdata, const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t& input_data, mla_pointer_t& output_data) {

    mla_pointer_t config_ptr = mla_user_data_get_pointer(userdata, mla_rpc_http_client_config_user_data_name);

    mla_rpc_http_client_config* config = mla_pointer_get_data<mla_rpc_http_client_config>(config_ptr);

    if (config == nullptr) {
        return false;
    }

    mla_string_t url = mla_string_empty();
    mla_string_t slash = mla_string_const("/");

    if (mla_string_ends_with(config->server_url, slash)) {
        url = mla_string_concat(config->server_url, procedure_name);
    } else {
        url = mla_string_concat(config->server_url, slash, procedure_name);
    }

    mla_http_request_t request = mla_http_post_request(url);
    mla_http_headers_add(request.headers, mla_string_const("Content-Type"), __mla_http_rpc_content_type_to_string(config->content_type));

    if (!mla_pointer_is_null(input_data)) {

        // If its an short input we can optimize by writing it directly
        mla_memory_stream_t temp_stream = mla_memory_stream(mla_global_config_rpc_stream_small_buffer_size, false);

        if (__mla_http_rpc_request_content_write(config->content_type, temp_stream.output, input_data, input_definition.write_function)) {
            // If serialization was successful we can use the memory stream as content
            mla_http_headers_add(request.headers, mla_string_const("Content-Length"), mla_string_from_int32(static_cast<mla_int32_t>(mla_memory_stream_get_size(temp_stream))));
            mla_memory_stream_set_position(temp_stream, 0);
            request.content = temp_stream.input;

        } else {

            mla_pointer_t body_config_ptr = mla_malloc_struct(mla_rpc_http_request_body_config);

            mla_rpc_http_request_body_config* body_config = mla_pointer_get_data<mla_rpc_http_request_body_config>(body_config_ptr);

            if (body_config == nullptr) {
                return false;
            }

            body_config->input_data = input_data;
            body_config->input_definition = input_definition;
            body_config->content_type = config->content_type;

            // We dont know the content length so send as chunked
            mla_http_headers_add(request.headers, mla_string_const("Transfer-Encoding"), mla_string_const("chunked"));

            mla_user_data_t writer_user_data = mla_user_data_empty();
            mla_user_data_set_pointer(writer_user_data, mla_http_rpc_request_content_writer_body_config_user_data_name, body_config_ptr);

            request.contentWriter = mla_http_request_content_writer(writer_user_data, __mla_http_rpc_request_content_writer);

        }

    }

    mla_http_client_response_t client_response = mla_http_client_send_request(request);

    if (client_response.status != MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
        mla_error(mla_string_concat("HTTP RPC request failed for procedure ", procedure_name, ": ", client_response.errorMessage));
        return false;
    }

    if (client_response.response.statusCode != mla_http_status_ok) {
        mla_error(mla_string_concat("HTTP RPC request returned error status for procedure ", procedure_name, ": ", mla_string_from_int32(client_response.response.statusCode), " -> ", client_response.response.statusMessage));
        return false;
    }

    if (!mla_pointer_is_null(output_data)) {
        mla_deserializer_t deserializer = mla_deserializer_invalid();

        if (config->content_type == mla_http_rpc_content_type_json) {
            deserializer = mla_json_deserializer(client_response.response.content);
        } else if (config->content_type == mla_http_rpc_content_type_binary) {
            deserializer = mla_binary_deserializer(client_response.response.content);
        } else {
            mla_error(mla_string_concat("Unsupported Content-Type in HTTP RPC response for procedure ", procedure_name));
            return false;
        }

        // Start reading
        deserializer.read_next(deserializer);
        if (!mla_deserializer_read_struct_read_function(deserializer, output_data, output_definition.read_function)) {
            return false; // Serialization failed
        }

    }

    return true;
}

mla_rpc_remote_endpoint_t mla_rpc_http_register_endpoint(const mla_string_t& server_url, mla_http_rpc_content_type content_type) {

    if (content_type == mla_http_rpc_content_type_unknown) {
        mla_error(mla_string_const("Cannot register HTTP RPC endpoint with unknown content type."));
        return mla_rpc_remote_endpoint_invalid();
    }

    mla_pointer_t config_ptr = mla_malloc_struct(mla_rpc_http_client_config);

    mla_rpc_http_client_config* config = mla_pointer_get_data<mla_rpc_http_client_config>(config_ptr);

    if (config == nullptr) {
        return mla_rpc_remote_endpoint_invalid();
    }

    mla_memset(config, 0, sizeof(mla_rpc_http_client_config));
    config->server_url = server_url;
    config->content_type = content_type;

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer(user_data, mla_rpc_http_client_config_user_data_name, config_ptr);

    mla_rpc_remote_endpoint_t endpoint = mla_rpc_remote_endpoint_all(__mla_rpc_http_execute, user_data);

    if (!mla_rpc_register_remote_endpoint(endpoint))
        return mla_rpc_remote_endpoint_invalid();

    return endpoint;
}