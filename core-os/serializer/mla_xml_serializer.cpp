//
// XML Serializer/Deserializer implementation
// Simple XML format: simple types as attributes, complex types as nested tags
//
// Created by copilot on 12/30/2025.
//

#include "mla_xml_serializer.h"

#include "../log/mla_logging.h"
#include "../system/mla_number.h"
#include "../system/mla_string.h"
#include "../system/mla_string_concat.h"

///////////////////////////////////////////////////////////////////////////////
// XML Serializer
///////////////////////////////////////////////////////////////////////////////

#define mla_bytes_prefix "mla_bytes:"

struct mla_xml_serializer_state_t {
    mla_bool_t in_open_tag;
    mla_string_t pending_property_name;
    mla_array_list_t<mla_string_t, mla_string_initializer> tag_stack; // Stack to track tag names
};

mla_xml_serializer_state_t *__mla_xml_ser_get_state(mla_serializer_t &inst) {
    return reinterpret_cast<mla_xml_serializer_state_t *>(inst.userdata);
}

mla_bool_t __mla_xml_write_str(const mla_stream_output_t &out, const mla_string_t str) {
    return mla_stream_output_write_string(out, str);
}

mla_bool_t __mla_xml_write_escaped(const mla_stream_output_t &out, const mla_string_t &str) {

    for (mla_size_t i = 0; i < str.length; i++) {
        mla_char_t c = str.data[i];
        switch (c) {
            case '<':
                if (!__mla_xml_write_str(out, mla_string_const("&lt;")))
                    return false;
                break;
            case '>':
                if (!__mla_xml_write_str(out, mla_string_const("&gt;")))
                    return false;
                break;
            case '&':
                if (!__mla_xml_write_str(out, mla_string_const("&amp;")))
                    return false;
                break;
            case '"':
                if (!__mla_xml_write_str(out, mla_string_const("&quot;")))
                    return false;
                break;
            case '\'':
                if (!__mla_xml_write_str(out, mla_string_const("&apos;")))
                    return false;
                break;
            default:
                if (out.write(out, 0, 1, reinterpret_cast<const mla_byte_t *>(&c)) != 1) return false;
                break;
        }
    }
    return true;
}

mla_bool_t __mla_xml_close_tag_if_open(mla_serializer_t &inst) {

    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);
    if (state->in_open_tag) {

        if (!__mla_xml_write_str(inst.output, mla_string_const(">")))
            return false;

        state->in_open_tag = false;
    }
    return true;
}

mla_bool_t mla_xml_serializer_write_start_struct(mla_serializer_t &inst) {

    if (!__mla_xml_close_tag_if_open(inst))
        return false;

    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);

    // Determine tag name: use property name if available, otherwise "item"
    mla_string_t tag_name = mla_string_empty();
    if (state->pending_property_name.length > 0) {
        tag_name = state->pending_property_name;
        state->pending_property_name = mla_string_empty();
    } else {
        tag_name = mla_string_const("item");
    }

    // Push tag name onto stack for closing
    mla_array_list_add(state->tag_stack, tag_name);

    // Write opening tag
    if (!__mla_xml_write_str(inst.output, mla_string_const("<")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, tag_name))
        return false;

    state->in_open_tag = true;
    return true;
}

mla_bool_t mla_xml_serializer_write_end_struct(mla_serializer_t &inst) {
    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);

    // Get the tag name from the stack
    if (mla_array_list_size(state->tag_stack) == 0) {
        return false; // Error: unmatched end_struct
    }
    mla_string_t tag_name = *mla_array_list_get_ref(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);
    mla_array_list_remove(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);

    if (state->in_open_tag) {
        // Self-closing tag
        if (!__mla_xml_write_str(inst.output, mla_string_const(" />")))
            return false;

        state->in_open_tag = false;
    } else {
        // Closing tag
        if (!__mla_xml_write_str(inst.output, mla_string_const("</")))
            return false;

        if (!__mla_xml_write_escaped(inst.output, tag_name))
            return false;

        if (!__mla_xml_write_str(inst.output, mla_string_const(">")))
            return false;
    }
    return true;
}

