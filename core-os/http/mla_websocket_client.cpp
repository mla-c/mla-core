//
// Created by chris on 11/13/2025.
//

#include "mla_websocket_client.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../utils/mla_endian_utils.h"
#include "../system/mla_id.h"
#include "mla_http_utils.h"
#include "../hash/mla_sha1.h"

// WebSocket frame opcodes
#define MLA_WEBSOCKET_OPCODE_CONTINUATION 0x00
#define MLA_WEBSOCKET_OPCODE_TEXT 0x01
#define MLA_WEBSOCKET_OPCODE_BINARY 0x02
#define MLA_WEBSOCKET_OPCODE_CLOSE 0x08
#define MLA_WEBSOCKET_OPCODE_PING 0x09
#define MLA_WEBSOCKET_OPCODE_PONG 0x0A

// WebSocket frame flags
#define MLA_WEBSOCKET_FIN_BIT 0x80
#define MLA_WEBSOCKET_MASK_BIT 0x80
#define MLA_WEBSOCKET_OPCODE_MASK 0x0F
#define MLA_WEBSOCKET_LENGTH_MASK 0x7F

// WebSocket payload length thresholds
#define MLA_WEBSOCKET_LENGTH_16BIT 126
#define MLA_WEBSOCKET_LENGTH_64BIT 127
#define MLA_WEBSOCKET_LENGTH_SHORT_MAX 125
#define MLA_WEBSOCKET_LENGTH_16BIT_MAX 65535

// WebSocket masking key size
#define MLA_WEBSOCKET_MASKING_KEY_SIZE 4

// WebSocket close status codes
#define MLA_WEBSOCKET_CLOSE_NORMAL 1000

// WebSocket frame header sizes
#define MLA_WEBSOCKET_EXTENDED_LENGTH_16BIT 2
#define MLA_WEBSOCKET_EXTENDED_LENGTH_64BIT 8
#define MLA_WEBSOCKET_STATUS_CODE_SIZE 2

// Timing constants
#define MLA_WEBSOCKET_SLEEP_INTERVAL_MS 10
#define MLA_WEBSOCKET_MAX_MESSAGES_PER_CALL 10

// Pong frame response
#define MLA_WEBSOCKET_PONG_FRAME_HEADER 0x8A

mla_websocket_client_t mla_websocket_client_invalid() {
    return {
        mla_network_connection_disconnected(),
        0
    };
}

mla_string_t __mla_websocket_client_websocket_key() {

    // Generate random 16-byte WebSocket key
    mla_bytes_t random_bytes = mla_bytes(16);
    mla_byte_t *key_data = mla_bytes_get_data_for_writing(random_bytes);
    for (mla_size_t i = 0; i < 16; i++) {
        key_data[i] = (mla_byte_t)(mla_random_uint32() & 0xFF);
    }
    return mla_bytes_to_base64(random_bytes);

}

