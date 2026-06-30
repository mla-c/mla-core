//
// Created by chris on 6/30/2026.
//

#include "mla_ui_html_template_compile.h"

#include "../../system/mla_string_builder.h"

#define mla_ui_html_template_compile_code_block_start "<%"
#define mla_ui_html_template_compile_code_block_end "%>"

mla_bool_t mla_private_ui_html_template_compile_emit_code_block(mla_string_builder_t& current_block, mla_ui_html_template_compile_writer_t& writer) {

    if (mla_string_builder_length(current_block) == 0) {
        return true;
    }

    if (writer.emit_code == nullptr) {
        return true; // If no emit_code function is provided, we can just ignore the code block and continue
    }

    mla_string_t code_block = mla_string_builder_to_string(current_block);

    if (!writer.emit_code(writer, code_block)) {
        return false;
    }

    return true;
}

mla_bool_t mla_private_ui_html_template_compile_emit_html_block(mla_string_builder_t& current_block, mla_ui_html_template_compile_writer_t& writer) {

    if (mla_string_builder_length(current_block) == 0) {
        return true;
    }

    if (writer.emit_html == nullptr) {
        return true; // If no emit_html function is provided, we can just ignore the HTML block and continue
    }

    mla_string_t html_block = mla_string_builder_to_string(current_block);

    if (!writer.emit_html(writer, html_block)) {
        return false;
    }

    return true;
}

mla_bool_t mla_ui_html_template_compile(mla_stream_input_t& input_stream, mla_ui_html_template_compile_writer_t& writer) {

    mla_bool_t emit_code = false;
    mla_bool_t has_pending_delimiter_char = false;
    mla_char_t pending_delimiter_char = 0;

    mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size];

    mla_string_builder_t current_block = mla_string_builder_create();

    while (true) {

        mla_size_t bytes_read = input_stream.read(input_stream, 0, sizeof(buffer), buffer);
        if (bytes_read == 0) {
            break;
        }

        for (mla_size_t i = 0; i < bytes_read; i++) {
            mla_char_t current_char = mla_s_cast<mla_char_t>(buffer[i]);

            if (has_pending_delimiter_char) {

                if (!emit_code && pending_delimiter_char == mla_ui_html_template_compile_code_block_start[0] && current_char == mla_ui_html_template_compile_code_block_start[1]) {

                    if (!mla_private_ui_html_template_compile_emit_html_block(current_block, writer)) {
                        return false;
                    }

                    current_block = mla_string_builder_create();
                    emit_code = true;
                    has_pending_delimiter_char = false;
                    continue;

                }

                if (emit_code && pending_delimiter_char == mla_ui_html_template_compile_code_block_end[0] && current_char == mla_ui_html_template_compile_code_block_end[1]) {

                    if (!mla_private_ui_html_template_compile_emit_code_block(current_block, writer)) {
                        return false;
                    }

                    current_block = mla_string_builder_create();
                    emit_code = false;
                    has_pending_delimiter_char = false;
                    continue;

                }

                if (!mla_string_builder_append(current_block, pending_delimiter_char)) {
                    return false;
                }

                has_pending_delimiter_char = false;
            }

            if (!emit_code && current_char == mla_ui_html_template_compile_code_block_start[0]) {
                has_pending_delimiter_char = true;
                pending_delimiter_char = current_char;
                continue;
            }

            if (emit_code && current_char == mla_ui_html_template_compile_code_block_end[0]) {
                has_pending_delimiter_char = true;
                pending_delimiter_char = current_char;
                continue;
            }

            if (!mla_string_builder_append(current_block, current_char)) {
                return false;
            }

        }

    }

    if (has_pending_delimiter_char) {
        if (!mla_string_builder_append(current_block, pending_delimiter_char)) {
            return false;
        }
    }

    // Emit any remaining content in the current block
    if (emit_code) {

        if (!mla_private_ui_html_template_compile_emit_code_block(current_block, writer)) {
            return false;
        }

    } else {

        if (!mla_private_ui_html_template_compile_emit_html_block(current_block, writer)) {
            return false;
        }

    }

    return true;

}

mla_ui_html_template_compile_writer_t mla_ui_html_template_compile_writer_noop() {
    return {
        mla_user_data_empty(),
        nullptr,
        nullptr
    };
}

mla_user_data_id_init(mla_ui_html_template_compile_writer_html_only_user_data_id);

mla_bool_t mla_private_ui_html_template_compile_writer_html_only_emit_html(mla_ui_html_template_compile_writer_t& writer, const mla_string_t& html_block) {

    mla_stream_output_t* output_stream = mla_user_data_get_struct_data<mla_stream_output_t>(writer.user_data, mla_ui_html_template_compile_writer_html_only_user_data_id);

    if (output_stream == nullptr) {
        return false;
    }

    return mla_stream_output_write_string(*output_stream, html_block);
}

mla_ui_html_template_compile_writer_t mla_ui_html_template_compile_writer_html_only(mla_stream_output_t& output_stream) {

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_struct(user_data, mla_ui_html_template_compile_writer_html_only_user_data_id, output_stream);

    return {
        user_data,
        mla_private_ui_html_template_compile_writer_html_only_emit_html,
        nullptr
    };
}
