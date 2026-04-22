//
// Created by christian on 11/18/25.
//

#include "mla_rpc_http_server.h"
#include "mla_rpc.h"
#include "../url/mla_url.h"
#include "../serializer/mla_serializer.h"
#include "../serializer/mla_json_serializer.h"
#include "../serializer/mla_binary_serializer.h"
#include "mla_rpc_http_data_types.h"
#include "../http/mla_http_chunked_stream.h"

#ifndef mla_rpc_http_server_use_deflate_compression
#define mla_rpc_http_server_use_deflate_compression mla_global_feature_flag_http_server_use_deflate_compression
#endif

mla_user_data_id_init(mla_rpc_http_server_writer_output_buffer_user_data)

mla_http_rpc_content_type __mla_rpc_http_server_get_content_type(const mla_http_request_t& request) {

    mla_string_t content_type = mla_http_headers_get_value(request.headers, mla_string_const("Content-Type"));

    if (mla_string_equals(content_type, mla_string_const("application/json"))) {
        return mla_http_rpc_content_type_json;
    } else if (mla_string_equals(content_type, mla_string_const("application/octet-stream"))) {
        return mla_http_rpc_content_type_binary;
    }

    return mla_http_rpc_content_type_unknown;
}

struct mla_rpc_http_server_handler_content_writer_header_t {
    mla_http_rpc_content_type contentType;
    mla_serialize_definition_write_function_t write_function;
    mla_bool_t support_deflate_compression;
};

mla_bool_t __mla_rpc_http_server_support_deflate_compression(const mla_rpc_http_server_handler_content_writer_header_t& header) {

#if (mla_rpc_http_server_use_deflate_compression == 1)
    return header.support_deflate_compression;
#else
    (void)header;
    return false;
#endif
}

mla_bool_t __mla_rpc_http_server_handler_content_write(mla_http_rpc_content_type contentType, const mla_stream_output_t &outputStream, const mla_platform_pointer_t outputData, const mla_serialize_definition_write_function_t &write_function) {

    mla_serializer_t serializer = mla_serializer_invalid();

    // Set Content-Type header
    if (contentType == mla_http_rpc_content_type_json) {
        serializer = mla_json_serializer(outputStream);
    } else if (contentType == mla_http_rpc_content_type_binary) {
        serializer = mla_binary_serializer(outputStream);
    } else {
        mla_error("Unsupported Content-Type in output writer");
        return false;
    }

    if (!mla_serializer_write_data_struct(serializer, outputData, write_function)) {
        return false;
    }

    return true;
}

mla_bool_t __mla_rpc_http_server_handler_content_writer(const mla_http_response_content_writer_t& writer, const mla_stream_output_t &outputStream) {

    mla_pointer_t buffer_ptr = mla_user_data_get_pointer(writer.userData, mla_rpc_http_server_writer_output_buffer_user_data);
    mla_platform_pointer_t buffer = mla_pointer_get_platform_pointer(buffer_ptr);

    if (buffer == nullptr) {
        return false;
    }

    mla_rpc_http_server_handler_content_writer_header_t* header = reinterpret_cast<mla_rpc_http_server_handler_content_writer_header_t*>(buffer);

    if (header->write_function == nullptr)
        return false;

    // Store content type and write function at the beginning of output buffer
    mla_platform_pointer_t outputData = reinterpret_cast<mla_uint8_t*>(buffer) + sizeof(mla_rpc_http_server_handler_content_writer_header_t);

    mla_http_chunked_stream_output_t chunked_output = mla_http_chunked_stream_output_invalid();

    if (__mla_rpc_http_server_support_deflate_compression(*header)) {
        chunked_output = mla_http_chunked_stream_output_deflate(outputStream);
    } else {
        chunked_output = mla_http_chunked_stream_output(outputStream);
    }

    mla_bool_t result = __mla_rpc_http_server_handler_content_write(header->contentType, chunked_output.output, outputData, header->write_function);

    // Finalize chunked output
    if (!mla_http_chunked_stream_output_finished(chunked_output)) {
        return false;
    }

    return result;
}

