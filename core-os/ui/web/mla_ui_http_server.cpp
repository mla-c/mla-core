//
// Created by christian on 12/5/25.
//

#include "mla_ui_http_server.h"

#include "mla_ui_web_embedded.h"
#include "mla_ui_web_remote_surface.h"
#include "../../task/mla_task_manager.h"
#include "../../serializer/mla_json_serializer.h"

#define mla_ui_web_socket_surface_path_prefix "/ws/surface/"

struct mla_ui_rpc_surface_info_t {
    mla_string_t displayName;
    mla_string_t surfaceName;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {

        const mla_ui_rpc_surface_info_t* surfaceInfo = static_cast<const mla_ui_rpc_surface_info_t*>(obj);
        mla_serializer_write_string(serializer, mla_string_const("displayName"), surfaceInfo->displayName);
        mla_serializer_write_string(serializer, mla_string_const("surfaceName"), surfaceInfo->surfaceName);
        return true;

    }

};

mla_ui_rpc_surface_info_t mla_ui_rpc_surface_info_empty() {
    return {
        mla_string_empty(),
        mla_string_empty()
    };
}

struct mla_ui_rpc_surface_info_t_initializer {
    static mla_ui_rpc_surface_info_t init() {
        return mla_ui_rpc_surface_info_empty();
    }
};

struct mla_ui_rpc_surface_infos_t {
    mla_array_list_t<mla_ui_rpc_surface_info_t, mla_ui_rpc_surface_info_t_initializer> surfaces;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {

        const mla_ui_rpc_surface_infos_t* surfaceInfos = static_cast<const mla_ui_rpc_surface_infos_t*>(obj);
        mla_serializer_write_list<mla_ui_rpc_surface_info_t>(serializer, mla_string_const("surfaces"), surfaceInfos->surfaces, mla_ui_rpc_surface_info_t::serialize);
        return true;

    }
};

mla_ui_rpc_surface_infos_t mla_ui_rpc_surface_infos_empty() {
    return {
        mla_array_list_empty<mla_ui_rpc_surface_info_t, mla_ui_rpc_surface_info_t_initializer>()
    };
}

#define mla_ui_http_server_web_surface_data_user_data_name "wuisd"

struct mla_ui_http_server_web_surface_data_t {
    mla_string_t displayName;
    mla_string_t surfaceName;
    mla_ui_control_surface_process_task_t processTask;
    mla_http_server_t* server;
};

struct mla_ui_http_server_web_surface_data_initializer {
    static mla_ui_http_server_web_surface_data_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            nullptr,
            nullptr
        };
    }
};


