//
// Created by chris on 11/15/2025.
//

#include  "mla_websocket_utils.h"
#include "../utils/mla_endian_utils.h"
#include "../system/mla_id.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

// WebSocket frame opcodes
#define mla_websocket_opcode_continuation 0x00
#define mla_websocket_opcode_text 0x01
#define mla_websocket_opcode_binary 0x02
#define mla_websocket_opcode_close 0x08
#define mla_websocket_opcode_ping 0x09
#define mla_websocket_opcode_pong 0x0A

// WebSocket frame flags
#define mla_websocket_fin_bit 0x80
#define mla_websocket_mask_bit 0x80
#define mla_websocket_opcode_mask 0x0F
#define mla_websocket_length_mask 0x7F

// WebSocket payload length thresholds
#define mla_websocket_length_16bit 126
#define mla_websocket_length_64bit 127
#define mla_websocket_length_short_max 125
#define mla_websocket_length_16bit_max 65535

// WebSocket masking key size
#define mla_websocket_masking_key_size 4



// WebSocket frame header sizes
#define mla_websocket_extended_length_16bit 2
#define mla_websocket_extended_length_64bit 8
#define mla_websocket_status_code_size 2

// Timing constants
#define mla_websocket_sleep_interval_ms 10

// Pong frame response
#define mla_websocket_pong_frame_header 0x8A


mla_bool_t __mla_websocket_client_send_masking_key(mla_stream_output_t &output, mla_uint8_t (&masking_key)[mla_websocket_masking_key_size]) {

    for (int i = 0; i < mla_websocket_masking_key_size; i++) {
        masking_key[i] = mla_random_uint32() & 0xFF;
    }

    return output.write(output, 0, mla_websocket_masking_key_size, masking_key) == mla_websocket_masking_key_size;

}

mla_bool_t __mla_websocket_client_write_message_length(mla_stream_output_t &output, mla_size_t payload_length) {

    if (payload_length < mla_websocket_length_16bit) {
        mla_uint8_t mask_and_length = mla_websocket_mask_bit | (mla_uint8_t) payload_length;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;
    } else if (payload_length < mla_websocket_length_16bit_max) {
        mla_uint8_t mask_and_length = mla_websocket_mask_bit | mla_websocket_length_16bit;

        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint16_t extended_length = mla_host_to_be_uint16((mla_uint16_t) payload_length);

        if (output.write(output, 0, mla_websocket_extended_length_16bit, (mla_byte_t *) &extended_length) != mla_websocket_extended_length_16bit)
            return false;
    } else {
        mla_uint8_t mask_and_length = mla_websocket_mask_bit | mla_websocket_length_64bit;
        if (output.write(output, 0, 1, &mask_and_length) != 1)
            return false;

        mla_uint64_t extended_length = mla_host_to_be_uint64(payload_length);

        if (output.write(output, 0, mla_websocket_extended_length_64bit, (mla_byte_t *) &extended_length) != mla_websocket_extended_length_64bit)
            return false;
    }

    return true;

}

