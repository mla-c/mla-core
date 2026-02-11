//
// Created by christian on 12/5/25.
//

#include "mla_ui_http_server.h"

#include <type_traits>

#include "mla_ui_web_embedded.h"
#include "mla_ui_web_remote_surface.h"
#include "../../task/mla_task_manager.h"

mla_bool_t __mla_ui_http_server_handler(const mla_http_request_t &request, mla_http_response_t &response) {

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
    mla_string_t connectionId;
    mla_string_t messageBuffer;
    mla_ui_control_surface_t remoteSurface;
};

mla_ui_http_server_web_surface_data_client_t mla_ui_http_server_web_surface_data_client_empty() {
    return {
        mla_string_empty(),
        mla_string_empty(),
        mla_ui_control_surface_empty()
    };
}

struct mla_ui_http_server_web_surface_data_client_initializer {
    static mla_ui_http_server_web_surface_data_client_t init() {
        return mla_ui_http_server_web_surface_data_client_empty();
    }
};


struct mla_ui_http_server_web_surface_data_t {
    mla_string_t surfaceName;
    mla_ui_control_surface_process_task_t processTask;
    mla_array_list_t<mla_ui_http_server_web_surface_data_client_t, mla_ui_http_server_web_surface_data_client_initializer> client_datas;
};

struct mla_ui_http_server_web_surface_data_initializer {
    static mla_ui_http_server_web_surface_data_t init() {
        return {
            mla_string_empty(),
            nullptr,
            mla_array_list_empty<mla_ui_http_server_web_surface_data_client_t, mla_ui_http_server_web_surface_data_client_initializer>()
        };
    }
};

#define mla_ui_http_server_web_surface_data_user_data_name "wuisd"

mla_ui_http_server_web_surface_data_client_t* mla_ui_http_server_web_surface_data_get_client_data(mla_ui_http_server_web_surface_data_t* surfaceData, const mla_string_t& connectionId) {

    for (mla_size_t i = 0; i < mla_array_list_size(surfaceData->client_datas); i++) {
        mla_ui_http_server_web_surface_data_client_t& clientData = mla_array_list_get_unsafe(surfaceData->client_datas, i);

        if (mla_string_equals(clientData.connectionId, connectionId)) {
            return &clientData; // Return pointer to the existing client data
        }
    }

    return nullptr; // Failed to add new client data
}

#define mla_web_surface_max_buffer_items 25

mla_bool_t __mla_ui_http_server_web_surface_text_handler(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_bool_t isFinalFragment) {

    mla_ui_http_server_web_surface_data_t* task_data = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(connection.userdata, mla_ui_http_server_web_surface_data_user_data_name);

    if (task_data == nullptr)
        return false;

    mla_ui_http_server_web_surface_data_client_t* data_client = mla_ui_http_server_web_surface_data_get_client_data(task_data, connection.id);

    if (data_client == nullptr) {
        mla_warning(mla_string_concat("Failed to find client data for connection ", connection.id, " on surface ", task_data->surfaceName));
        return false;
    }

    data_client->messageBuffer = mla_string_concat(data_client->messageBuffer, message);

    if (isFinalFragment) {

        // Limit the number of complete messages stored in the buffer to prevent memory issues
        if (mla_array_list_size(data_client->remoteSurface.drawing.unprocessedInputEvents) >= mla_web_surface_max_buffer_items) {
            mla_array_list_remove(data_client->remoteSurface.drawing.unprocessedInputEvents, 0);
        }

        // Process the complete message
        //mla_array_list_add(data_client->, data_client->messageBuffer);
        data_client->messageBuffer = mla_string_empty();
    }

    return true;
}