mla_bool_t __mla_ui_http_server_handler(mla_http_server_t& http_server, const mla_http_request_t &request, mla_http_response_t &response) {

    // Remove "/ui/" prefix
    mla_string_t resource_name = mla_string_substr(request.url, 4, mla_string_length(request.url) - 4);

    if (mla_string_is_empty(resource_name)) {
        resource_name = mla_string_const("index.html");
    }

    if (mla_string_equals_const(resource_name, "index.html")) {
        // Serve index.html
        response.statusCode = mla_http_status_ok;
        response.statusMessage = mla_string_const("OK");
        // Add headers
        mla_http_headers_add(response.headers, mla_string_const("Content-Type"), mla_string_const("text/html; charset=UTF-8"));
        mla_http_headers_add(response.headers, mla_string_const("Content-Length"), mla_string_from_uint32(mla_ui_web_embedded_index_html_len));

        mla_string_t content_encoding = mla_string(mla_ui_web_embedded_index_html_content_encoding);

        if (!mla_string_is_empty(content_encoding)) {
            mla_http_headers_add(response.headers, mla_string_const("Content-Encoding"), content_encoding);
        }

        // Set content
        response.content = mla_stream_input_from_buffer(mla_ui_web_embedded_index_html, mla_ui_web_embedded_index_html_len);
        return true;
    } else if (mla_string_equals_const(resource_name, "surfaces.json")) {

        response.statusCode = mla_http_status_ok;
        response.statusMessage = mla_string_const("OK");

        // Gather surface infos
        mla_ui_rpc_surface_infos_t surfaceInfos = mla_ui_rpc_surface_infos_empty();
        mla_array_list_t<mla_http_server_websocket_handler_item_t, mla_http_server_websocket_handler_item_initializer> surfaces = mla_http_server_get_websocket_handler_for_path(http_server, mla_string_const(mla_ui_web_socket_surface_path_prefix));

        for (mla_size_t i = 0; i < mla_array_list_size(surfaces); i++) {
            mla_http_server_websocket_handler_item_t& handlerItem = mla_array_list_get_unsafe(surfaces, i);
            mla_ui_http_server_web_surface_data_t* taskData = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(handlerItem.userdata, mla_ui_http_server_web_surface_data_user_data_name);

            if (taskData != nullptr) {
                mla_ui_rpc_surface_info_t surfaceInfo = {
                    taskData->displayName,
                    taskData->surfaceName
                };
                mla_array_list_add(surfaceInfos.surfaces, surfaceInfo);
            }
        }


        mla_http_headers_add(response.headers, mla_string_const("Content-Type"), mla_string_const("application/json"));
        mla_http_headers_add(response.headers, mla_string_const("Access-Control-Allow-Origin"), mla_string_const("*"));

        // Calculate content length
        mla_memory_stream_t memory_stream = mla_memory_stream(mla_stream_fast_read_buffer_size);
        mla_serializer_t serializer = mla_json_serializer(memory_stream.output);
        mla_serializer_write_data_struct<>(serializer, surfaceInfos);

        mla_http_headers_add(response.headers, mla_string_const("Content-Length"), mla_string_from_uint32(mla_memory_stream_get_size(memory_stream)));

        mla_memory_stream_set_position(memory_stream, 0);

        // Set content
        response.content = memory_stream.input;
        return true;


    } else {
        // Not found
        response.statusCode = mla_http_status_not_found;
        response.statusMessage = mla_string_const("Not Found");
        return true;
    }
}

mla_bool_t mla_ui_http_server_initialize(mla_http_server_t &server) {

    mla_http_server_handler_item_t handler = mla_http_server_handler_starts_with(mla_http_method_get, mla_string_const("/ui/"), __mla_ui_http_server_handler);
    return mla_http_server_register_handler(server, handler);

}

struct mla_ui_http_server_web_surface_data_client_t {
    mla_string_t messageBuffer;
    mla_ui_control_surface_t remoteSurface;
    mla_ui_web_remote_surface_t webRemoteSurface;
};

mla_ui_http_server_web_surface_data_client_t mla_ui_http_server_web_surface_data_client_empty() {
    return {
        mla_string_empty(),
        mla_ui_control_surface_empty(),
        mla_ui_web_remote_surface_invalid()
    };
}

struct mla_ui_http_server_web_surface_data_client_initializer {
    static mla_ui_http_server_web_surface_data_client_t init() {
        return mla_ui_http_server_web_surface_data_client_empty();
    }
};

#define mla_ui_http_server_web_surface_data_client_list_user_data_name "wuicld"

#define mla_web_surface_max_buffer_items 25

mla_bool_t __mla_ui_http_server_web_surface_text_handler(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_bool_t isFinalFragment) {

    mla_ui_http_server_web_surface_data_client_t* data_client = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_client_t>(connection.userdata, mla_ui_http_server_web_surface_data_client_list_user_data_name);

    if (data_client == nullptr) {
        mla_ui_http_server_web_surface_data_t* task_data = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(connection.userdata, mla_ui_http_server_web_surface_data_user_data_name);

        if (task_data != nullptr) {
            mla_warning(mla_string_concat("Failed to find client data for connection ", connection.id, " on surface ", task_data->surfaceName));
        } else {
            mla_warning(mla_string_concat("Failed to find client data for connection ", connection.id, " and task data on surface "));
        }
        return false;
    }

    data_client->messageBuffer = mla_string_concat(data_client->messageBuffer, message);

    if (isFinalFragment) {



        if (data_client->webRemoteSurface.handle_client_text_message != nullptr) {
            mla_ui_web_remote_surface_message_result_t result = data_client->webRemoteSurface.handle_client_text_message(data_client->webRemoteSurface, data_client->messageBuffer);
            if (result.handled) {
                mla_array_list_add_all(data_client->remoteSurface.drawing.unprocessedInputEvents, result.inputEvents);

                mla_size_t droppedEventsCount = 0;

                // Limit the number of complete messages stored in the buffer to prevent memory issues
                if (mla_array_list_size(data_client->remoteSurface.drawing.unprocessedInputEvents) >= mla_web_surface_max_buffer_items) {
                    mla_array_list_remove(data_client->remoteSurface.drawing.unprocessedInputEvents, 0);
                    droppedEventsCount++;
                }

                if (droppedEventsCount > 0) {
                    mla_warning(mla_string_concat("Dropped ", mla_string_from_uint32(droppedEventsCount), " input events from connection ", connection.id, " due to buffer limit."));
                }
            } else {
                mla_warning(mla_string_concat("Received unhandled message from webRemoteSurface on connection ", connection.id, " on surface : ", data_client->messageBuffer));
            }
        }

        data_client->messageBuffer = mla_string_empty();
    }

    return true;
}

