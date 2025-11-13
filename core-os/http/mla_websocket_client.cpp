//
// Created by chris on 11/13/2025.
//

#include "mla_websocket_client.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../utils/mla_endian_utils.h"

mla_websocket_client_t mla_websocket_client_invalid() {
    return {
        mla_network_connection_disconnected(),
        0
    };
}

mla_bool_t mla_websocket_client_connect(mla_websocket_client_t &client, const mla_network_host_t &host,
                                        mla_size_t timeout_ms) {
    (void)client;
    (void)host;
    (void)timeout_ms;
    return false;
}

mla_bool_t mla_websocket_client_disconnect(mla_websocket_client_t &client, mla_uint16_t status_code,
                                           const mla_string_t &reason) {
    if (!mla_network_connection_is_connected(client.connection))
        return true;

    mla_stream_output_t &output = client.connection.outputStream;

    // Calculate response payload length (2 bytes for status code + reason length)
    mla_size_t reason_length = reason.length;
    mla_size_t response_length = 2 + reason_length;

    // Byte 0: FIN + Close opcode
    mla_uint8_t fin_and_opcode = 0x88;
    output.write(output, 0, 1, &fin_and_opcode);

    // Byte 1: MASK bit + payload length
    if (response_length < 126) {
        mla_uint8_t mask_and_length = 0x80 | (mla_uint8_t) response_length;
        output.write(output, 0, 1, &mask_and_length);
    } else if (response_length < 65536) {
        mla_uint8_t mask_and_length = 0x80 | 126;
        output.write(output, 0, 1, &mask_and_length);
        mla_uint16_t extended_length = mla_host_to_be_uint16((mla_uint16_t) response_length);
        output.write(output, 0, 2, (mla_byte_t *) &extended_length);
    } else {
        mla_uint8_t mask_and_length = 0x80 | 127;
        output.write(output, 0, 1, &mask_and_length);
        mla_uint64_t extended_length = mla_host_to_be_uint64(response_length);
        output.write(output, 0, 8, (mla_byte_t *) &extended_length);
    }

    // Generate masking key
    mla_uint8_t masking_key[4];
    for (int i = 0; i < 4; i++) {
        masking_key[i] = rand() & 0xFF;
    }
    output.write(output, 0, 4, masking_key);

    // Write masked status code (big-endian)
    mla_uint16_t status_be = mla_host_to_be_uint16(status_code);
    mla_uint8_t *status_bytes = (mla_uint8_t *) &status_be;
    for (int i = 0; i < 2; i++) {
        mla_uint8_t masked_byte = status_bytes[i] ^ masking_key[i];
        output.write(output, 0, 1, &masked_byte);
    }

    for (int i = 0; i < 2; i++) {
        mla_uint8_t masked_byte = status_bytes[i] ^ masking_key[i];
        output.write(output, 0, 1, &masked_byte);
    }

    // Write masked reason string
    for (mla_size_t i = 0; i < reason_length; i++) {
        mla_uint8_t masked_byte = reason.data[i] ^ masking_key[(i + 2) % 4];
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
    mla_uint8_t fin_and_opcode = (is_final ? 0x80 : 0x00) | 0x01;

    if (output.write(output, 0, 1, &fin_and_opcode) != 1)
        return false;

    // Payload length
    mla_size_t payload_length = message.length;

    if (payload_length < 126) {
        mla_uint8_t mask_and_length = 0x80 | (mla_uint8_t) payload_length;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;
    } else if (payload_length < 65536) {
        mla_uint8_t mask_and_length = 0x80 | 126;

        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint16_t extended_length = mla_host_to_be_uint16((mla_uint16_t) payload_length);

        if (output.write(output, 0, 2, (mla_byte_t *) &extended_length) != 2)
            return false;
    } else {
        mla_uint8_t mask_and_length = 0x80 | 127;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint64_t extended_length = mla_host_to_be_uint64(payload_length);

        if (output.write(output, 0, 8, (mla_byte_t *) &extended_length) != 8)
            return false;
    }

    // Generate and write masking key
    mla_uint8_t masking_key[4];
    for (int i = 0; i < 4; i++) {
        masking_key[i] = rand() & 0xFF;
    }

    if (output.write(output, 0, 4, masking_key) != 4)
        return false;

    // Mask and write payload

    const mla_char_t *payload = message.data;
    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t masked_byte = payload[i] ^ masking_key[i % 4];
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
    mla_uint8_t fin_and_opcode = (is_final ? 0x80 : 0x00) | 0x02;

    if (output.write(output, 0, 1, &fin_and_opcode) != 1)
        return false;

    // Payload length
    mla_size_t payload_length = message.size;

    if (payload_length < 126) {
        mla_uint8_t mask_and_length = 0x80 | (mla_uint8_t) payload_length;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;
    } else if (payload_length < 65536) {
        mla_uint8_t mask_and_length = 0x80 | 126;

        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint16_t extended_length = mla_host_to_be_uint16((mla_uint16_t) payload_length);

        if (output.write(output, 0, 2, (mla_byte_t *) &extended_length) != 2)
            return false;
    } else {
        mla_uint8_t mask_and_length = 0x80 | 127;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint64_t extended_length = mla_host_to_be_uint64(payload_length);

        if (output.write(output, 0, 8, (mla_byte_t *) &extended_length) != 8)
            return false;
    }

    // Generate and write masking key
    mla_uint8_t masking_key[4];
    for (int i = 0; i < 4; i++) {
        masking_key[i] = rand() & 0xFF;
    }

    if (output.write(output, 0, 4, masking_key) != 4)
        return false;

    // Mask and write payload
    const mla_byte_t *payload = message.data;
    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t masked_byte = payload[i] ^ masking_key[i % 4];
        if (output.write(output, 0, 1, &masked_byte) != 1)
            return false;
    }

    return true;
}

