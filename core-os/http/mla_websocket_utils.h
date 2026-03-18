//
// Created by chris on 11/15/2025.
//

#ifndef COREOS_MLA_WEBSOCKET_UTILS_H
#define COREOS_MLA_WEBSOCKET_UTILS_H

#include "../system/mla_stream.h"
#include "../network/mla_network.h"

// WebSocket close status codes
#define mla_websocket_close_normal 1000
#define mla_websocket_close_abnormal 1006

typedef mla_bool_t (mla_websocket_transport_message_generator_t)(mla_stream_output_t& output, mla_user_data_t& user_data);

mla_bool_t mla_websocket_transport_send_close_frame(mla_stream_output_t &output, mla_uint16_t status_code, const mla_string_t &reason, mla_bool_t mask_message);
mla_bool_t mla_websocket_transport_send_text_with_generator(mla_stream_output_t &output, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t mask_message, mla_bool_t use_deflate_compression);
mla_bool_t mla_websocket_transport_send_text_frame(mla_stream_output_t &output, const mla_string_t &message, mla_bool_t is_final, mla_bool_t mask_message, mla_bool_t use_deflate_compression);
mla_bool_t mla_websocket_transport_send_binary_with_generator(mla_stream_output_t &output, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t mask_message, mla_bool_t use_deflate_compression);
mla_bool_t mla_websocket_transport_send_binary_frame(mla_stream_output_t &output, const mla_bytes_t &message, mla_bool_t is_final, mla_bool_t mask_message, mla_bool_t use_deflate_compression);

enum mla_websocket_transport_message_receive_type_t: mla_uint8_t {
    MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TEXT,    ///< Text message received
    MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_BINARY,  ///< Binary message received
    MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_CLOSED,  ///< Connection is already closed
    MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_PONG,  ///< Pong message received
    MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT,     ///< Timeout while receiving message
    MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE,  ///< No message received

};

mla_websocket_transport_message_receive_type_t mla_websocket_transport_receive_message(mla_network_connection_t& connection, mla_size_t timeout_ms, mla_string_t& textMessage, mla_bytes_t& binaryMessage, mla_bool_t& is_final, mla_bool_t mask_message);


#endif