mla_bool_t mla_websocket_client_connect(mla_websocket_client_t &client, const mla_network_host_t &host,
                                        mla_size_t timeout_ms) {
    // Create TCP connection
    mla_network_connection_t connection = mla_network_connection_disconnected();
    if (!mla_network_connection_connect(connection, host, mla_connection_type_tcp, timeout_ms)) {
        return false;
    }

    mla_stream_output_t &output = connection.outputStream;
    mla_stream_input_t &input = connection.inputStream;

    // Generate random WebSocket key (16 bytes, base64 encoded)
    mla_string_t ws_key = __mla_websocket_client_websocket_key();

    // Build HTTP upgrade request
    if (!mla_stream_output_write_string(output, mla_string_const("GET / HTTP/1.1\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Host: "))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, host.address.address)) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Upgrade: websocket\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Connection: Upgrade\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Sec-WebSocket-Key: "))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, ws_key)) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("Sec-WebSocket-Version: 13\r\n"))) {
        return false;
    }
    if (!mla_stream_output_write_string(output, mla_string_const("\r\n"))) {
        return false;
    }

    // Read status line
    mla_string_t status_line = mla_string_empty();
    if (!mla_http_utils_read_line(input, status_line, timeout_ms)) {
        return false;
    }

    // Verify "HTTP/1.1 101"
    if (!mla_string_starts_with(status_line, mla_string_const("HTTP/1.1 101"))) {
        return false;
    }

    // Read and verify headers
    mla_array_list_t<mla_http_header_t, mla_http_header_initializer> headers = mla_array_list<mla_http_header_t, mla_http_header_initializer>();
    if (!mla_http_utils_read_headers(headers, input, timeout_ms)) {
        return false;
    }

    // Verify Upgrade: websocket header
    mla_string_t upgrade = mla_http_headers_get_value(headers, mla_string_const("Upgrade"));
    if (!mla_string_equals_const(upgrade, "websocket")) {
        return false;
    }



    // Verify Sec-WebSocket-Accept header
    mla_string_t accept_header = mla_http_headers_get_value(headers, mla_string_const("Sec-WebSocket-Accept"));
    if (mla_string_is_empty(accept_header)) {
        return false;
    }

    // Compute expected accept value: base64(sha1(key + GUID))
    mla_string_t key_with_guid = mla_string_concat(ws_key, mla_string_const("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
    mla_bytes_t hash = mla_sha1(mla_bytes_from_string(key_with_guid));
    mla_string_t expected_accept = mla_bytes_to_base64(hash);

    // Verify accept header matches
    if (!mla_string_equals(accept_header, expected_accept)) {
        return false;
    }

    // Connection successful, store it
    client.connection = connection;
    return true;
}


mla_bool_t __mla_websocket_client_send_masking_key(mla_stream_output_t &output, mla_uint8_t (&masking_key)[MLA_WEBSOCKET_MASKING_KEY_SIZE]) {

    for (int i = 0; i < MLA_WEBSOCKET_MASKING_KEY_SIZE; i++) {
        masking_key[i] = mla_random_uint32() & 0xFF;
    }

    return output.write(output, 0, MLA_WEBSOCKET_MASKING_KEY_SIZE, masking_key) == MLA_WEBSOCKET_MASKING_KEY_SIZE;

}

mla_bool_t __mla_websocket_client_write_message_length(mla_stream_output_t &output, mla_size_t payload_length) {

    if (payload_length < MLA_WEBSOCKET_LENGTH_16BIT) {
        mla_uint8_t mask_and_length = MLA_WEBSOCKET_MASK_BIT | (mla_uint8_t) payload_length;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;
    } else if (payload_length < MLA_WEBSOCKET_LENGTH_16BIT_MAX) {
        mla_uint8_t mask_and_length = MLA_WEBSOCKET_MASK_BIT | MLA_WEBSOCKET_LENGTH_16BIT;

        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint16_t extended_length = mla_host_to_be_uint16((mla_uint16_t) payload_length);

        if (output.write(output, 0, MLA_WEBSOCKET_EXTENDED_LENGTH_16BIT, (mla_byte_t *) &extended_length) != MLA_WEBSOCKET_EXTENDED_LENGTH_16BIT)
            return false;
    } else {
        mla_uint8_t mask_and_length = MLA_WEBSOCKET_MASK_BIT | MLA_WEBSOCKET_LENGTH_64BIT;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint64_t extended_length = mla_host_to_be_uint64(payload_length);

        if (output.write(output, 0, MLA_WEBSOCKET_EXTENDED_LENGTH_64BIT, (mla_byte_t *) &extended_length) != MLA_WEBSOCKET_EXTENDED_LENGTH_64BIT)
            return false;
    }

    return true;

}

mla_bool_t mla_websocket_client_disconnect(mla_websocket_client_t &client, mla_uint16_t status_code,
                                           const mla_string_t &reason) {
    if (!mla_network_connection_is_connected(client.connection))
        return true;

    mla_stream_output_t &output = client.connection.outputStream;

    // Calculate response payload length (2 bytes for status code + reason length)
    mla_size_t reason_length = reason.length;
    mla_size_t response_length = MLA_WEBSOCKET_STATUS_CODE_SIZE + reason_length;

    // Byte 0: FIN + Close opcode
    mla_uint8_t fin_and_opcode = MLA_WEBSOCKET_FIN_BIT | MLA_WEBSOCKET_OPCODE_CLOSE;
    output.write(output, 0, 1, &fin_and_opcode);

    if (!__mla_websocket_client_write_message_length(output, response_length))
        return false;

    // Generate masking key
    mla_uint8_t masking_key[MLA_WEBSOCKET_MASKING_KEY_SIZE];
    __mla_websocket_client_send_masking_key(output, masking_key);

    // Write masked status code (big-endian)
    mla_uint16_t status_be = mla_host_to_be_uint16(status_code);
    mla_uint8_t *status_bytes = (mla_uint8_t *) &status_be;
    for (int i = 0; i < MLA_WEBSOCKET_STATUS_CODE_SIZE; i++) {
        mla_uint8_t masked_byte = status_bytes[i] ^ masking_key[i];
        output.write(output, 0, 1, &masked_byte);
    }

    for (int i = 0; i < MLA_WEBSOCKET_STATUS_CODE_SIZE; i++) {
        mla_uint8_t masked_byte = status_bytes[i] ^ masking_key[i];
        output.write(output, 0, 1, &masked_byte);
    }

    // Write masked reason string
    for (mla_size_t i = 0; i < reason_length; i++) {
        mla_uint8_t masked_byte = reason.data[i] ^ masking_key[(i + MLA_WEBSOCKET_STATUS_CODE_SIZE) % MLA_WEBSOCKET_MASKING_KEY_SIZE];
        output.write(output, 0, 1, &masked_byte);
    }

    client = mla_websocket_client_invalid();
    return true;
}



mla_bool_t mla_websocket_client_send_text_message(mla_websocket_client_t &client, const mla_string_t &message,
                                                  mla_bool_t is_final) {
    if (!mla_network_connection_is_connected(client.connection))
        return false;

    // Sending the text message

    mla_stream_output_t &output = client.connection.outputStream;

    // Byte 0: FIN bit + opcode (0x01 for text)
    mla_uint8_t fin_and_opcode = (is_final ? MLA_WEBSOCKET_FIN_BIT : 0x00) | MLA_WEBSOCKET_OPCODE_TEXT;

    if (output.write(output, 0, 1, &fin_and_opcode) != 1)
        return false;

    // Payload length
    mla_size_t payload_length = message.length;

    if (!__mla_websocket_client_write_message_length(output, payload_length))
        return false;

    // Generate and write masking key
    mla_uint8_t masking_key[MLA_WEBSOCKET_MASKING_KEY_SIZE];
    if (!__mla_websocket_client_send_masking_key(output, masking_key))
        return false;

    // Mask and write payload

    const mla_char_t *payload = message.data;
    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t masked_byte = payload[i] ^ masking_key[i % MLA_WEBSOCKET_MASKING_KEY_SIZE];
        if (output.write(output, 0, 1, &masked_byte) != 1)
            return false;
    }

    return true;
}