mla_bool_t mla_xml_serializer_write_start_list(mla_serializer_t &inst) {
    if (!__mla_xml_close_tag_if_open(inst)) return false;

    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);

    // Determine tag name: use property name if available, otherwise "list"
    mla_string_t tag_name = mla_string_empty();
    if (state->pending_property_name.length > 0) {
        tag_name = state->pending_property_name;
        state->pending_property_name = mla_string_empty();
    } else {
        tag_name = mla_string_const("list");
    }

    // Push tag name onto stack for closing
    mla_array_list_add(state->tag_stack, tag_name);

    // Write opening tag
    if (!__mla_xml_write_str(inst.output, mla_string_const("<")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, tag_name))
        return false;

    if (!__mla_xml_write_str(inst.output, mla_string_const(">")))
        return false;

    return true;
}

mla_bool_t mla_xml_serializer_write_end_list(mla_serializer_t &inst) {
    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);

    // Get the tag name from the stack
    if (mla_array_list_size(state->tag_stack) == 0) {
        return false; // Error: unmatched end_list
    }
    mla_string_t tag_name = *mla_array_list_get_ref(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);
    mla_array_list_remove(state->tag_stack, mla_array_list_size(state->tag_stack) - 1);

    // Write closing tag
    if (!__mla_xml_write_str(inst.output, mla_string_const("</")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, tag_name))
        return false;

    if (!__mla_xml_write_str(inst.output, mla_string_const(">")))
        return false;

    return true;
}

mla_bool_t mla_xml_serializer_write_property_name(mla_serializer_t &inst, const mla_string_t &name) {
    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);
    state->pending_property_name = name;
    return true;
}

static mla_bool_t __mla_xml_write_attr(mla_serializer_t &inst, const mla_string_t &value) {
    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);

    if (!__mla_xml_write_str(inst.output, mla_string_const(" ")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, state->pending_property_name))
        return false;

    if (!__mla_xml_write_str(inst.output, mla_string_const("=\"")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, value))
        return false;

    if (!__mla_xml_write_str(inst.output, mla_string_const("\"")))
        return false;

    return true;
}

mla_bool_t mla_xml_serializer_write_bool(mla_serializer_t &inst, const mla_bool_t value) {
    return __mla_xml_write_attr(inst, value ? mla_string_const("true") : mla_string_const("false"));
}

mla_bool_t mla_xml_serializer_write_int8(mla_serializer_t &inst, const mla_int8_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_int8(value));
}

mla_bool_t mla_xml_serializer_write_int16(mla_serializer_t &inst, const mla_int16_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_int16(value));
}

mla_bool_t mla_xml_serializer_write_int32(mla_serializer_t &inst, const mla_int32_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_int32(value));
}

mla_bool_t mla_xml_serializer_write_int64(mla_serializer_t &inst, const mla_int64_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_int64(value));
}

mla_bool_t mla_xml_serializer_write_uint8(mla_serializer_t &inst, const mla_uint8_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_uint8(value));
}

mla_bool_t mla_xml_serializer_write_uint16(mla_serializer_t &inst, const mla_uint16_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_uint16(value));
}

mla_bool_t mla_xml_serializer_write_uint32(mla_serializer_t &inst, const mla_uint32_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_uint32(value));
}

mla_bool_t mla_xml_serializer_write_uint64(mla_serializer_t &inst, const mla_uint64_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_uint64(value));
}

mla_bool_t mla_xml_serializer_write_float(mla_serializer_t &inst, const mla_float_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_float(value, 6));
}

mla_bool_t mla_xml_serializer_write_double(mla_serializer_t &inst, const mla_double_t value) {
    return __mla_xml_write_attr(inst, mla_string_from_double(value, 6));
}

mla_bool_t mla_xml_serializer_write_string(mla_serializer_t &inst, const mla_string_t &value) {
    return __mla_xml_write_attr(inst, value);
}