mla_string_t __mla_ui_http_server_web_surface_path_from_surface_name(const mla_string_t& surface_name) {
    return mla_string_concat(mla_ui_web_socket_surface_path_prefix, surface_name);
}

mla_bool_t __mla_ui_http_server_web_surface_path_checker(const mla_user_data_t &userdata, const mla_string_t &url, mla_http_request_handler_checker_compare_mode_t compare_mode) {

    mla_ui_http_server_web_surface_data_t* task_data = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(userdata, mla_ui_http_server_web_surface_data_user_data_name);

    if (task_data == nullptr)
        return false;

    mla_string_t final_url = __mla_ui_http_server_web_surface_path_from_surface_name(task_data->surfaceName);

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PERFECT_MATCH) {
        return mla_string_equals(url, final_url);
    }

    if (compare_mode == MLA_HTTP_REQUEST_HANDLER_CHECKER_COMPARE_MODE_PREFIX) {
        return mla_string_starts_with(final_url, url);
    }

    return false;
}

mla_buffer_cleanup_mode __mla_ui_http_server_web_surface_cleanup(mla_pointer_t data, const mla_dynamic_data_t& userData) {
    (void)userData;
    mla_ui_http_server_web_surface_data_t* taskData = reinterpret_cast<mla_ui_http_server_web_surface_data_t*>(data);

    if (taskData == nullptr) {
        return CLEAN_UP_SKIP; // Nothing to clean up
    }

    mla_task_manager_abort_task(taskData->surfaceName);
    *taskData = mla_ui_http_server_web_surface_data_initializer::init(); // Reset the structure to release resources
    return CLEAN_UP_NEEDED;
}


#define mla_web_surface_max_connected_clients 5

mla_bool_t __mla_ui_http_server_web_surface_open(mla_http_server_websocket_connection_t& connection) {

    mla_ui_http_server_web_surface_data_t* task_data = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(connection.userdata, mla_ui_http_server_web_surface_data_user_data_name);

    if (task_data == nullptr) {
        return false;
    }

    mla_size_t clients_count = mla_http_server_get_websocket_connection_count(*connection.server, task_data->surfaceName);

    if (clients_count >= mla_web_surface_max_connected_clients) {
        mla_warning(mla_string_concat("Maximum number of connected clients reached for surface ", task_data->surfaceName, ". Connection ", connection.id, " will be closed."));
        return false; // Reject new connection
    }

    mla_ui_web_remote_surface_t surface =  mla_ui_web_remote_surface_create(connection);

    if (!mla_ui_surface_is_valid(surface.surface)) {
        mla_warning(mla_string_concat("Failed to create remote surface for connection ", connection.id, " on surface ", task_data->surfaceName));
        return false; // Failed to create remote surface
    }

    mla_ui_control_surface_t remoteSurface = mla_ui_control_surface_create(surface.surface, task_data->processTask);

    mla_ui_http_server_web_surface_data_client_t* data_client = reinterpret_cast<mla_ui_http_server_web_surface_data_client_t*>(mla_malloc(sizeof(mla_ui_http_server_web_surface_data_client_t)));

    if (data_client == nullptr) {
        mla_warning(mla_string_concat("Failed to allocate memory for client data for connection ", connection.id, " on surface ", task_data->surfaceName));
        return false; // Allocation failed
    }

    mla_memset(data_client, 0, sizeof(mla_ui_http_server_web_surface_data_client_t));

    data_client->messageBuffer = mla_string_empty();
    data_client->remoteSurface = remoteSurface;
    data_client->webRemoteSurface = surface;

    if (!mla_user_data_set_pointer_with_ownership<mla_ui_http_server_web_surface_data_client_t, mla_ui_http_server_web_surface_data_client_initializer>(connection.userdata, mla_ui_http_server_web_surface_data_client_list_user_data_name, data_client)) {
        mla_free(data_client);
        mla_warning(mla_string_concat("Failed to set client data for connection ", connection.id, " on surface ", task_data->surfaceName));
        return false; // Failed to set client data
    }

    return true;
}

