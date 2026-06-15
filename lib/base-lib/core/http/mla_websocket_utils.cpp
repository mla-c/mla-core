//
// Created by chris on 11/15/2025.
//

#include  "mla_websocket_utils.h"
#include "../utils/mla_endian_utils.h"
#include "../system/mla_id.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

#define mla_websocket_stream_small_buffer_size 1024 // 1 KB

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
#define mla_websocket_rsv1_bit 0x40
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


mla_bool_t mla_internal_websocket_transport_write_frame_header(mla_stream_output_t &output, mla_bool_t is_text_message, mla_bool_t is_final_frame, mla_bool_t use_deflate_compression) {

    mla_uint8_t fin_and_opcode;

    if (is_text_message) {
        // Byte 0: FIN bit + opcode (0x01 for text)
        fin_and_opcode = (is_final_frame ? mla_websocket_fin_bit : 0x00) | mla_websocket_opcode_text;
    } else {
        // Byte 0: FIN bit + opcode (0x02 for binary)
        fin_and_opcode = (is_final_frame ? mla_websocket_fin_bit : 0x00) | mla_websocket_opcode_binary;
    }

    if (use_deflate_compression) {
        fin_and_opcode |= mla_websocket_rsv1_bit;
    }

    if (output.write(output, 0, 1, &fin_and_opcode) != 1) {
        return false;
    }

    return true;

}

void mla_internal_websocket_client_init_masking_key(mla_uint8_t (&masking_key)[mla_websocket_masking_key_size]) {

    for (int i = 0; i < mla_websocket_masking_key_size; i++) {
        masking_key[i] = mla_random_uint32() & 0xFF;
    }

}

mla_bool_t mla_internal_websocket_client_send_masking_key(mla_stream_output_t &output, const mla_uint8_t (&masking_key)[mla_websocket_masking_key_size]) {
    return output.write(output, 0, mla_websocket_masking_key_size, masking_key) == mla_websocket_masking_key_size;

}

mla_bool_t mla_internal_websocket_client_write_message_length(mla_stream_output_t &output, mla_size_t payload_length, mla_bool_t mask_message) {

    if (payload_length < mla_websocket_length_16bit) {
        mla_uint8_t mask_and_length;

        if (mask_message) {
             mask_and_length = mla_websocket_mask_bit | (mla_uint8_t) payload_length;
        } else {
             mask_and_length = (mla_uint8_t) payload_length;
        }

        if (output.write(output, 0, 1, &mask_and_length) != 1) {
            return false;
        }

    } else if (payload_length < mla_websocket_length_16bit_max) {

        mla_uint8_t mask_and_length;

        if (mask_message) {
             mask_and_length = mla_websocket_mask_bit | mla_websocket_length_16bit;
        } else {
             mask_and_length = mla_websocket_length_16bit;
        }

        if (output.write(output, 0, 1, &mask_and_length) != 1) {
            return false;
        }

        mla_uint16_t extended_length = mla_host_to_be_uint16((mla_uint16_t) payload_length);

        if (output.write(output, 0, mla_websocket_extended_length_16bit, (mla_byte_t *) &extended_length) != mla_websocket_extended_length_16bit) {
            return false;
        }
    } else {
        mla_uint8_t mask_and_length;

        if (mask_message) {
             mask_and_length = mla_websocket_mask_bit | mla_websocket_length_64bit;
        } else {
             mask_and_length = mla_websocket_length_64bit;
        }

        if (output.write(output, 0, 1, &mask_and_length) != 1) {
            return false;
        }

        mla_uint64_t extended_length = mla_host_to_be_uint64(payload_length);

        if (output.write(output, 0, mla_websocket_extended_length_64bit, (mla_byte_t *) &extended_length) != mla_websocket_extended_length_64bit) {
            return false;
        }
    }

    return true;

}

struct mla_websocket_masking_state_t {
    mla_uint8_t masking_key[mla_websocket_masking_key_size];
    mla_size_t byte_offset;
};

mla_user_data_id_init(mla_websocket_transport_mask_user_data_name)

