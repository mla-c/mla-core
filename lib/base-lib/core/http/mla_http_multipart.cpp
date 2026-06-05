#include "mla_http_multipart.h"
#include "mla_http_utils.h"
#include "../system/mla_string_builder.h"
#include "mla_http_chunked_stream.h"
#include "../mla_data_types.h"

struct mla_http_client_multipart_context_t {
    mla_string_t field_name;
    mla_string_t file_name;
    mla_string_t content_type;
    mla_stream_input_t file_content;
    mla_string_t boundary;
    mla_bool_t is_chunked;
};

static mla_bool_t mla_http_multipart_content_writer(const mla_http_request_content_writer_t& writer, const mla_stream_output_t &outputStream) {
    mla_platform_pointer_t ptr = mla_pointer_get_platform_pointer(writer.userData.data);
    mla_http_client_multipart_context_t* ctx = (mla_http_client_multipart_context_t*)ptr;
    if (!ctx) return false;

    mla_stream_output_t out = outputStream; 
    mla_http_chunked_stream_output_t chunked = mla_http_chunked_stream_output_invalid();
    
    if (ctx->is_chunked) {
        chunked = mla_http_chunked_stream_output(out);
        out = chunked.output;
    }

    mla_string_builder_t sb = mla_string_builder_create(256);
    mla_string_builder_append(sb, mla_string_const("--"));
    mla_string_builder_append(sb, ctx->boundary);
    mla_string_builder_append(sb, mla_string_const("\r\n"));
    
    mla_string_builder_append(sb, mla_string_const("Content-Disposition: form-data; name=\""));
    mla_string_builder_append(sb, ctx->field_name);
    mla_string_builder_append(sb, mla_string_const("\"; filename=\""));
    mla_string_builder_append(sb, ctx->file_name);
    mla_string_builder_append(sb, mla_string_const("\"\r\n"));

    mla_string_builder_append(sb, mla_string_const("Content-Type: "));
    mla_string_builder_append(sb, ctx->content_type);
    mla_string_builder_append(sb, mla_string_const("\r\n\r\n"));

    mla_string_t header_str = mla_string_builder_to_string(sb);
    mla_string_builder_reset(sb);

    mla_bool_t ok = mla_stream_output_write_string(out, header_str);
    mla_string_destroy(header_str);

    if (ok) {
        ok = mla_stream_copy(ctx->file_content, out);
    }

    if (ok) {
        mla_string_builder_t sb_foot = mla_string_builder_create(64);
        mla_string_builder_append(sb_foot, mla_string_const("\r\n--"));
        mla_string_builder_append(sb_foot, ctx->boundary);
        mla_string_builder_append(sb_foot, mla_string_const("--\r\n"));
        mla_string_t footer_str = mla_string_builder_to_string(sb_foot);
        mla_string_builder_reset(sb_foot);

        ok = mla_stream_output_write_string(out, footer_str);
        mla_string_destroy(footer_str);
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
    const mla_string_t& url, 
    const mla_string_t& field_name, 
    const mla_string_t& file_name, 
    const mla_string_t& content_type, 
    mla_stream_input_t file_content)
{
    mla_http_request_t request = mla_http_post_request(url);

    mla_string_t boundary = mla_string_const("----MlaMultipartBoundary123456789");

    mla_string_builder_t sb = mla_string_builder_create(64);
    mla_string_builder_append(sb, mla_string_const("multipart/form-data; boundary="));
    mla_string_builder_append(sb, boundary);
    mla_string_t contentTypeHeader = mla_string_builder_to_string(sb);
    mla_string_builder_reset(sb);

    mla_http_headers_add(request.headers, mla_string_const("Content-Type"), contentTypeHeader);
    mla_string_destroy(contentTypeHeader);

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
        mla_string_destroy(len_str);
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
    ud.data = mla_platform_pointer_to_managed_pointer((mla_platform_pointer_t)&ctx);

    request.contentWriter = mla_http_request_content_writer(
        ud,
        mla_http_multipart_content_writer
    );

    mla_http_client_response_t response = mla_http_client_send_request(client, request);

    mla_array_list_destroy(request.headers);
    mla_string_destroy(request.url);
    mla_string_destroy(request.method);

    return response;
}

////////////////////////////////////////////////////////////////
/// Server Multipart Helper
////////////////////////////////////////////////////////////////

struct mla_multipart_stream_state_t {
    mla_stream_input_t base_stream;
    mla_string_t boundary;
    mla_bool_t eof;
    mla_string_t target_boundary;
    mla_byte_t buffer[1024];
    mla_size_t buffer_len;
    mla_size_t buffer_pos;
    mla_bool_t hit_boundary;
    mla_bool_t is_last_part;
};

static mla_bool_t __refill_buffer(mla_multipart_stream_state_t* state) {
    if (state->buffer_pos < state->buffer_len) return true;
    if (state->eof) return false;
    
    mla_size_t r = state->base_stream.read(state->base_stream, 0, sizeof(state->buffer), state->buffer);
    if (r == 0) {
        state->eof = true;
        return false;
    }
    state->buffer_len = r;
    state->buffer_pos = 0;
    return true;
}

static mla_size_t mla_multipart_part_read(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    mla_platform_pointer_t ptr = mla_pointer_get_platform_pointer(input.userdata.data);
    mla_multipart_stream_state_t* state = (mla_multipart_stream_state_t*)ptr;
    if (state->hit_boundary) return 0;
    
    mla_string_t target = state->target_boundary;
    
    mla_size_t bytes_read = 0;
    
    while (bytes_read < length) {
        if (!__refill_buffer(state)) {
            break;
        }
        
        mla_bool_t found_possible_start = false;
        mla_size_t match_len = 0;
        
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
                    
                    mla_size_t r = state->base_stream.read(state->base_stream, 0, sizeof(state->buffer) - state->buffer_len, state->buffer + state->buffer_len);
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
                    if (state->buffer[state->buffer_pos] == '-' && state->buffer[state->buffer_pos+1] == '-') {
                        state->is_last_part = true;
                    }
                } else {
                    mla_size_t remaining = state->buffer_len - state->buffer_pos;
                    for (mla_size_t i = 0; i < remaining; i++) {
                        state->buffer[i] = state->buffer[state->buffer_pos + i];
                    }
                    state->buffer_pos = 0;
                    state->buffer_len = remaining;
                    mla_size_t r = state->base_stream.read(state->base_stream, 0, sizeof(state->buffer) - state->buffer_len, state->buffer + state->buffer_len);
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

static mla_bool_t __read_line(mla_multipart_stream_state_t* state, mla_string_builder_t& sb) {
    mla_string_builder_reset(sb);
    while (true) {
        if (!__refill_buffer(state)) {
            return mla_string_builder_length(sb) > 0;
        }
        
        mla_byte_t b = state->buffer[state->buffer_pos++];
        if (b == '\n') {
            return true;
        } else if (b != '\r') {
            mla_string_builder_append(sb, (mla_char_t)b);
        }
    }
}

// Helper for extracting substring to avoid repeated logic
static mla_string_t __extract_substring(const mla_string_t& source, mla_size_t start, mla_size_t length) {
    mla_pointer_t temp = mla_create_char_array(length);
    for (mla_size_t k = 0; k < length; k++) {
        mla_pointer_get_data<mla_char_t>(temp)[k] = mla_string_data(source)[start + k];
    }
    mla_string_t result = mla_string_from_c_string(temp, length);
    return result;
}

static void __parse_content_disposition(const mla_string_t& header, mla_string_t& field_name, mla_string_t& file_name) {
    mla_size_t i = 0;
    
    mla_string_t name_key = mla_string_const("name=\"");
    mla_string_t file_key = mla_string_const("filename=\"");
    
    for (i = 0; i < mla_string_length(header); ++i) {
        if (i + mla_string_length(name_key) <= mla_string_length(header)) {
            mla_bool_t match = true;
            for (mla_size_t j = 0; j < mla_string_length(name_key); ++j) {
                if (mla_string_data(header)[i+j] != mla_string_data(name_key)[j]) { match = false; break; }
            }
            if (match) {
                mla_size_t start = i + mla_string_length(name_key);
                mla_size_t end = start;
                while (end < mla_string_length(header) && mla_string_data(header)[end] != '"') end++;
                
                field_name = __extract_substring(header, start, end - start);
            }
        }
        if (i + mla_string_length(file_key) <= mla_string_length(header)) {
            mla_bool_t match = true;
            for (mla_size_t j = 0; j < mla_string_length(file_key); ++j) {
                if (mla_string_data(header)[i+j] != mla_string_data(file_key)[j]) { match = false; break; }
            }
            if (match) {
                mla_size_t start = i + mla_string_length(file_key);
                mla_size_t end = start;
                while (end < mla_string_length(header) && mla_string_data(header)[end] != '"') end++;
                
                file_name = __extract_substring(header, start, end - start);
            }
        }
    }
}

mla_bool_t mla_http_server_parse_multipart(const mla_http_request_t& request, mla_user_data_t userdata, mla_http_multipart_part_handler_t handler) {
    mla_string_t content_type_header = mla_http_headers_get_value(request.headers, mla_string_const("Content-Type"));
    if (mla_string_is_empty(content_type_header)) return false;
    
    mla_string_t boundary_prefix = mla_string_const("boundary=");
    mla_size_t pos = 0;
    for (mla_size_t i = 0; i + mla_string_length(boundary_prefix) <= mla_string_length(content_type_header); i++) {
        mla_bool_t match = true;
        for (mla_size_t j = 0; j < mla_string_length(boundary_prefix); j++) {
            if (mla_string_data(content_type_header)[i+j] != mla_string_data(boundary_prefix)[j]) {
                match = false; break;
            }
        }
        if (match) { pos = i + mla_string_length(boundary_prefix); break; }
    }
    if (pos == 0) return false;
    
    mla_size_t bound_len = mla_string_length(content_type_header) - pos;
    mla_string_t boundary = __extract_substring(content_type_header, pos, bound_len);
    
    mla_string_builder_t tsb = mla_string_builder_create(64);
    mla_string_builder_append(tsb, mla_string_const("\r\n--"));
    mla_string_builder_append(tsb, boundary);
    mla_string_t target_boundary = mla_string_builder_to_string(tsb);
    mla_string_builder_reset(tsb);
    
    mla_multipart_stream_state_t state = { request.content, boundary, false, target_boundary, {0}, 0, 0, false, false };
    
    mla_string_builder_t sb = mla_string_builder_create(256);
    
    while (__read_line(&state, sb)) {
        mla_string_t line = mla_string_builder_to_string(sb);
        mla_bool_t is_boundary = false;
        if (mla_string_length(line) >= mla_string_length(boundary) + 2) {
            if (mla_string_data(line)[0] == '-' && mla_string_data(line)[1] == '-') {
                is_boundary = true;
                for (mla_size_t i = 0; i < mla_string_length(boundary); i++) {
                    if (mla_string_data(line)[i+2] != mla_string_data(boundary)[i]) { is_boundary = false; break; }
                }
            }
        }
        mla_string_destroy(line);
        if (is_boundary) break;
    }
    
    mla_bool_t success = true;
    
    while (!state.eof && !state.is_last_part) {
        state.hit_boundary = false;
        mla_string_t field_name = mla_string_empty();
        mla_string_t file_name = mla_string_empty();
        mla_string_t part_content_type = mla_string_empty();
        
        while (__read_line(&state, sb)) {
            if (mla_string_builder_length(sb) == 0) break; 
            
            mla_string_t line = mla_string_builder_to_string(sb);
            
            mla_string_t cd_prefix = mla_string_const("Content-Disposition:");
            mla_string_t ct_prefix = mla_string_const("Content-Type:");
            
            if (mla_string_length(line) >= mla_string_length(cd_prefix)) {
                mla_bool_t match = true;
                for (mla_size_t i = 0; i < mla_string_length(cd_prefix); i++) {
                    if (mla_string_data(line)[i] != mla_string_data(cd_prefix)[i]) { match = false; break; }
                }
                if (match) __parse_content_disposition(line, field_name, file_name);
            }
            
            if (mla_string_length(line) >= mla_string_length(ct_prefix)) {
                mla_bool_t match = true;
                for (mla_size_t i = 0; i < mla_string_length(ct_prefix); i++) {
                    if (mla_string_data(line)[i] != mla_string_data(ct_prefix)[i]) { match = false; break; }
                }
                if (match) {
                    mla_size_t start = mla_string_length(ct_prefix);
                    while (start < mla_string_length(line) && mla_string_data(line)[start] == ' ') start++;
                    
                    part_content_type = __extract_substring(line, start, mla_string_length(line) - start);
                }
            }
            
            mla_string_destroy(line);
        }
        
        mla_stream_input_t part_stream = mla_stream_noop_input();
        part_stream.userdata = mla_user_data_empty();
        part_stream.userdata.data = mla_platform_pointer_to_managed_pointer((mla_platform_pointer_t)&state);
        part_stream.read = mla_multipart_part_read;
        
        mla_http_multipart_part_t part = {
            field_name,
            file_name,
            part_content_type,
            part_stream
        };
        
        if (!handler(userdata, part)) {
            success = false;
            break;
        }
        
        if (!state.hit_boundary) {
            mla_byte_t dump[128];
            while (part_stream.read(part_stream, 0, sizeof(dump), dump) > 0);
        }
        
        mla_string_destroy(field_name);
        mla_string_destroy(file_name);
        mla_string_destroy(part_content_type);
    }
    
    mla_string_destroy(boundary);
    mla_string_destroy(target_boundary);
    mla_string_builder_reset(sb);
    return success;
}
