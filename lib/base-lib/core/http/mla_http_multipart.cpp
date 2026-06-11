#include "mla_http_multipart.h"
#include "mla_http_utils.h"
#include "../system/mla_string_builder.h"
#include "mla_http_chunked_stream.h"
#include "../mla_data_types.h"
#include "../system/mla_string_concat.h"

mla_user_data_id_init(mla_http_client_multipart_context_user_data_id)

struct mla_http_client_multipart_context_t {
    mla_string_t field_name;
    mla_string_t file_name;
    mla_string_t content_type;
    mla_stream_input_t file_content;
    mla_string_t boundary;
    mla_bool_t is_chunked;
};

static mla_bool_t mla_http_multipart_content_writer(const mla_http_request_content_writer_t &writer,
                                                    const mla_stream_output_t &outputStream) {
    mla_pointer_t ctx_ptr = mla_user_data_get_pointer(writer.userData, mla_http_client_multipart_context_user_data_id);

    mla_http_client_multipart_context_t *ctx = mla_pointer_get_data<mla_http_client_multipart_context_t>(ctx_ptr);

    if (!ctx)
        return false;

    mla_stream_output_t out = outputStream;
    mla_http_chunked_stream_output_t chunked = mla_http_chunked_stream_output_invalid();

    if (ctx->is_chunked) {
        chunked = mla_http_chunked_stream_output(out);
        out = chunked.output;
    }

    mla_string_builder_t sb = mla_string_builder_create(256);
    mla_bool_t ok = mla_string_builder_append(sb, mla_string_const("--"));
    ok = ok && mla_string_builder_append(sb, ctx->boundary);
    ok = ok && mla_string_builder_append(sb, mla_string_const("\r\n"));

    ok = ok && mla_string_builder_append(sb, mla_string_const("Content-Disposition: form-data; name=\""));
    ok = ok && mla_string_builder_append(sb, ctx->field_name);
    ok = ok && mla_string_builder_append(sb, mla_string_const("\"; filename=\""));
    ok = ok && mla_string_builder_append(sb, ctx->file_name);
    ok = ok && mla_string_builder_append(sb, mla_string_const("\"\r\n"));

    ok = ok && mla_string_builder_append(sb, mla_string_const("Content-Type: "));
    ok = ok && mla_string_builder_append(sb, ctx->content_type);
    ok = ok && mla_string_builder_append(sb, mla_string_const("\r\n\r\n"));

    mla_string_t header_str = mla_string_builder_to_string(sb);
    mla_string_builder_reset(sb);

    ok = ok && mla_stream_output_write_string(out, header_str);

    if (ok) {
        ok = mla_stream_copy(ctx->file_content, out);
    }

    if (ok) {
        mla_string_t footer_str = mla_string_concat("\r\n--", ctx->boundary, "--\r\n");
        ok = mla_stream_output_write_string(out, footer_str);
    }

    if (ctx->is_chunked) {
        if (!mla_http_chunked_stream_output_finished(chunked)) {
            ok = false;
        }
    }

    return ok;
}