mla_size_t mla_internal_websocket_transport_masked_write(mla_stream_output_t& wrapper, mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    mla_websocket_masking_state_t* state = mla_user_data_get_pointer_data<mla_websocket_masking_state_t>(wrapper.userdata, mla_websocket_transport_mask_user_data_name);

    if (state == nullptr) {
        return 0;
    }

    for (mla_size_t i = 0; i < length; i++) {
        mla_uint8_t masked_byte = buffer[offset + i] ^ state->masking_key[(state->byte_offset + i) % mla_websocket_masking_key_size];
        if (output.write(output, 0, 1, &masked_byte) != 1) {
            state->byte_offset += i;
            return i;
        }
    }

    state->byte_offset += length;
    return length;
}

mla_bool_t mla_websocket_transport_send_close_frame(mla_stream_output_t &output, mla_uint16_t status_code, const mla_string_t &reason, mla_bool_t mask_message) {

    // Calculate response payload length (2 bytes for status code + reason length)
    mla_size_t reason_length = mla_string_length(reason);

    // Close frame payload is limited to 125 bytes per WebSocket spec
    if (reason_length > mla_websocket_length_short_max - mla_websocket_status_code_size) {
        reason_length = mla_websocket_length_short_max - mla_websocket_status_code_size;
    }

    mla_size_t response_length = mla_websocket_status_code_size + reason_length;

    // Build close payload into a contiguous stack buffer
    mla_byte_t close_payload[mla_websocket_status_code_size + mla_websocket_length_short_max];
    mla_uint16_t status_be = mla_host_to_be_uint16(status_code);
    mla_memcpy(close_payload, &status_be, mla_websocket_status_code_size);
    if (reason_length > 0) {
        mla_memcpy(close_payload + mla_websocket_status_code_size, mla_string_data(reason), reason_length);
    }

    // Byte 0: FIN + Close opcode
    mla_uint8_t fin_and_opcode = mla_websocket_fin_bit | mla_websocket_opcode_close;
    if (output.write(output, 0, 1, &fin_and_opcode) != 1) {
        return false;
    }

    if (!mla_internal_websocket_client_write_message_length(output, response_length, mask_message)) {
        return false;
    }

    mla_websocket_masking_state_t masking_state = {};
    mla_stream_output_t final_out = output;

    if (mask_message) {
        mla_internal_websocket_client_init_masking_key(masking_state.masking_key);
        if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
            return false;
        }

        final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
        mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
        mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
    }

    if (final_out.write(final_out, 0, response_length, close_payload) != response_length) {
        return false;
    }

    return true;

}