mla_task_process_result_state __mla_ui_http_server_web_surface_render_and_draw_task(mla_user_data_t& userData) {

    mla_ui_http_server_web_surface_data_t* taskData = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(userData, mla_ui_http_server_web_surface_data_user_data_name);

    if (taskData == nullptr) {
        return TASK_PROCESS_RESULT_DONE;
    }

    if (taskData->server == nullptr) {
        return TASK_PROCESS_RESULT_DONE;
    }

    mla_http_server_t& http_server = *taskData->server;

    if (!mla_http_server_running(http_server)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    mla_string_t final_url = __mla_ui_http_server_web_surface_path_from_surface_name(taskData->surfaceName);
    mla_array_list_t<mla_http_server_websocket_connection_t, mla_http_server_websocket_connection_initializer> connections = mla_http_server_find_websocket_connections(http_server, final_url);

    // Process each connected client
    for (mla_size_t i = 0; i < mla_array_list_size(connections); i++) {

        mla_http_server_websocket_connection_t& connection = mla_array_list_get_unsafe(connections, i);

        if (!mla_http_server_is_websocket_connection_open(connection)) {
            continue; // Skip closed connections
        }

        mla_ui_http_server_web_surface_data_client_t* clientData = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_client_t>(connection.userdata, mla_ui_http_server_web_surface_data_client_list_user_data_name);

        if (clientData == nullptr) {
            mla_warning(mla_string_concat("Failed to find client data for connection ", connection.id, " on surface ", taskData->surfaceName));
            continue; // Skip if we can't find client data
        }

        mla_ui_control_surface_execute_render_and_draw(clientData->remoteSurface);
    }

    return TASK_PROCESS_RESULT_CONTINUE;
}


mla_bool_t mla_ui_http_server_add_web_surface(mla_http_server_t& http_server, const mla_string_t& display_name, const mla_string_t& surface_name, mla_ui_control_surface_process_task_t processTask) {

    mla_ui_http_server_web_surface_data_t* taskData = reinterpret_cast<mla_ui_http_server_web_surface_data_t*>(mla_malloc(sizeof(mla_ui_http_server_web_surface_data_t)));

    if (taskData == nullptr) {
        return false; // Allocation failed
    }

    mla_memset(taskData, 0, sizeof(mla_ui_http_server_web_surface_data_t));
    taskData->displayName = display_name;
    taskData->surfaceName = surface_name;
    taskData->processTask = processTask;
    taskData->server = &http_server;

    mla_user_data_t ui_http_user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership_ex(ui_http_user_data, mla_ui_http_server_web_surface_data_user_data_name, taskData, __mla_ui_http_server_web_surface_cleanup, true);

    mla_http_server_websocket_handler_item_t handler = mla_http_server_websocket_handler(ui_http_user_data,
            __mla_ui_http_server_web_surface_path_checker,
            __mla_ui_http_server_web_surface_text_handler,
            nullptr);

    handler.open_executor = __mla_ui_http_server_web_surface_open;

    // Create rendering task
    mla_task_t render_draw_task = mla_task_repeating(
            mla_string_concat(mla_string_const("web_ui_sf_"), taskData->surfaceName),
            __mla_ui_http_server_web_surface_render_and_draw_task,
            ui_http_user_data
        );

    if (!mla_task_manager_register_task(render_draw_task)) {
        mla_error(mla_string_concat("Failed to register rendering task for web surface ", surface_name));
        return false;
    }

    if (!mla_http_server_register_websocket_handler(http_server, handler))
        return false;

    return true;
}