mla_http_client_response_t mla_http_client_upload_file(
    const mla_http_client_t &client,
    const mla_string_t &url,
    const mla_string_t &field_name,
    const mla_string_t &file_name,
    const mla_string_t &content_type,
    mla_stream_input_t& file_content) {
    mla_http_request_t request = mla_http_post_request(url);

    mla_string_t boundary = mla_string_const("----MlaMultipartBoundary123456789");
    mla_string_t contentTypeHeader = mla_string_concat("multipart/form-data; boundary=", boundary);
    mla_http_headers_add(request.headers, mla_string_const("Content-Type"), contentTypeHeader);

    mla_size_t file_len = mla_size_max;
    if (file_content.remaining_bytes) {
        file_len = file_content.remaining_bytes(file_content);
    }

    mla_bool_t is_chunked = false;

    if (file_len != mla_size_max) {
        mla_size_t header_len = 2 + mla_string_length(boundary) + 2 +
                                38 + mla_string_length(field_name) + 13 + mla_string_length(file_name) + 3 +
                                14 + mla_string_length(content_type) + 4;
        mla_size_t footer_len = 4 + mla_string_length(boundary) + 4;
        mla_size_t total_len = header_len + file_len + footer_len;

        mla_string_t len_str = mla_string_from_size(total_len);
        mla_http_headers_add(request.headers, mla_string_const("Content-Length"), len_str);
    } else {
        mla_http_headers_add(request.headers, mla_string_const("Transfer-Encoding"), mla_string_const("chunked"));
        is_chunked = true;
    }

    mla_http_client_multipart_context_t ctx = {
        field_name,
        file_name,
        content_type,
        file_content,
        boundary,
        is_chunked
    };

    mla_user_data_t ud = mla_user_data_empty();
    mla_pointer_t ctx_ptr = mla_platform_pointer_to_managed_pointer(&ctx);
    mla_user_data_set_pointer(ud, mla_http_client_multipart_context_user_data_id, ctx_ptr);

    request.contentWriter = mla_http_request_content_writer(
        ud,
        mla_http_multipart_content_writer
    );

    mla_http_client_response_t response = mla_http_client_send_request(client, request);


    return response;
}

////////////////////////////////////////////////////////////////
/// Server Multipart Helper
////////////////////////////////////////////////////////////////

mla_user_data_id_init(mla_http_server_multipart_context_user_data_id)

struct mla_multipart_stream_state_t {
    mla_stream_input_t base_stream;
    mla_string_t target_boundary;
    mla_byte_t buffer[mla_global_config_http_multipart_parser_buffer_size];
    mla_size_t buffer_len;
    mla_size_t buffer_pos;
    mla_bool_t hit_boundary;
    mla_bool_t is_last_part;
    mla_bool_t eof;

    static mla_multipart_stream_state_t init() {
        return {
            mla_stream_noop_input(),
            mla_string_empty(),
            {0},
            0,
            0,
            false,
            false,
            false
        };
    }
};

mla_bool_t __mla_http_server_parse_multipart_refill_buffer(mla_multipart_stream_state_t *state) {
    if (state->buffer_pos < state->buffer_len)
        return true;

    if (state->eof)
        return false;

    mla_size_t r = state->base_stream.read(state->base_stream, 0, sizeof(state->buffer), state->buffer);

    if (r == 0) {
        state->eof = true;
        return false;
    }

    state->buffer_len = r;
    state->buffer_pos = 0;
    return true;
}

