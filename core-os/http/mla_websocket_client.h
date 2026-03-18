//
// Created by chris on 11/13/2025.
//

/**
 * @file mla_websocket_client.h
 * @brief WebSocket client implementation for establishing and managing WebSocket connections.
 *
 * This header provides a C-style API for creating WebSocket clients that can connect to
 * WebSocket servers, send and receive text/binary messages, and handle connection lifecycle.
 * The implementation supports both text and binary message types with fragmentation support.
 */

#ifndef COREOS_MLA_WEBSOCKET_CLIENT_H
#define COREOS_MLA_WEBSOCKET_CLIENT_H

#include "../network/mla_network.h"

struct mla_websocket_client_t;

/**
 * @brief Represents a WebSocket text message.
 *
 * Used for receiving text-based WebSocket frames. Supports fragmented messages
 * through the is_final flag.
 */
struct mla_websocket_text_message_t {
    mla_bool_t is_final;    ///< True if this is the final fragment of the message
    mla_string_t message;    ///< The text content of the message
};

/**
 * @brief Represents a WebSocket binary message.
 *
 * Used for receiving binary WebSocket frames. Supports fragmented messages
 * through the is_final flag.
 */
struct mla_websocket_binary_message_t {
    mla_bool_t is_final;    ///< True if this is the final fragment of the message
    mla_bytes_t message;     ///< The binary content of the message
};

/**
 * @brief WebSocket client structure.
 *
 * Encapsulates a WebSocket connection with its underlying network connection
 * and associated user data for callbacks.
 */
struct mla_websocket_client_t {
    mla_network_connection_t connection;  ///< Underlying network connection
    mla_user_data_t userdata;       ///< User-defined data for callbacks
    mla_bool_t supports_deflate_compression; ///< Indicates if the connection supports deflate compression
};

/**
 * @brief Enumeration of message receive types.
 *
 * Indicates the type of data received or the state of the WebSocket connection
 * when attempting to receive a message.
 */
enum mla_websocket_client_message_receive_type_t: mla_uint8_t {
    MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TEXT,    ///< Text message received
    MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_BINARY,  ///< Binary message received
    MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_CLOSED,  ///< Connection is already closed
    MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TIMEOUT,     ///< timeout
    MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE,     ///< No message received
};

/**
 * @brief Creates an invalid WebSocket client instance.
 * @return An uninitialized WebSocket client structure.
 */
mla_websocket_client_t mla_websocket_client_invalid();

mla_websocket_binary_message_t mla_websocket_binary_message_empty();
mla_websocket_text_message_t mla_websocket_text_message_empty();

/**
 * @brief Establishes a WebSocket connection to a remote host.
 * @param client The WebSocket client to initialize.
 * @param url The target host information (address, port, path).
 * @param timeout_ms Connection timeout in milliseconds.
 * @return True if connection successful, false otherwise.
 */
mla_bool_t mla_websocket_client_connect(mla_websocket_client_t &client, const mla_string_t& url, mla_size_t timeout_ms, mla_bool_t supports_deflate_compression = true);


mla_bool_t mla_websocket_client_is_connected(const mla_websocket_client_t &client);

/**
 * @brief Closes the WebSocket connection.
 * @param client The WebSocket client to disconnect.
 * @return True if disconnection successful, false otherwise.
 */
mla_bool_t mla_websocket_client_disconnect(mla_websocket_client_t &client);
mla_bool_t mla_websocket_client_disconnect(mla_websocket_client_t &client, mla_uint16_t status_code,
                                           const mla_string_t &reason);


/**
 * @brief Sends a text message over the WebSocket connection.
 * @param client The WebSocket client to use.
 * @param message The text message to send.
 * @param is_final True if this is the final fragment, false for partial messages.
 * @return True if send successful, false otherwise.
 */
mla_bool_t mla_websocket_client_send_text_message(mla_websocket_client_t &client, const mla_string_t &message, mla_bool_t is_final);

/**
 * @brief Sends a binary message over the WebSocket connection.
 * @param client The WebSocket client to use.
 * @param message The binary message to send.
 * @param is_final True if this is the final fragment, false for partial messages.
 * @return True if send successful, false otherwise.
 */
mla_bool_t mla_websocket_client_send_binary_message(mla_websocket_client_t &client, const mla_bytes_t &message, mla_bool_t is_final);

/**
 * @brief Receives a message from the WebSocket connection.
 * @param client The WebSocket client to receive from.
 * @param timeout_ms Receive timeout in milliseconds.
 * @param textMessage Output parameter for text messages.
 * @param binaryMessage Output parameter for binary messages.
 * @return The type of message received or connection state.
 */
mla_websocket_client_message_receive_type_t mla_websocket_client_receive_message(mla_websocket_client_t &client, mla_size_t timeout_ms, mla_websocket_text_message_t &textMessage, mla_websocket_binary_message_t &binaryMessage);

#endif