mla_bool_t mla_websocket_transport_send_close_frame(mla_stream_output_t &output, mla_uint16_t status_code, const mla_string_t &reason) {

    // Calculate response payload length (2 bytes for status code + reason length)
    mla_size_t reason_length = mla_string_length(reason);
    mla_size_t response_length = mla_websocket_status_code_size + reason_length;

    // Byte 0: FIN + Close opcode
    mla_uint8_t fin_and_opcode = mla_websocket_fin_bit | mla_websocket_opcode_close;
    output.write(output, 0, 1, &fin_and_opcode);

    if (!__mla_websocket_client_write_message_length(output, response_length))
        return false;

    // Generate masking key
    mla_uint8_t masking_key[mla_websocket_masking_key_size];
    __mla_websocket_client_send_masking_key(output, masking_key);

    // Write masked status code (big-endian)
    mla_uint16_t status_be = mla_host_to_be_uint16(status_code);
    mla_uint8_t *status_bytes = (mla_uint8_t *) &status_be;
    for (int i = 0; i < mla_websocket_status_code_size; i++) {
        mla_uint8_t masked_byte = status_bytes[i] ^ masking_key[i];
        output.write(output, 0, 1, &masked_byte);
    }

    for (int i = 0; i < mla_websocket_status_code_size; i++) {
        mla_uint8_t masked_byte = status_bytes[i] ^ masking_key[i];
        output.write(output, 0, 1, &masked_byte);
    }

    const mla_char_t* data = mla_string_data(reason);

    // Write masked reason string
    for (mla_size_t i = 0; i < reason_length; i++) {
        mla_uint8_t masked_byte = data[i] ^ masking_key[(i + mla_websocket_status_code_size) % mla_websocket_masking_key_size];
        output.write(output, 0, 1, &masked_byte);
    }

    return true;

}

mla_bool_t mla_websocket_transport_send_text_frame(mla_stream_output_t &output, const mla_string_t &message, mla_bool_t is_final) {

    // Byte 0: FIN bit + opcode (0x01 for text)
    mla_uint8_t fin_and_opcode = (is_final ? mla_websocket_fin_bit : 0x00) | mla_websocket_opcode_text;

    if (output.write(output, 0, 1, &fin_and_opcode) != 1)
        return false;

    // Payload length
    mla_size_t payload_length = mla_string_length(message);

    if (!__mla_websocket_client_write_message_length(output, payload_length))
        return false;

    // Generate and write masking key
    mla_uint8_t masking_key[mla_websocket_masking_key_size];
    if (!__mla_websocket_client_send_masking_key(output, masking_key))
        return false;

    // Mask and write payload

    const mla_char_t *payload = mla_string_data(message);
    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t masked_byte = payload[i] ^ masking_key[i % mla_websocket_masking_key_size];
        if (output.write(output, 0, 1, &masked_byte) != 1)
            return false;
    }

    return true;
}

mla_bool_t mla_websocket_transport_send_binary_frame(mla_stream_output_t &output, const mla_bytes_t &message, mla_bool_t is_final) {

    // Byte 0: FIN bit + opcode (0x02 for binary)
    mla_uint8_t fin_and_opcode = (is_final ? mla_websocket_fin_bit : 0x00) | mla_websocket_opcode_binary;

    if (output.write(output, 0, 1, &fin_and_opcode) != 1)
        return false;

    // Payload length
    mla_size_t payload_length = message.size;

    if (!__mla_websocket_client_write_message_length(output, payload_length))
        return false;

    // Generate and write masking key
    mla_uint8_t masking_key[mla_websocket_masking_key_size];
    if (!__mla_websocket_client_send_masking_key(output, masking_key))
        return false;

    // Mask and write payload
    const mla_byte_t *payload = message.data;
    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t masked_byte = payload[i] ^ masking_key[i % mla_websocket_masking_key_size];
        if (output.write(output, 0, 1, &masked_byte) != 1)
            return false;
    }

    return true;
}