mla_bool_t __mla_ui_http_server_web_surface_path_checker(const mla_user_data_t &userdata, const mla_string_t &url) {

    mla_ui_http_server_web_surface_data_t* task_data = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(userdata, mla_ui_http_server_web_surface_data_user_data_name);

    if (task_data == nullptr)
        return false;

    return mla_string_equals(url, task_data->surfaceName);
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

    if (mla_array_list_size(task_data->client_datas) >= mla_web_surface_max_connected_clients) {
        mla_warning(mla_string_concat("Maximum number of connected clients reached for surface ", task_data->surfaceName, ". Connection ", connection.id, " will be closed."));
        return false; // Reject new connection
    }

    mla_ui_surface_t surface =  mla_ui_surface_invalid();//mla_ui_web_remote_surface_create(connection);

    if (!mla_ui_surface_is_valid(surface)) {
        mla_warning(mla_string_concat("Failed to create remote surface for connection ", connection.id, " on surface ", task_data->surfaceName));
        return false; // Failed to create remote surface
    }

    mla_ui_control_surface_t remoteSurface = mla_ui_control_surface_create(surface, task_data->processTask);

    mla_ui_http_server_web_surface_data_client_t newClientData = {
        connection.id,
        mla_string_empty(),
        remoteSurface
    };

    if (!mla_array_list_add(task_data->client_datas, newClientData)) {
        mla_warning(mla_string_concat("Failed to add new client data for connection ", connection.id, " on surface ", task_data->surfaceName));
        return false; // Failed to add new client data
    }

    return true;
}

void __mla_ui_http_server_web_surface_closed(const mla_http_server_websocket_connection_t& connection) {

    mla_ui_http_server_web_surface_data_t* task_data = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(connection.userdata, mla_ui_http_server_web_surface_data_user_data_name);

    if (task_data == nullptr) {
        return;
    }

    // Remove client data for the closed connection
    for (mla_size_t i = 0; i < mla_array_list_size(task_data->client_datas); i++) {
        mla_ui_http_server_web_surface_data_client_t& clientData = mla_array_list_get_unsafe(task_data->client_datas, i);

        if (mla_string_equals(clientData.connectionId, connection.id)) {
            mla_array_list_remove(task_data->client_datas, i);
            break;
        }

    }
}

mla_task_process_result_state __mla_ui_http_server_web_surface_render_and_draw_task(mla_user_data_t& userData) {

    mla_ui_http_server_web_surface_data_t* taskData = mla_user_data_get_pointer<mla_ui_http_server_web_surface_data_t>(userData, mla_ui_http_server_web_surface_data_user_data_name);

    if (taskData == nullptr) {
        return TASK_PROCESS_RESULT_DONE;
    }

    // Process each connected client
    for (mla_size_t i = 0; i < mla_array_list_size(taskData->client_datas); i++) {

        mla_ui_http_server_web_surface_data_client_t& clientData = mla_array_list_get_unsafe(taskData->client_datas, i);
        mla_ui_control_surface_execute_render_and_draw(clientData.remoteSurface);
    }

    return TASK_PROCESS_RESULT_CONTINUE;
}


mla_bool_t mla_ui_http_server_add_web_surface(mla_http_server_t& http_server, mla_string_t surface_name, mla_ui_control_surface_process_task_t processTask) {

    mla_ui_http_server_web_surface_data_t* taskData = reinterpret_cast<mla_ui_http_server_web_surface_data_t*>(mla_malloc(sizeof(mla_ui_http_server_web_surface_data_t)));

    if (taskData == nullptr) {
        return false; // Allocation failed
    }

    mla_memset(taskData, 0, sizeof(mla_ui_http_server_web_surface_data_t));
    taskData->surfaceName = mla_string_concat("/surface/", surface_name);
    taskData->processTask = processTask;
    taskData->client_datas = mla_array_list_empty<mla_ui_http_server_web_surface_data_client_t, mla_ui_http_server_web_surface_data_client_initializer>();

    mla_user_data_t ui_http_user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership_ex(ui_http_user_data, mla_ui_http_server_web_surface_data_user_data_name, taskData, __mla_ui_http_server_web_surface_cleanup, true);

    mla_http_server_websocket_handler_item_t handler = mla_http_server_websocket_handler(ui_http_user_data,
            __mla_ui_http_server_web_surface_path_checker,
            __mla_ui_http_server_web_surface_text_handler,
            nullptr);

    handler.open_executor = __mla_ui_http_server_web_surface_open;
    handler.close_executor = __mla_ui_http_server_web_surface_closed;

    // Create rendering task
    mla_task_t render_draw_task = mla_task_repeating(
            taskData->surfaceName,
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