mla_bool_t mla_xml_serializer_write_bytes(mla_serializer_t &inst, const mla_bytes_t &value) {
    mla_string_t base64 = mla_bytes_to_base64(value);

    mla_xml_serializer_state_t *state = __mla_xml_ser_get_state(inst);

    if (!__mla_xml_write_str(inst.output, mla_string_const(" ")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, state->pending_property_name))
        return false;

    if (!__mla_xml_write_str(inst.output, mla_string_const("=\"")))
        return false;

    if (!__mla_xml_write_escaped(inst.output, mla_string_const(mla_bytes_prefix)))
        return false;

    if (!__mla_xml_write_escaped(inst.output, base64))
        return false;

    if (!__mla_xml_write_str(inst.output, mla_string_const("\"")))
        return false;

    return true;


    mla_bool_t result = __mla_xml_write_attr(inst, base64);
    mla_string_destroy(base64);



    return result;
}

mla_serializer_t mla_xml_serializer(const mla_stream_output_t &output) {
    // Note: state memory is allocated but framework has no destructor pattern
    // This is consistent with the overall framework design where serializers
    // are expected to be short-lived
    mla_xml_serializer_state_t *state = static_cast<mla_xml_serializer_state_t *>(mla_malloc(
        sizeof(mla_xml_serializer_state_t)));

    if (state == nullptr) {
        return mla_serializer_invalid();
    }

    mla_memset(state, 0, sizeof(mla_xml_serializer_state_t));
    state->in_open_tag = false;
    state->pending_property_name = mla_string_empty();
    state->tag_stack = mla_array_list<mla_string_t, mla_string_initializer>();

    return {
        output,
        reinterpret_cast<mla_callback_userdata>(state),
        mla_xml_serializer_write_start_struct,
        mla_xml_serializer_write_end_struct,
        mla_xml_serializer_write_start_list,
        mla_xml_serializer_write_end_list,
        mla_xml_serializer_write_property_name,
        mla_xml_serializer_write_bool,
        mla_xml_serializer_write_int8,
        mla_xml_serializer_write_int16,
        mla_xml_serializer_write_int32,
        mla_xml_serializer_write_int64,
        mla_xml_serializer_write_uint8,
        mla_xml_serializer_write_uint16,
        mla_xml_serializer_write_uint32,
        mla_xml_serializer_write_uint64,
        mla_xml_serializer_write_float,
        mla_xml_serializer_write_double,
        mla_xml_serializer_write_string,
        mla_xml_serializer_write_bytes
    };
}

///////////////////////////////////////////////////////////////////////////////
// XML Deserializer
///////////////////////////////////////////////////////////////////////////////

struct mla_xml_attr_t {
    mla_string_t name;
    mla_string_t value;

    mla_xml_attr_t() : name(mla_string_empty()), value(mla_string_empty()) {
    }

    mla_xml_attr_t(const mla_string_t &n, const mla_string_t &v) : name(n), value(v) {
    }
};

struct mla_xml_deser_state_t {
    mla_array_list_t<mla_xml_attr_t> attrs;
    mla_size_t attr_idx;
    mla_bool_t returning_attr_val;
    mla_char_t buffered_chars[16]; // Multi-character lookahead buffer
    mla_size_t buffered_count; // Number of buffered characters
    mla_string_t pending_tag_name; // Tag name waiting to be processed as property name
    mla_bool_t pending_tag_is_struct;
    mla_bool_t pending_tag_is_list;
};

static mla_xml_deser_state_t *__mla_xml_deser_get_state(mla_deserializer_t &inst) {
    return reinterpret_cast<mla_xml_deser_state_t *>(inst.userdata);
}

static mla_bool_t __mla_xml_read_char(mla_deserializer_t &inst, mla_char_t &c) {
    mla_xml_deser_state_t *state = __mla_xml_deser_get_state(inst);

    if (state->buffered_count > 0) {
        c = state->buffered_chars[--state->buffered_count];
        return true;
    }

    return inst.input.read(inst.input, 0, 1, reinterpret_cast<mla_byte_t *>(&c)) == 1;
}