mla_bool_t __mla_mla_websocket_client_read(mla_stream_input_t &input, mla_size_t size, mla_byte_t *buffer,
                                           mla_size_t timeout_ms) {
    mla_size_t total_read = 0;
    mla_size_t elapsed_time = 0;
    const mla_int32_t sleep_interval = 10; // milliseconds

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
    for (mla_size_t repeatCount = 0; repeatCount < 10; repeatCount++) {

        if (!mla_network_connection_is_connected(client.connection))
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_CLOSED;

        mla_stream_input_t &input = client.connection.inputStream;

        // Read first byte (FIN + opcode)
        mla_uint8_t fin_and_opcode;
        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &fin_and_opcode, timeout_ms))
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;

        mla_bool_t is_final = (fin_and_opcode & 0x80) != 0;
        mla_uint8_t opcode = fin_and_opcode & 0x0F;

        // Read second byte (MASK + payload length)
        mla_uint8_t mask_and_length;
        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &mask_and_length, timeout_ms))
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;

        mla_bool_t is_masked = (mask_and_length & 0x80) != 0;
        mla_uint64_t payload_length = mask_and_length & 0x7F;

        // Read extended payload length if needed
        if (payload_length == 126) {
            mla_uint16_t extended_length;
            if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint16_t), (mla_byte_t *) &extended_length,
                                                 timeout_ms))
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
            payload_length = mla_be_to_host_uint16(extended_length);
        } else if (payload_length == 127) {
            mla_uint64_t extended_length;
            if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint64_t), (mla_byte_t *) &extended_length,
                                                 timeout_ms))
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;

            payload_length = mla_be_to_host_uint64(extended_length);
        }

        // Read masking key if present
        mla_uint8_t masking_key[4] = {0};
        if (is_masked) {
            if (!__mla_mla_websocket_client_read(input, sizeof(masking_key), masking_key, timeout_ms))
                return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TIMEOUT;
        }

        // Read and unmask payload
        if (payload_length > SIZE_MAX) {
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
            payload_write[i] = is_masked ? (byte ^ masking_key[i % 4]) : byte;
        }

        // Handle different opcodes
        if (opcode == 0x01) {
            // Text message

            textMessage = {
                is_final,
                mla_bytes_to_string(payload)
            };
            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_TEXT;

        } else if (opcode == 0x02) {
            // Binary message

            binaryMessage = {
                is_final,
                payload
            };

            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_BINARY;
        } else if (opcode == 0x08) {
            // Close

            // Extract status code from payload if present
            mla_uint16_t status_code = 1000; // Default: normal closure
            if (payload_length >= 2) {
                mla_uint16_t status_be = ((mla_uint16_t) payload.data[0] << 8) | payload.data[1];
                status_code = mla_be_to_host_uint16(status_be);
            }

            mla_websocket_client_disconnect(client, status_code, mla_string_empty());

            return MLA_WEBSOCKET_CLIENT_MESSAGE_TYPE_CLOSED;
        } else if (opcode == 0x09) {
            // Ping

            // Send pong frame
            mla_stream_output_t &output = client.connection.outputStream;
            mla_uint8_t pong_frame[2] = {0x8A, (mla_uint8_t) payload_length}; // FIN + Pong opcode

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

        } else if (opcode == 0x0A) {
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