mla_bool_t mla_internal_websocket_transport_send_with_generator(mla_stream_output_t &output, mla_bool_t is_text_message, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t mask_message, mla_bool_t use_deflate_compression) {

    // If its an short input we can optimize by writing it directly
    mla_memory_stream_t temp_stream = mla_memory_stream(mla_websocket_stream_small_buffer_size, false);

    mla_websocket_masking_state_t masking_state = {};
    if (mask_message) {
        mla_internal_websocket_client_init_masking_key(masking_state.masking_key);
    }

    if (message_generator(temp_stream.output, userData)) {

        // Short Message

        mla_memory_stream_set_position(temp_stream, 0);
        mla_size_t payload_length = mla_memory_stream_get_size(temp_stream);

        if (use_deflate_compression && payload_length > mla_global_config_stream_output_deflate_min_compression_data_size) {

            if (!mla_internal_websocket_transport_write_frame_header(output, is_text_message, true, true)) {
                return false;
            }

            // Calc the compressed size
            mla_stream_output_t size_calc_stream = mla_stream_output_size_calculation();
            mla_stream_output_t final_out = mla_stream_output_deflate_compress_wrapper(size_calc_stream, mla_deflate_mode_raw_websocket);

            if (!mla_stream_copy(temp_stream.input, final_out)) {
                return false;
            }

            mla_stream_output_deflate_finish(final_out);

            mla_size_t compressed_size = mla_stream_output_size_calculation_get_size(size_calc_stream);
            final_out = output;
            size_calc_stream = mla_stream_noop_output();

            // Write the real output
            if (!mla_internal_websocket_client_write_message_length(output, compressed_size, mask_message)) {
                return false;
            }

            if (mask_message) {
                if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                    return false;
                }

                // We need to generate the message again and mask it on the fly since we don't want to buffer the entire message in memory
                final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
                mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
                mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
            }

            final_out = mla_stream_output_deflate_compress_wrapper(final_out, mla_deflate_mode_raw_websocket);

            mla_memory_stream_set_position(temp_stream, 0);

            if (!mla_stream_copy(temp_stream.input, final_out)) {
                return false;
            }

            if (!mla_stream_output_deflate_finish(final_out)) {
                return false;
            }

        } else {

            if (!mla_internal_websocket_transport_write_frame_header(output, is_text_message, true, false)) {
                return false;
            }

            mla_stream_output_t final_out = output;

            if (!mla_internal_websocket_client_write_message_length(output, payload_length, mask_message)) {
                return false;
            }

            if (mask_message) {
                if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                    return false;
                }

                // We need to generate the message again and mask it on the fly since we don't want to buffer the entire message in memory
                final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
                mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
                mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
            }

            if (!mla_stream_copy(temp_stream.input, final_out)) {
                return false;
            }

        }

    } else {

        if (!mla_internal_websocket_transport_write_frame_header(output, is_text_message, true, use_deflate_compression)) {
            return false;
        }

        // Long Message
        mla_stream_output_t size_calculation_stream = mla_stream_output_size_calculation();
        mla_stream_output_t final_out = mla_stream_noop_output();

        if (use_deflate_compression) {
            final_out = mla_stream_output_deflate_compress_wrapper(size_calculation_stream, mla_deflate_mode_raw_websocket);
        } else {
            final_out = size_calculation_stream;
        }

        if (!message_generator(final_out, userData)) {
            return false;
        }

        if (use_deflate_compression) {
            if (!mla_stream_output_deflate_finish(final_out)) {
                return false;
            }

        }

        mla_size_t compressed_size = mla_stream_output_size_calculation_get_size(size_calculation_stream);

        final_out = output;
        size_calculation_stream = mla_stream_noop_output();

        // Write the real output
        if (!mla_internal_websocket_client_write_message_length(output, compressed_size, mask_message)) {
            return false;
        }

        if (mask_message) {
            if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                return false;
            }

            // We need to generate the message again and mask it on the fly since we don't want to buffer the entire message in memory
            final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
            mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
            mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
        }

        if (use_deflate_compression) {
            final_out = mla_stream_output_deflate_compress_wrapper(final_out, mla_deflate_mode_raw_websocket);
        }

        if (!message_generator(final_out, userData)) {
            return false;
        }

        if (use_deflate_compression) {
            if (!mla_stream_output_deflate_finish(final_out)) {
                return false;
            }

        }
    }

    return true;


}

mla_bool_t mla_websocket_transport_send_text_with_generator(mla_stream_output_t &output, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t mask_message, mla_bool_t use_deflate_compression) {
    return mla_internal_websocket_transport_send_with_generator(output, true, userData, message_generator, mask_message, use_deflate_compression);
}

mla_bool_t mla_websocket_transport_send_text_frame(mla_stream_output_t &output, const mla_string_t &message, mla_bool_t mask_message, mla_bool_t use_deflate_compression) {

    // Payload length
    mla_size_t payload_length = mla_string_length(message);

    mla_websocket_masking_state_t masking_state = {};
    if (mask_message) {
        mla_internal_websocket_client_init_masking_key(masking_state.masking_key);
    }

    // Check if it make sense to use compression
    if (use_deflate_compression && payload_length > mla_global_config_stream_output_deflate_min_compression_data_size) {

        if (!mla_internal_websocket_transport_write_frame_header(output, true, true, true)) {
            return false;
        }

        // Calc the compressed size
        mla_stream_output_t size_calc_stream = mla_stream_output_size_calculation();
        mla_stream_output_t final_out = mla_stream_output_deflate_compress_wrapper(size_calc_stream, mla_deflate_mode_raw_websocket);

        mla_stream_input_t payload_input = mla_stream_input_from_string(message);
        if (!mla_stream_copy(payload_input, final_out)) {
            return false;
        }

        mla_stream_output_deflate_finish(final_out);

        mla_size_t compressed_size = mla_stream_output_size_calculation_get_size(size_calc_stream);
        final_out = output;
        size_calc_stream = mla_stream_noop_output();

        // Write the real output
        if (!mla_internal_websocket_client_write_message_length(output, compressed_size, mask_message)) {
            return false;
        }

        if (mask_message) {
            if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                return false;
            }

            final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
            mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
            mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
        }

        final_out = mla_stream_output_deflate_compress_wrapper(final_out, mla_deflate_mode_raw_websocket);

        payload_input = mla_stream_input_from_string(message);
        if (!mla_stream_copy(payload_input, final_out)) {
            return false;
        }

        if (!mla_stream_output_deflate_finish(final_out)) {
            return false;
        }

    } else {

        if (!mla_internal_websocket_transport_write_frame_header(output, true, true, false)) {
            return false;
        }

        mla_stream_output_t final_out = output;

        if (!mla_internal_websocket_client_write_message_length(output, payload_length, mask_message)) {
            return false;
        }

        if (mask_message) {
            if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                return false;
            }

            final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
            mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
            mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
        }

        if (final_out.write(final_out, 0, payload_length, (const mla_byte_t *) mla_string_data(message)) != payload_length) {
            return false;
        }

    }

    return true;
}