static void __mla_xml_unread_char(mla_deserializer_t &inst, mla_char_t c) {
    mla_xml_deser_state_t *state = __mla_xml_deser_get_state(inst);
    if (state->buffered_count < 16) {
        state->buffered_chars[state->buffered_count++] = c;
    }
}

static void __mla_xml_skip_ws(mla_deserializer_t &inst) {
    mla_char_t c;
    while (__mla_xml_read_char(inst, c)) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            __mla_xml_unread_char(inst, c);
            break;
        }
    }
}

static mla_string_t __mla_xml_read_until(mla_deserializer_t &inst, mla_char_t stop) {
    mla_string_t result = mla_string_empty();
    mla_char_t buf[256];
    mla_size_t pos = 0;
    mla_char_t c;

    while (__mla_xml_read_char(inst, c)) {
        if (c == stop) {
            __mla_xml_unread_char(inst, c);
            break;
        }

        if (pos >= 255) {
            result = mla_string_concat(result, mla_string(buf, pos));
            pos = 0;
        }
        buf[pos++] = c;
    }

    if (pos > 0) {
        result = mla_string_concat(result, mla_string(buf, pos));
    }

    return result;
}

static mla_string_t __mla_xml_unescape(const mla_string_t &str) {
    mla_string_t result = mla_string_empty();

    for (mla_size_t i = 0; i < str.length; i++) {
        if (str.data[i] == '&') {
            if (i + 4 <= str.length && mla_memcmp(&str.data[i], "&lt;", 4) == 0) {
                result = mla_string_concat(result, mla_string_const("<"));
                i += 3;
            } else if (i + 4 <= str.length && mla_memcmp(&str.data[i], "&gt;", 4) == 0) {
                result = mla_string_concat(result, mla_string_const(">"));
                i += 3;
            } else if (i + 5 <= str.length && mla_memcmp(&str.data[i], "&amp;", 5) == 0) {
                result = mla_string_concat(result, mla_string_const("&"));
                i += 4;
            } else if (i + 6 <= str.length && mla_memcmp(&str.data[i], "&quot;", 6) == 0) {
                result = mla_string_concat(result, mla_string_const("\""));
                i += 5;
            } else if (i + 6 <= str.length && mla_memcmp(&str.data[i], "&apos;", 6) == 0) {
                result = mla_string_concat(result, mla_string_const("'"));
                i += 5;
            } else {
                result = mla_string_concat(result, mla_string(&str.data[i], 1));
            }
        } else {
            result = mla_string_concat(result, mla_string(&str.data[i], 1));
        }
    }

    return result;
}

static void __mla_xml_parse_val(mla_deserializer_t &inst, const mla_string_t &val_str) {
    if (mla_string_equals_const(val_str, "true")) {
        inst.current_token.type = MLA_DESERIALIZER_VALUE_BOOL;
        inst.current_token.simple.bool_value = true;
    } else if (mla_string_equals_const(val_str, "false")) {
        inst.current_token.type = MLA_DESERIALIZER_VALUE_BOOL;
        inst.current_token.simple.bool_value = false;
    } else {
        mla_int64_t int_val;
        mla_double_t dbl_val;

        mla_string_t byte_prefix = mla_string_const(mla_bytes_prefix);

        if (mla_parse_int64(val_str, int_val)) {
            if (int_val >= mla_int8_min && int_val <= mla_int8_max) {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT8;
                inst.current_token.simple.int8_value = static_cast<mla_int8_t>(int_val);
            } else if (int_val >= mla_int16_min && int_val <= mla_int16_max) {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT16;
                inst.current_token.simple.int16_value = static_cast<mla_int16_t>(int_val);
            } else if (int_val >= mla_int32_min && int_val <= mla_int32_max) {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT32;
                inst.current_token.simple.int32_value = static_cast<mla_int32_t>(int_val);
            } else {
                inst.current_token.type = MLA_DESERIALIZER_VALUE_INT64;
                inst.current_token.simple.int64_value = int_val;
            }
        } else if (mla_parse_double(val_str, dbl_val)) {
            inst.current_token.type = MLA_DESERIALIZER_VALUE_DOUBLE;
            inst.current_token.simple.double_value = dbl_val;
        } else if (mla_string_starts_with(val_str, byte_prefix)) {
            mla_string_t base64Part = mla_string_substr(val_str, mla_string_length(byte_prefix));
            inst.current_token.type = MLA_DESERIALIZER_VALUE_BYTES;
            inst.current_token.complex.bytes_value = mla_bytes_from_base64(base64Part);
            mla_string_destroy(base64Part);
        } else {
            inst.current_token.type = MLA_DESERIALIZER_VALUE_STRING;
            inst.current_token.complex.string_value = val_str;
        }
    }
}