mla_bool_t mla_websocket_client_send_binary_message(mla_websocket_client_t &client, const mla_bytes_t &message,
                                                    mla_bool_t is_final) {
    if (!mla_network_connection_is_connected(client.connection))
        return false;

    mla_stream_output_t &output = client.connection.outputStream;

    // Byte 0: FIN bit + opcode (0x02 for binary)
    mla_uint8_t fin_and_opcode = (is_final ? MLA_WEBSOCKET_FIN_BIT : 0x00) | MLA_WEBSOCKET_OPCODE_BINARY;

    if (output.write(output, 0, 1, &fin_and_opcode) != 1)
        return false;

    // Payload length
    mla_size_t payload_length = message.size;

    if (!__mla_websocket_client_write_message_length(output, payload_length))
        return false;

    // Generate and write masking key
    mla_uint8_t masking_key[MLA_WEBSOCKET_MASKING_KEY_SIZE];
    if (!__mla_websocket_client_send_masking_key(output, masking_key))
        return false;

    // Mask and write payload
    const mla_byte_t *payload = message.data;
    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t masked_byte = payload[i] ^ masking_key[i % MLA_WEBSOCKET_MASKING_KEY_SIZE];
        if (output.write(output, 0, 1, &masked_byte) != 1)
            return false;
    }

    return true;
}

mla_bool_t __mla_mla_websocket_client_read(mla_stream_input_t &input, mla_size_t size, mla_byte_t *buffer,
                                           mla_size_t timeout_ms) {
    mla_size_t total_read = 0;
    mla_size_t elapsed_time = 0;

    const mla_int32_t sleep_interval = MLA_WEBSOCKET_SLEEP_INTERVAL_MS; // milliseconds

    while (total_read < size) {
        mla_size_t bytes_read = input.read(input, total_read, size - total_read, buffer + total_read);
        if (bytes_read > 0) {
            total_read += bytes_read;
        } else {
            if (timeout_ms > 0 && elapsed_time >= timeout_ms) {
                break; // Timeout reached
            }
            mla_sleep(sleep_interval);
            elapsed_time += sleep_interval;
        }
    }

    return total_read == size;
}