mla_bool_t __mla_rpc_http_server_handler_options(mla_http_server_t& http_server, const mla_http_request_t &request, mla_http_response_t &response) {

    (void)request;
    (void)http_server;

    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    mla_http_headers_add(response.headers, mla_string_const("Allow"), mla_string_const("POST, OPTIONS"));

    // Add CORS headers
    mla_http_headers_add(response.headers, mla_string_const("Access-Control-Allow-Origin"), mla_string_const("*"));
    mla_http_headers_add(response.headers, mla_string_const("Access-Control-Allow-Methods"), mla_string_const("POST, OPTIONS"));
    mla_http_headers_add(response.headers, mla_string_const("Access-Control-Allow-Headers"), mla_string_const("Content-Type"));

    return true;

}

mla_bool_t __mla_rpc_http_server_handler(mla_http_server_t& http_server, const mla_http_request_t &request, mla_http_response_t &response) {

    (void)http_server;

    // Remove "/rpc/" prefix
    mla_string_t procedure_name = mla_string_substr(request.url, 5, mla_string_length(request.url) - 5);

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

    mla_http_rpc_content_type contentType = __mla_rpc_http_server_get_content_type(request);

    if (contentType == mla_http_rpc_content_type_unknown) {

        if (procedure.inputDefinition.data_size > 0) {
            // We expect input data but content type is unknown
            response.statusCode = 415; // Unsupported Media Type
            response.statusMessage = mla_string_const("Unsupported Media Type");
            mla_error(mla_string_concat("Unsupported Content-Type for procedure ", procedure_name));
            return false;
        }

        contentType = mla_http_rpc_content_type_json; // Default to JSON for no input


    } else if (contentType == mla_http_rpc_content_type_json) {
        deserializer = mla_json_deserializer(request.content);
    } else if (contentType == mla_http_rpc_content_type_binary) {
        deserializer = mla_binary_deserializer(request.content);
    }

    // Prepare Input data
    mla_platform_pointer_t input = nullptr;

    if (procedure.inputDefinition.data_size > 0) {
        input = mla_platform_malloc(procedure.inputDefinition.data_size);

        if (input == nullptr) {
            return false;
        }

        mla_memset(input, 0, procedure.inputDefinition.data_size);
    }

    // Prepare Output data
    mla_platform_pointer_t output = nullptr;

    if (procedure.outputDefinition.data_size > 0) {

        // Calculate output size including content type and write function
        mla_size_t output_size = sizeof(mla_rpc_http_server_handler_content_writer_header_t) + procedure.outputDefinition.data_size;

        output = mla_platform_malloc(output_size);

        if (output == nullptr) {

            if (input != nullptr) {
                mla_platform_free(input);
            }
            return false;
        }

        mla_memset(output, 0, output_size);

        mla_rpc_http_server_handler_content_writer_header_t* header = reinterpret_cast<mla_rpc_http_server_handler_content_writer_header_t*>(output);
        header->contentType = contentType;
        header->write_function = procedure.outputDefinition.write_function;
        header->support_deflate_compression = mla_http_headers_has_header_value(request.headers, mla_string_const("Accept-Encoding"), mla_string_const("deflate"), mla_string_const(","));
    }

    if (input != nullptr && procedure.inputDefinition.read_function != nullptr) {

        // Start reading
        deserializer.read_next(deserializer);
        if (!mla_deserializer_read_struct_read_function(deserializer, input, procedure.inputDefinition.read_function)) {
            response.statusCode = mla_http_status_bad_request;
            mla_error(mla_string_concat("Failed to deserialize input for procedure ", procedure_name));
            mla_platform_free(output);
            mla_platform_free(input);
            return false;
        }
    }

    // Store content type and write function at the beginning of output buffer
    mla_platform_pointer_t outputData = nullptr;

    if (output != nullptr) {
        outputData = reinterpret_cast<mla_uint8_t*>(output) + sizeof(mla_rpc_http_server_handler_content_writer_header_t);
    }

    if (procedure.execute(input, outputData)) {
        mla_platform_free(input);

        response.statusCode = mla_http_status_ok;
        response.statusMessage = mla_string_const("Success");

        // Add CORS header for actual request
        mla_http_headers_add(response.headers, mla_string_const("Access-Control-Allow-Origin"), mla_string_const("*"));

        if (output != nullptr) {

            // Set Content-Type header
            if (contentType == mla_http_rpc_content_type_json) {
                mla_http_headers_add(response.headers, mla_string_const("Content-Type"), mla_string_const("application/json"));
            } else if (contentType == mla_http_rpc_content_type_binary) {
                mla_http_headers_add(response.headers, mla_string_const("Content-Type"), mla_string_const("application/octet-stream"));
            } else {
                mla_error(mla_string_concat("Unsupported Content-Type for procedure ", procedure_name));
                mla_platform_free(output);
                return false;
            }

            // If its an short input we can optimize by writing it directly
            mla_memory_stream_t temp_stream = mla_memory_stream(mla_global_config_rpc_stream_small_buffer_size, false);

            if (__mla_rpc_http_server_handler_content_write(contentType, temp_stream.output, outputData, procedure.outputDefinition.write_function)) {

                mla_size_t content_size = mla_memory_stream_get_size(temp_stream);

                mla_memory_stream_set_position(temp_stream, 0);
                mla_rpc_http_server_handler_content_writer_header_t* header = reinterpret_cast<mla_rpc_http_server_handler_content_writer_header_t*>(output);

                if (__mla_rpc_http_server_support_deflate_compression(*header) && content_size > mla_global_config_stream_output_deflate_min_compression_data_size) {

                    mla_memory_stream_t temp_compressed_stream = mla_memory_stream(mla_global_config_rpc_stream_small_buffer_size, true);
                    mla_stream_output_t mla_deflate_stream = mla_stream_output_deflate_compress_wrapper(temp_compressed_stream.output, mla_deflate_mode_zlib);
                    mla_stream_copy(temp_stream.input, mla_deflate_stream);
                    mla_stream_output_deflate_finish(mla_deflate_stream);

                    mla_memory_stream_set_position(temp_compressed_stream, 0);

                    mla_http_headers_add(response.headers, mla_string_const("Content-Encoding"), mla_string_const("deflate"));
                    mla_http_headers_add(response.headers, mla_string_const("Content-Length"), mla_string_from_size(mla_memory_stream_get_size(temp_compressed_stream)));

                    response.content = temp_compressed_stream.input;


                } else {
                    // Content is small or client does not support deflate compression, we can write it directly with known content length
                    mla_http_headers_add(response.headers, mla_string_const("Content-Length"), mla_string_from_size(content_size));
                    response.content = temp_stream.input;
                }



                mla_platform_free(output);
            } else {

                mla_rpc_http_server_handler_content_writer_header_t* header = reinterpret_cast<mla_rpc_http_server_handler_content_writer_header_t*>(output);

                if (__mla_rpc_http_server_support_deflate_compression(*header)) {
                    mla_http_headers_add(response.headers, mla_string_const("Content-Encoding"), mla_string_const("deflate"));
                }

                mla_http_headers_add(response.headers, mla_string_const("Transfer-Encoding"), mla_string_const("chunked"));

                mla_user_data_t writer_user_data = mla_user_data_empty();
                mla_user_data_set_pointer_with_ownership_ex(writer_user_data, mla_rpc_http_server_writer_output_buffer_user_data, output, nullptr, false);

                response.contentWriter = mla_http_response_content_writer(writer_user_data, __mla_rpc_http_server_handler_content_writer);
            }
        }

    } else {
        mla_platform_free(input);
        mla_platform_free(output);
        response.statusCode = mla_http_status_internal_server_error;
    }

    return true;
}

mla_bool_t mla_rpc_http_server_initialize(mla_http_server_t &server) {

    mla_http_server_handler_item_t handler = mla_http_server_handler_starts_with(mla_http_method_post, mla_string_const("/rpc/"), __mla_rpc_http_server_handler);
    mla_bool_t result1 = mla_http_server_register_handler(server, handler);
    mla_http_server_handler_item_t handlerOptions = mla_http_server_handler_starts_with(mla_http_method_options, mla_string_const("/rpc/"), __mla_rpc_http_server_handler_options);
    return result1 && mla_http_server_register_handler(server, handlerOptions);
}