mla_bool_t __mla_mla_websocket_client_read(mla_stream_input_t &input, mla_size_t size, mla_byte_t *buffer,
                                           mla_size_t timeout_ms) {
    mla_size_t total_read = 0;
    mla_size_t elapsed_time = 0;

    const mla_int32_t sleep_interval = mla_websocket_sleep_interval_ms; // milliseconds

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

mla_websocket_transport_message_receive_type_t mla_websocket_transport_receive_message(mla_network_connection_t& connection, mla_size_t timeout_ms, mla_string_t& textMessage, mla_bytes_t& binaryMessage, mla_bool_t& is_final) {

    mla_stream_input_t &input = connection.inputStream;

    // read first byte (fin + opcode)
    mla_uint8_t fin_and_opcode;
    if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &fin_and_opcode, timeout_ms))
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE;

    is_final = (fin_and_opcode & mla_websocket_fin_bit) != 0;
    mla_uint8_t opcode = fin_and_opcode & mla_websocket_opcode_mask;

    // read second byte (mask + payload length)
    mla_uint8_t mask_and_length;
    if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &mask_and_length, timeout_ms))
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;

    mla_bool_t is_masked = (mask_and_length & mla_websocket_mask_bit) != 0;
    mla_uint64_t payload_length = mask_and_length & mla_websocket_length_mask;

    // read extended payload length if needed
    if (payload_length == mla_websocket_length_16bit) {

        mla_uint16_t extended_length;
        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint16_t), (mla_byte_t *) &extended_length,
                                             timeout_ms))
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;


        payload_length = mla_be_to_host_uint16(extended_length);

    } else if (payload_length == mla_websocket_length_64bit) {

        mla_uint64_t extended_length;
        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint64_t), (mla_byte_t *) &extended_length,
                                             timeout_ms))
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;

        payload_length = mla_be_to_host_uint64(extended_length);
    }

    // read masking key if present
    mla_uint8_t masking_key[mla_websocket_masking_key_size] = {0};
    if (is_masked) {
        if (!__mla_mla_websocket_client_read(input, sizeof(masking_key), masking_key, timeout_ms))
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
    }

    // read and unmask payload
    if (payload_length > mla_size_max) {
        // payload too large to handle
        mla_warning(mla_string_concat("websocket client received payload too large: ", mla_string_from_uint64(payload_length)));
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
    }

    mla_bytes_t payload = mla_bytes((mla_size_t)payload_length);
    mla_byte_t *payload_write = mla_bytes_get_data_for_writing(payload);

    if (payload_write == nullptr) {
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
    }

    for (mla_size_t i = 0; i < payload_length; i++) {
        mla_uint8_t byte = 0;

        if (!__mla_mla_websocket_client_read(input, sizeof(mla_uint8_t), &byte, timeout_ms)) {
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
        }
        payload_write[i] = is_masked ? (byte ^ masking_key[i % mla_websocket_masking_key_size]) : byte;
    }

    // handle different opcodes
    if (opcode == mla_websocket_opcode_text) {
        // text message
        textMessage = mla_bytes_to_string(payload);
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TEXT;

    } else if (opcode == mla_websocket_opcode_binary) {
        // binary message

        binaryMessage = payload;
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_BINARY;
    } else if (opcode == mla_websocket_opcode_close) {
        // close

        // extract status code from payload if present
        mla_uint16_t status_code = mla_websocket_close_normal; // default: normal closure
        if (payload_length >= mla_websocket_status_code_size) {
            mla_uint16_t status_be = ((mla_uint16_t) payload.data[0] << 8) | payload.data[1];
            status_code = mla_be_to_host_uint16(status_be);
        }

        mla_websocket_transport_send_close_frame(connection.outputStream, status_code, mla_string_empty());
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_CLOSED;

    } else if (opcode == mla_websocket_opcode_ping) {
        // ping

        // send pong frame
        mla_stream_output_t &output = connection.outputStream;
        mla_uint8_t pong_frame[2] = {mla_websocket_pong_frame_header, (mla_uint8_t) payload_length}; // fin + pong opcode

        if (output.write(output, 0, 2, pong_frame) != 2) {
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
        }

        // echo payload if present
        if (payload_length > 0) {
            if (output.write(output, 0, (mla_size_t)payload_length, payload.data) != (mla_size_t)payload_length) {
                return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
            }
        }

        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE;

    } else if (opcode == mla_websocket_opcode_pong) {
        // pong
        // ignore pong frames
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_PONG;

    } else {
        // unknown opcode, ignore frame
        mla_warning(mla_string_concat("websocket client received unknown opcode: ", mla_string_from_uint32(opcode)));
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
    }


}