mla_size_t __mla_http_server_parse_multipart_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length,
                                                  mla_byte_t *buffer) {
    (void) offset;

    mla_multipart_stream_state_t *state = mla_user_data_get_pointer_data<mla_multipart_stream_state_t>(
        input.userdata, mla_http_server_multipart_context_user_data_id);

    if (state == nullptr) {
        return 0;
    }

    if (state->hit_boundary)
        return 0;

    mla_string_t target = state->target_boundary;

    mla_size_t bytes_read = 0;

    while (bytes_read < length) {
        if (!__mla_http_server_parse_multipart_refill_buffer(state)) {
            break;
        }

        mla_byte_t b = state->buffer[state->buffer_pos];

        if (b == mla_string_data(target)[0]) {
            mla_size_t lookahead = 0;
            mla_bool_t matched = true;

            while (lookahead < mla_string_length(target)) {
                if (state->buffer_pos + lookahead >= state->buffer_len) {
                    mla_size_t remaining = state->buffer_len - state->buffer_pos;
                    for (mla_size_t i = 0; i < remaining; i++) {
                        state->buffer[i] = state->buffer[state->buffer_pos + i];
                    }
                    state->buffer_pos = 0;
                    state->buffer_len = remaining;

                    if (state->eof) {
                        matched = false;
                        break;
                    }

                    mla_size_t r = state->base_stream.read(state->base_stream, 0,
                                                           sizeof(state->buffer) - state->buffer_len,
                                                           state->buffer + state->buffer_len);
                    if (r == 0) {
                        state->eof = true;
                        matched = false;
                        break;
                    }
                    state->buffer_len += r;
                }

                if (state->buffer[state->buffer_pos + lookahead] != mla_string_data(target)[lookahead]) {
                    matched = false;
                    break;
                }
                lookahead++;
            }

            if (matched) {
                state->hit_boundary = true;
                state->buffer_pos += mla_string_length(target);

                if (state->buffer_pos + 1 < state->buffer_len) {
                    if (state->buffer[state->buffer_pos] == '-' && state->buffer[state->buffer_pos + 1] == '-') {
                        state->is_last_part = true;
                    }
                } else {
                    mla_size_t remaining = state->buffer_len - state->buffer_pos;
                    mla_memcpy(state->buffer, state->buffer + state->buffer_pos, remaining);
                    state->buffer_pos = 0;
                    state->buffer_len = remaining;
                    mla_size_t r = state->base_stream.read(state->base_stream, 0,
                                                           sizeof(state->buffer) - state->buffer_len,
                                                           state->buffer + state->buffer_len);
                    if (r > 0) state->buffer_len += r;
                    if (state->buffer_len >= 2 && state->buffer[0] == '-' && state->buffer[1] == '-') {
                        state->is_last_part = true;
                    }
                }
                break;
            }
        }

        buffer[bytes_read++] = b;
        state->buffer_pos++;
    }

    return bytes_read;
}

mla_bool_t __mla_http_server_parse_multipart_read_line(mla_multipart_stream_state_t *state, mla_string_builder_t &sb) {
    mla_string_builder_reset(sb);

    while (true) {
        if (!__mla_http_server_parse_multipart_refill_buffer(state)) {
            return mla_string_builder_length(sb) > 0;
        }

        mla_byte_t b = state->buffer[state->buffer_pos++];
        if (b == '\n') {
            return true;
        } else if (b != '\r') {
            mla_string_builder_append(sb, (mla_char_t) b);
        }
    }
}

void __mla_http_server_parse_multipart_parse_content_disposition(const mla_string_t &header, mla_string_t &field_name,
                                                                 mla_string_t &file_name) {
    mla_string_t name_key = mla_string_const("name=\"");
    mla_string_t file_key = mla_string_const("filename=\"");
    mla_string_t end_key = mla_string_const("\"");

    // Extract the name
    mla_int32_t name_pos = mla_string_index_of(header, name_key);

    if (name_pos > -1) {
        mla_size_t start = name_pos + mla_string_length(name_key);

        mla_int32_t end_pos = mla_string_index_of(header, end_key, start);
        mla_size_t end = end_pos > -1 ? end_pos : mla_string_length(header);
        field_name = mla_string_substr(header, start, end - start);
    }

    // Extract the filename
    mla_int32_t filename_pos = mla_string_index_of(header, file_key);

    if (filename_pos > -1) {
        mla_size_t start = filename_pos + mla_string_length(file_key);

        mla_int32_t end_pos = mla_string_index_of(header, end_key, start);
        mla_size_t end = end_pos > -1 ? end_pos : mla_string_length(header);
        file_name = mla_string_substr(header, start, end - start);
    }
}

mla_http_server_multipart_parse_context_t mla_http_server_multipart_parse_context_empty() {
    return {
        mla_user_data_empty()
    };
}