mla_bool_t mla_xml_deserializer_read_next(mla_deserializer_t &inst) {
    mla_xml_deser_state_t *state = __mla_xml_deser_get_state(inst);

    inst.current_token.complex = {mla_string_empty(), mla_string_empty(), mla_bytes_empty()};
    inst.current_token.simple = {0};

    // If we have a pending property tag, return the struct/list start now
    if (state->pending_tag_is_struct || state->pending_tag_is_list) {
        if (state->pending_tag_is_struct) {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_START;
            state->attr_idx = 0;
            state->pending_tag_is_struct = false;
        } else {
            inst.current_token.type = MLA_DESERIALIZER_LIST_START;
            state->pending_tag_is_list = false;
        }
        state->pending_tag_name = mla_string_empty();
        return true;
    }

    // If we just returned a property name from attribute, return the value
    if (state->returning_attr_val) {
        mla_xml_attr_t *attr = mla_array_list_get_ref(state->attrs, state->attr_idx - 1);
        mla_string_t unesc = __mla_xml_unescape(attr->value);
        __mla_xml_parse_val(inst, unesc);
        state->returning_attr_val = false;
        return true;
    }

    // Check if we have more attributes to process
    if (state->attr_idx < mla_array_list_size(state->attrs)) {
        mla_xml_attr_t *attr = mla_array_list_get_ref(state->attrs, state->attr_idx);
        state->attr_idx++;

        inst.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
        inst.current_token.complex.property_name = attr->name;
        state->returning_attr_val = true;
        return true;
    }

    __mla_xml_skip_ws(inst);

    mla_char_t c;
    if (!__mla_xml_read_char(inst, c))
        return false;

    if (c != '<')
        return false;

    if (!__mla_xml_read_char(inst, c))
        return false;

    if (c == '/') {
        // End tag
        mla_string_t tag_name = __mla_xml_read_until(inst, '>');
        __mla_xml_read_char(inst, c); // consume '>'

        if (mla_string_equals_const(tag_name, "item")) {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_END;
        } else if (mla_string_equals_const(tag_name, "list")) {
            inst.current_token.type = MLA_DESERIALIZER_LIST_END;
        } else {
            // Closing tag for a property - treat as struct/list end
            // We can't easily distinguish, so assume struct
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_END;
        }

        return true;
    }

    __mla_xml_unread_char(inst, c);

    // Read tag name
    mla_string_t tag_name = mla_string_empty(); {
        mla_char_t buf[256];
        mla_size_t pos = 0;

        while (__mla_xml_read_char(inst, c)) {
            if (c == ' ' || c == '>' || c == '/') {
                __mla_xml_unread_char(inst, c);
                break;
            }
            if (pos >= 255) {
                tag_name = mla_string_concat(tag_name, mla_string(buf, pos));
                pos = 0;
            }
            buf[pos++] = c;
        }
        if (pos > 0) {
            tag_name = mla_string_concat(tag_name, mla_string(buf, pos));
        }
    }

    // Parse attributes
    mla_array_list_clear(state->attrs);

    while (true) {
        __mla_xml_skip_ws(inst);

        if (!__mla_xml_read_char(inst, c))
            break;

        if (c == '>' || c == '/') {
            __mla_xml_unread_char(inst, c);
            break;
        }

        __mla_xml_unread_char(inst, c);

        mla_string_t attr_name = __mla_xml_read_until(inst, '=');
        __mla_xml_read_char(inst, c); // consume '='

        __mla_xml_skip_ws(inst);
        __mla_xml_read_char(inst, c); // quote

        mla_string_t attr_val = __mla_xml_read_until(inst, c);
        __mla_xml_read_char(inst, c); // consume quote

        mla_xml_attr_t attr(attr_name, attr_val);
        mla_array_list_add(state->attrs, attr);
    }

    if (!__mla_xml_read_char(inst, c))
        return false;

    if (c == '/') {
        // Self-closing tag
        __mla_xml_read_char(inst, c); // consume '>'

        if (mla_string_equals_const(tag_name, "item")) {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_START;
            state->attr_idx = 0;
        } else {
            // Property tag for a struct (self-closing means it's a struct with attributes only)
            inst.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
            inst.current_token.complex.property_name = tag_name;
            state->pending_tag_is_struct = true;
            state->pending_tag_name = tag_name;
        }

        return true;
    } else if (c == '>') {
        // Opening tag - need to determine if next content indicates a list or struct
        if (mla_string_equals_const(tag_name, "item")) {
            inst.current_token.type = MLA_DESERIALIZER_STRUCT_START;
            state->attr_idx = 0;
        } else if (mla_string_equals_const(tag_name, "list")) {
            inst.current_token.type = MLA_DESERIALIZER_LIST_START;
        } else {
            // Property tag - return the property name now
            inst.current_token.type = MLA_DESERIALIZER_PROPERTY_NAME;
            inst.current_token.complex.property_name = tag_name;

            // Peek ahead to determine if it's a list (next tag is <item>) or struct
            __mla_xml_skip_ws(inst);

            mla_char_t peek_chars[5];
            mla_size_t peek_count = 0;
            mla_bool_t is_list = false;

            // Try to read "<item"
            if (__mla_xml_read_char(inst, peek_chars[0])) {
                peek_count = 1;
                if (peek_chars[0] == '<' && __mla_xml_read_char(inst, peek_chars[1])) {
                    peek_count = 2;
                    if (peek_chars[1] == 'i' && __mla_xml_read_char(inst, peek_chars[2])) {
                        peek_count = 3;
                        if (peek_chars[2] == 't' && __mla_xml_read_char(inst, peek_chars[3])) {
                            peek_count = 4;
                            if (peek_chars[3] == 'e' && __mla_xml_read_char(inst, peek_chars[4])) {
                                peek_count = 5;
                                if (peek_chars[4] == 'm') {
                                    is_list = true;
                                }
                            }
                        }
                    }
                }
            }

            // Unread all peeked characters in reverse order
            for (mla_size_t i = peek_count; i > 0; i--) {
                __mla_xml_unread_char(inst, peek_chars[i - 1]);
            }

            if (is_list) {
                state->pending_tag_is_list = true;
            } else {
                state->pending_tag_is_struct = true;
            }
            state->pending_tag_name = tag_name;
        }

        return true;
    }

    return false;
}

mla_deserializer_t mla_xml_deserializer(const mla_stream_input_t &input) {
    // Note: state memory is allocated but framework has no destructor pattern
    // This is consistent with the overall framework design where deserializers
    // are expected to be short-lived
    mla_xml_deser_state_t *state = static_cast<mla_xml_deser_state_t *>(mla_malloc(sizeof(mla_xml_deser_state_t)));

    if (state == nullptr) {
        return mla_deserializer_invalid();
    }

    mla_memset(state, 0, sizeof(mla_xml_deser_state_t));

    state->attrs = mla_array_list<mla_xml_attr_t>();
    state->attr_idx = 0;
    state->returning_attr_val = false;
    state->buffered_count = 0;
    state->pending_tag_name = mla_string_empty();
    state->pending_tag_is_struct = false;
    state->pending_tag_is_list = false;

    return {
        input,
        reinterpret_cast<mla_callback_userdata>(state),
        {MLA_DESERIALIZER_NULL, {mla_string_empty(), mla_string_empty(), mla_bytes_empty()}, {0}},
        mla_xml_deserializer_read_next
    };
}