mla_bool_t mla_websocket_transport_send_binary_with_generator(mla_stream_output_t &output, mla_user_data_t &userData, mla_websocket_transport_message_generator_t message_generator, mla_bool_t mask_message, mla_bool_t use_deflate_compression) {
    return mla_internal_websocket_transport_send_with_generator(output, false, userData, message_generator, mask_message, use_deflate_compression);
}

mla_bool_t mla_websocket_transport_send_binary_frame(mla_stream_output_t &output, const mla_bytes_t &message, mla_bool_t mask_message, mla_bool_t use_deflate_compression) {

    // Payload length
    mla_size_t payload_length = mla_bytes_length(message);
    const mla_byte_t *payload = mla_bytes_get_data_readonly(message);

    mla_websocket_masking_state_t masking_state = {};
    if (mask_message) {
        mla_internal_websocket_client_init_masking_key(masking_state.masking_key);
    }

    // Check if it make sense to use compression
    if (use_deflate_compression && payload_length > mla_global_config_stream_output_deflate_min_compression_data_size) {

        if (!mla_internal_websocket_transport_write_frame_header(output, false, true, true)) {
            return false;
        }

        // Calc the compressed size
        mla_stream_output_t size_calc_stream = mla_stream_output_size_calculation();
        mla_stream_output_t final_out = mla_stream_output_deflate_compress_wrapper(size_calc_stream, mla_deflate_mode_raw_websocket);

        mla_stream_input_t payload_input = mla_stream_input_from_buffer((mla_byte_t *) payload, payload_length);
        if (!mla_stream_copy(payload_input, final_out)) {
            return false;
        }

        mla_stream_output_deflate_finish(final_out);

        mla_size_t compressed_size = mla_stream_output_size_calculation_get_size(size_calc_stream);
        final_out = output;
        size_calc_stream = mla_stream_noop_output();

        // Write the real output
        if (!mla_internal_websocket_client_write_message_length(output, compressed_size, mask_message)) {
            return false;
        }

        if (mask_message) {
            if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                return false;
            }

            final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
            mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
            mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
        }

        final_out = mla_stream_output_deflate_compress_wrapper(final_out, mla_deflate_mode_raw_websocket);

        payload_input = mla_stream_input_from_buffer((mla_byte_t *) payload, payload_length);
        if (!mla_stream_copy(payload_input, final_out)) {
            return false;
        }

        if (!mla_stream_output_deflate_finish(final_out)) {
            return false;
        }

    } else {

        if (!mla_internal_websocket_transport_write_frame_header(output, false, true, false)) {
            return false;
        }

        mla_stream_output_t final_out = output;

        if (!mla_internal_websocket_client_write_message_length(output, payload_length, mask_message)) {
            return false;
        }

        if (mask_message) {
            if (!mla_internal_websocket_client_send_masking_key(output, masking_state.masking_key)) {
                return false;
            }

            final_out = mla_stream_output_interceptor_wrapper(final_out, mla_internal_websocket_transport_masked_write, nullptr);
            mla_pointer_t masking_state_ptr = mla_platform_pointer_to_managed_pointer(reinterpret_cast<const mla_websocket_masking_state_t*>(&masking_state));
            mla_user_data_set_pointer(final_out.userdata, mla_websocket_transport_mask_user_data_name, masking_state_ptr);
        }

        if (final_out.write(final_out, 0, payload_length, payload) != payload_length) {
            return false;
        }

    }

    return true;
}