mla_websocket_client_message_receive_type_t mla_websocket_client_receive_message(mla_websocket_client_t &client, mla_size_t timeout_ms, mla_websocket_text_message_t &textMessage, mla_websocket_binary_message_t &binaryMessage) {

    // Max process 10 messages per call to avoid blocking too long
    for (mla_size_t repeatCount = 0; repeatCount < MLA_WEBSOCKET_MAX_MESSAGES_PER_CALL; repeatCount++) {

        if (!mla_network_connection_is_connected(client.connection))
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_CLOSED;

        mla_stream_input_t &input = client.connection.inputStream;

        // Read first byte (FIN + opcode)
        mla_uint8_t fin_and_opcode;
        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &fin_and_opcode, timeout_ms))
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;

        mla_bool_t is_final = (fin_and_opcode & MLA_WEBSOCKET_FIN_BIT) != 0;
        mla_uint8_t opcode = fin_and_opcode & MLA_WEBSOCKET_OPCODE_MASK;

        // Read second byte (MASK + payload length)
        mla_uint8_t mask_and_length;
        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &mask_and_length, timeout_ms))
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;

        mla_bool_t is_masked = (mask_and_length & MLA_WEBSOCKET_MASK_BIT) != 0;
        mla_uint64_t payload_length = mask_and_length & MLA_WEBSOCKET_LENGTH_MASK;

        // Read extended payload length if needed
        if (payload_length == MLA_WEBSOCKET_LENGTH_16BIT) {

            mla_uint16_t extended_length;
            if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint16_t), (mla_byte_t *) &extended_length,
                                                 timeout_ms))
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
            payload_length = mla_be_to_host_uint16(extended_length);

        } else if (payload_length == MLA_WEBSOCKET_LENGTH_64BIT) {

            mla_uint64_t extended_length;
            if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint64_t), (mla_byte_t *) &extended_length,
                                                 timeout_ms))
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;

            payload_length = mla_be_to_host_uint64(extended_length);
        }

        // Read masking key if present
        mla_uint8_t masking_key[MLA_WEBSOCKET_MASKING_KEY_SIZE] = {0};
        if (is_masked) {
            if (!__mla_mla_websocket_client_read(input, sizeof(masking_key), masking_key, timeout_ms))
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
        }

        // Read and unmask payload
        if (payload_length > mla_size_max) {
            // Payload too large to handle
            mla_warning(mla_string_concat("WebSocket client received payload too large: ", mla_string_from_uint64(payload_length)));
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
        }

        mla_bytes_t payload = mla_bytes((mla_size_t)payload_length);
        mla_byte_t *payload_write = mla_bytes_get_data_for_writing(payload);

        if (payload_write == nullptr) {
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
        }

        for (mla_size_t i = 0; i < payload_length; i++) {
            mla_uint8_t byte = 0;

            if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &byte, timeout_ms)) {
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
            }
            payload_write[i] = is_masked ? (byte ^ masking_key[i % MLA_WEBSOCKET_MASKING_KEY_SIZE]) : byte;
        }

        // Handle different opcodes
        if (opcode == MLA_WEBSOCKET_OPCODE_TEXT) {
            // Text message

            textMessage = {
                is_final,
                mla_bytes_to_string(payload)
            };
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TEXT;

        } else if (opcode == MLA_WEBSOCKET_OPCODE_BINARY) {
            // Binary message

            binaryMessage = {
                is_final,
                payload
            };

            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_BINARY;
        } else if (opcode == MLA_WEBSOCKET_OPCODE_CLOSE) {
            // Close

            // Extract status code from payload if present
            mla_uint16_t status_code = MLA_WEBSOCKET_CLOSE_NORMAL; // Default: normal closure
            if (payload_length >= MLA_WEBSOCKET_STATUS_CODE_SIZE) {
                mla_uint16_t status_be = ((mla_uint16_t) payload.data[0] << 8) | payload.data[1];
                status_code = mla_be_to_host_uint16(status_be);
            }

            mla_websocket_client_disconnect(client, status_code, mla_string_empty());

            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_CLOSED;
        } else if (opcode == MLA_WEBSOCKET_OPCODE_PING) {
            // Ping

            // Send pong frame
            mla_stream_output_t &output = client.connection.outputStream;
            mla_uint8_t pong_frame[2] = {MLA_WEBSOCKET_PONG_FRAME_HEADER, (mla_uint8_t) payload_length}; // FIN + Pong opcode

            if (output.write(output, 0, 2, pong_frame) != 2) {
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
            }

            // Echo payload if present
            if (payload_length > 0) {
                if (output.write(output, 0, (mla_size_t)payload_length, payload.data) != (mla_size_t)payload_length) {
                    return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
                }
            }

            // Start again
            continue;

        } else if (opcode == MLA_WEBSOCKET_OPCODE_PONG) {
            // Pong
            // Ignore pong frames
            continue;

        } else {
            // Unknown opcode, ignore frame
            mla_warning(mla_string_concat("WebSocket client received unknown opcode: ", mla_string_from_uint32(opcode)));
            continue;
        }

    }

    return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
}