mla_bool_t mla_http_server_parse_multipart_create_context(const mla_http_request_t &request,
                                                          mla_http_server_multipart_parse_context_t &context) {
    mla_string_t content_type_header = mla_http_headers_get_value(request.headers, mla_string_const("Content-Type"));

    if (mla_string_is_empty(content_type_header))
        return false;

    mla_string_t boundary_prefix = mla_string_const("boundary=");
    mla_int32_t index_of = mla_string_index_of(content_type_header, boundary_prefix);

    if (index_of < 0) {
        return false;
    }

    mla_size_t pos = index_of + mla_string_length(boundary_prefix);
    mla_string_t boundary = mla_string_substr(content_type_header, pos);
    mla_string_t target_boundary = mla_string_concat("\r\n--", boundary);


    mla_pointer_t state_ptr = mla_malloc_struct(mla_multipart_stream_state_t);

    mla_multipart_stream_state_t *state = mla_pointer_get_data<mla_multipart_stream_state_t>(state_ptr);

    if (state == nullptr)
        return false;

    state->base_stream = request.content;
    state->target_boundary = target_boundary;

    mla_string_builder_t sb = mla_string_builder_create();

    // remove the new line char
    mla_string_t boundary_start = mla_string_substr(target_boundary, 2);

    // Search for where the boundary starts
    mla_bool_t success = false;
    while (__mla_http_server_parse_multipart_read_line(state, sb)) {
        mla_string_t line = mla_string_builder_to_string(sb);

        if (mla_string_starts_with(line, boundary_start)) {
            success = true;
            break;
        }
    }

    if (success) {
        mla_user_data_t ud = mla_user_data_empty();
        mla_user_data_set_pointer(ud, mla_http_server_multipart_context_user_data_id, state_ptr);

        context = {
            ud
        };

        return true;
    } else {
        return false;
    }
}

mla_bool_t mla_http_server_parse_multipart_next_item(mla_http_server_multipart_parse_context_t &context, mla_http_multipart_part_t &result) {

    mla_multipart_stream_state_t *state = mla_user_data_get_pointer_data<mla_multipart_stream_state_t>(context.userdata, mla_http_server_multipart_context_user_data_id);

    if (state == nullptr) {
        return false;
    }

    mla_string_builder_t sb = mla_string_builder_create();

    while (!state->eof && !state->is_last_part) {

        state->hit_boundary = false;
        mla_string_t field_name = mla_string_empty();
        mla_string_t file_name = mla_string_empty();
        mla_string_t part_content_type = mla_string_empty();

        while (__mla_http_server_parse_multipart_read_line(state, sb)) {
            // Found empty line
            if (mla_string_builder_length(sb) == 0)
                break;

            mla_string_t line = mla_string_builder_to_string(sb);

            mla_string_t cd_prefix = mla_string_const("Content-Disposition:");
            mla_string_t ct_prefix = mla_string_const("Content-Type:");

            if (mla_string_starts_with(line, cd_prefix)) {
                __mla_http_server_parse_multipart_parse_content_disposition(line, field_name, file_name);
            } else if (mla_string_starts_with(line, ct_prefix)) {
                part_content_type = mla_string_substr(line, mla_string_length(ct_prefix));
                part_content_type = mla_string_trim(part_content_type);
            }
        }


        mla_stream_input_t part_stream = {
            context.userdata,
            __mla_http_server_parse_multipart_read,
            nullptr
        };

        // Prepare the result data
        result = {
            field_name,
            file_name,
            part_content_type,
            part_stream
        };

        return true;
    }

    return false;

}

mla_bool_t mla_http_server_parse_multipart_finish_item(mla_http_server_multipart_parse_context_t& context) {

    mla_multipart_stream_state_t *state = mla_user_data_get_pointer_data<mla_multipart_stream_state_t>(context.userdata, mla_http_server_multipart_context_user_data_id);

    if (state == nullptr) {
        return false;
    }

    if (!state->hit_boundary) {
        mla_byte_t dump[mla_global_config_stream_fast_read_buffer_size];
        while (state->base_stream.read(state->base_stream, 0, sizeof(dump), dump) > 0);
    }

    return true;
}




mla_http_multipart_part_t mla_http_multipart_part_empty() {
    return {
        mla_string_empty(),
        mla_string_empty(),
        mla_string_empty(),
        mla_stream_noop_input()
    };
}
