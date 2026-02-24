//
// Created by chris on 2/23/2026.
//

#include "mla_ui_web_remote_surface.h"

#include "../../serializer/mla_json_serializer.h"

struct mla_ui_web_remote_surface_draw_data_t {
    const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
        const auto* in = static_cast<const mla_ui_web_remote_surface_draw_data_t*>(obj);
        mla_serializer_write_list_struct(serializer, mla_string_const("drawCommands"), in->drawCommands, mla_ui_surface_draw_command_t);
        return true;
    }

};

mla_ui_surface_size_t __mla_ui_web_remote_surface_get_size(const mla_ui_surface_t& surface) {
    (void)surface;
    return {
        1920,
        1080
    };
}

mla_bool_t __mla_ui_web_remote_surface_set_size(const mla_ui_surface_t& surface, mla_ui_surface_size_t size) {
    (void)surface;
    (void)size;
    // not possible
    return false;
}

#define mla_ui_web_remote_surface_draw_commands_message_user_data_name "drdata"

mla_bool_t ___mla_ui_web_remote_surface_render_draw_commands_text_message_generator(mla_stream_output_t& output, mla_user_data_t& user_data) {

    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>* drawCommands = mla_user_data_get_pointer<mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>>(user_data, mla_ui_web_remote_surface_draw_commands_message_user_data_name);

    if (drawCommands == nullptr) {
        return false; // No draw commands to serialize
    }

    mla_serializer_t serializer = mla_json_serializer(output);

    mla_ui_web_remote_surface_draw_data_t drawData = {
        *drawCommands
    };

    return mla_serializer_write_data_struct(serializer, drawData);

}

mla_bool_t __mla_ui_web_remote_surface_render_draw_commands(const mla_ui_surface_t& surface, const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>& eventsSinceLastFame) {

    (void)eventsSinceLastFame;

    mla_http_server_websocket_connection_t* connection = reinterpret_cast<mla_http_server_websocket_connection_t*>(surface.resource);

    if (connection == nullptr) {
        return false; // Can't render without a valid connection
    }

    if (!mla_http_server_is_websocket_connection_open(*connection)){
        return false; // Can't render if the connection is closed
    }


    mla_user_data_t messageData = mla_user_data_empty();
    mla_user_data_set_pointer_without_ownership(messageData, mla_ui_web_remote_surface_draw_commands_message_user_data_name, &drawCommands);

    // Serialize the commands to JSON
    return mla_http_server_send_websocket_text_message(*connection, messageData, ___mla_ui_web_remote_surface_render_draw_commands_text_message_generator);

}

mla_ui_surface_draw_size_t __mla_ui_web_remote_surface_render_calc_text_size(const mla_ui_surface_t &surface, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {
    (void)surface;
    (void)font_type;
    (void)text;

    return {
        100, // width
        20   // height
    };
}

mla_ui_surface_input_states_t __mla_ui_web_remote_surface_get_input_states(const mla_ui_surface_t &surface) {
    (void)surface;
    // Not implemented yet, but we can return an empty input state for now
    return mla_ui_surface_input_states_empty();
}

mla_ui_surface_t mla_ui_web_remote_surface_create(const mla_http_server_websocket_connection_t& connection) {

    return  {
        const_cast<mla_http_server_websocket_connection_t*>(&connection),
        mla_buffer_reference_noOwner(),
        __mla_ui_web_remote_surface_get_size,
        __mla_ui_web_remote_surface_set_size,
        __mla_ui_web_remote_surface_render_draw_commands,
        __mla_ui_web_remote_surface_render_calc_text_size,
        __mla_ui_web_remote_surface_get_input_states
    };

}