mla_bool_t mla_internal_mla_websocket_client_read(mla_stream_input_t &input, mla_size_t size, mla_byte_t *buffer,
                                           mla_size_t timeout_ms) {
    mla_size_t total_read = 0;
    mla_size_t elapsed_time = 0;

    const mla_int32_t sleep_interval = mla_websocket_sleep_interval_ms; // milliseconds

    while (total_read < size) {
        mla_size_t bytes_read = input.read(input, total_read, size - total_read, buffer);
        if (bytes_read > 0) {
            total_read += bytes_read;
        } else {
            if (timeout_ms == 0 || elapsed_time >= timeout_ms) {
                break; // Timeout reached
            }
            mla_sleep(sleep_interval);
            elapsed_time += sleep_interval;
        }
    }

    return total_read == size;
}

mla_websocket_transport_message_receive_type_t mla_websocket_transport_receive_message(mla_network_connection_t& connection, mla_size_t timeout_ms, mla_string_t& textMessage, mla_bytes_t& binaryMessage, mla_bool_t mask_message) {

    mla_stream_input_t &input = connection.inputStream;

    // read first byte (fin + opcode)
    mla_uint8_t fin_and_opcode;
    mla_bool_t is_final_frame;
    mla_uint8_t opcode;
    mla_bool_t use_deflate_compression;

    mla_memory_stream_t payload_data = mla_memory_stream_empty();
    do {

        // At the first call we dont wait for any timeouts we just look if there is something
        if (!mla_internal_mla_websocket_client_read(input, sizeof(mla_uint8_t), &fin_and_opcode, 0)) {
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_NO_MESSAGE;
        }

        is_final_frame = (fin_and_opcode & mla_websocket_fin_bit) != 0;
        opcode = fin_and_opcode & mla_websocket_opcode_mask;
        use_deflate_compression = (( fin_and_opcode & mla_websocket_rsv1_bit) != 0);

        // read second byte (mask + payload length)
        mla_uint8_t mask_and_length;
        if (!mla_internal_mla_websocket_client_read(input, sizeof(mla_uint8_t), &mask_and_length, timeout_ms)) {
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
        }

        mla_bool_t is_masked = (mask_and_length & mla_websocket_mask_bit) != 0;
        mla_uint64_t payload_length = mask_and_length & mla_websocket_length_mask;

        // read extended payload length if needed
        if (payload_length == mla_websocket_length_16bit) {

            mla_uint16_t extended_length;
            if (!mla_internal_mla_websocket_client_read(input, sizeof(mla_uint16_t), (mla_byte_t *) &extended_length,
                                                 timeout_ms)) {
                return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
            }

            payload_length = mla_be_to_host_uint16(extended_length);

        } else if (payload_length == mla_websocket_length_64bit) {

            mla_uint64_t extended_length;
            if (!mla_internal_mla_websocket_client_read(input, sizeof(mla_uint64_t), (mla_byte_t *) &extended_length,
                                                 timeout_ms)) {
                return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
            }

            payload_length = mla_be_to_host_uint64(extended_length);
        }

        // read masking key if present
        mla_uint8_t masking_key[mla_websocket_masking_key_size] = {0};
        if (is_masked) {
            if (!mla_internal_mla_websocket_client_read(input, sizeof(masking_key), masking_key, timeout_ms)) {
                return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
            }
        }

        // read and unmask payload
        if (payload_length > mla_size_max) {
            // payload too large to handle
            mla_warning(mla_string_concat("websocket client received payload too large: ", mla_string_from_uint64(payload_length)));
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
        }

        if (payload_length > 0) {

            mla_size_t current_stream_size = mla_memory_stream_get_size(payload_data);

            // Init Memory Stream if empty
            if (current_stream_size == 0) {

                if (is_final_frame) {
                    payload_data = mla_memory_stream((mla_size_t)payload_length, true);
                } else {
                    payload_data = mla_memory_stream((mla_size_t)mla_min(payload_length, mla_global_config_stream_fast_read_buffer_size), true);
                }
            }

            // Copy to memory stream
            mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size] = {0};

            while (payload_length > 0) {

                mla_size_t chunk_size = (payload_length > sizeof(buffer)) ? sizeof(buffer) : (mla_size_t) payload_length;

                if (!mla_internal_mla_websocket_client_read(input, chunk_size, buffer, timeout_ms)) {
                    return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
                }

                if (is_masked) {
                    for (mla_size_t i = 0; i < chunk_size; i++) {
                        buffer[i] ^= masking_key[(current_stream_size + i) % mla_websocket_masking_key_size];
                    }
                }

                if (payload_data.output.write(payload_data.output, 0, chunk_size, buffer) != chunk_size) {
                    return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
                }

                current_stream_size += chunk_size;
                payload_length -= chunk_size;
            }
        }

    } while (!is_final_frame);

    mla_memory_stream_set_position(payload_data, 0);
    mla_size_t payload_size = mla_memory_stream_get_size(payload_data);
    mla_stream_input_t final_payload_stream = payload_data.input;

    if (use_deflate_compression) {
        // RFC 7692 permessage-deflate: the sender strips the trailing 4 bytes
        // (00 00 FF FF) from the compressed payload before transmitting.
        // Re-append them so the raw-DEFLATE decompressor can cleanly find the
        // end-of-stream marker.
        mla_memory_stream_set_position(payload_data, payload_size);
        const mla_byte_t websocket_deflate_tail[4] = { 0x00, 0x00, 0xFF, 0xFF };
        payload_data.output.write(payload_data.output, 0, 4, websocket_deflate_tail);

        // Recalc the final_payload_size
        mla_memory_stream_set_position(payload_data, 0);
        payload_size = mla_stream_input_deflate_decompressed_size_calculation(final_payload_stream);
        mla_memory_stream_set_position(payload_data, 0);
        // Create the wrapper for the decompression
        final_payload_stream = mla_stream_input_deflate_decompress_wrapper(payload_data.input);
    }

    // handle different opcodes
    if (opcode == mla_websocket_opcode_text) {
        // text message
        textMessage = mla_string_from_stream(final_payload_stream, payload_size);
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TEXT;

    } else if (opcode == mla_websocket_opcode_binary) {
        // binary message

        binaryMessage = mla_bytes_from_stream(final_payload_stream, payload_size);
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_BINARY;
    } else if (opcode == mla_websocket_opcode_close) {
        // close

        // extract status code from payload if present
        mla_uint16_t status_code = mla_websocket_close_normal; // default: normal closure
        if (payload_size >= mla_websocket_status_code_size) {

            mla_byte_t buffer[2] = {0};
            final_payload_stream.read(final_payload_stream, 0, mla_websocket_status_code_size, buffer);

            mla_uint16_t status_be = ((mla_uint16_t) buffer[0] << 8) | buffer[1];
            status_code = mla_be_to_host_uint16(status_be);
        }

        mla_websocket_transport_send_close_frame(connection.outputStream, status_code, mla_string_empty(), mask_message);
        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_CLOSED;

    } else if (opcode == mla_websocket_opcode_ping) {
        // ping

        // send pong frame
        mla_stream_output_t &output = connection.outputStream;
        mla_uint8_t content_length = (mla_uint8_t) mla_min(payload_size, (mla_size_t)mla_websocket_length_short_max);
        mla_uint8_t pong_length_byte = content_length;
        if (mask_message) {
            pong_length_byte |= mla_websocket_mask_bit;
        }
        mla_uint8_t pong_frame[2] = {mla_websocket_pong_frame_header, pong_length_byte};

        if (output.write(output, 0, 2, pong_frame) != 2) {
            return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
        }

        if (content_length > 0) {

            mla_byte_t buffer[mla_websocket_length_short_max] = {0};

            if (final_payload_stream.read(final_payload_stream, 0, content_length, buffer) != content_length) {
                return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
            }


            if (mask_message) {

                mla_uint8_t masking_key[mla_websocket_masking_key_size] = {0};
                mla_internal_websocket_client_init_masking_key(masking_key);

                if (!mla_internal_websocket_client_send_masking_key(output, masking_key)) {
                    return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
                }

                // Echo masked payload if present
                for (mla_size_t i = 0; i < content_length; i++) {
                    mla_uint8_t masked_byte = buffer[i] ^ masking_key[i % mla_websocket_masking_key_size];
                    if (output.write(output, 0, 1, &masked_byte) != 1) {
                        return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
                    }
                }
            } else {
                // Echo unmasked payload if present
                if (output.write(output, 0, (mla_size_t)content_length, buffer) != (mla_size_t)content_length) {
                    return MLA_WEBSOCKET_TRANSPORT_MESSAGE_RECEIVE_TYPE_TIMEOUT;
                }

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