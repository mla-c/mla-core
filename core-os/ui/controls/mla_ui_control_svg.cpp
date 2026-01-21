//
// Created by chris on 1/19/2026.
//

#include "mla_ui_control.h"

mla_bool_t mla_ui_control_render_to_svg(const mla_ui_control_context_t &context, const mla_ui_control_t &control, const mla_stream_output_t& output) {

    //<svg width="420" height="380" viewBox="0 0 420 380"

    mla_stream_output_write_string(output, mla_string_const("<svg width="));
    mla_stream_output_write_string(output, mla_string_from_int32(context.width));
    mla_stream_output_write_string(output, mla_string_const(" height="));
    mla_stream_output_write_string(output, mla_string_from_int32(context.height));
    mla_stream_output_write_string(output, mla_string_const(" viewBox=\"0 0 "));
    mla_stream_output_write_string(output, mla_string_from_int32(context.width));
    mla_stream_output_write_string(output, mla_string_const(" "));;
    mla_stream_output_write_string(output, mla_string_from_int32(context.height));
    mla_stream_output_write_string(output, mla_string_const("\" xmlns=\"http://www.w3.org/2000/svg\">\n"));

    if (control.renderToSvg != nullptr) {

        if (!control.renderToSvg(context, control, output)) {
            return false;
        }
    }

    mla_stream_output_write_string(output, mla_string_const("</svg>\n"));
    return true;

}

mla_bool_t mla_ui_control_svg_write_raw_string(const mla_stream_output_t& output, const mla_string_t& text) {
    return mla_stream_output_write_string(output, text);
}

mla_bool_t mla_ui_control_svg_write_uint8(const mla_stream_output_t& output, mla_uint8_t value) {

    mla_string_t tmp = mla_string_from_uint64(static_cast<mla_uint64_t>(value));
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_int8(const mla_stream_output_t& output, mla_int8_t value) {

    mla_string_t tmp = mla_string_from_int64(static_cast<mla_int64_t>(value));
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_uint16(const mla_stream_output_t& output, mla_uint16_t value) {

    mla_string_t tmp = mla_string_from_uint64(static_cast<mla_uint64_t>(value));
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_int16(const mla_stream_output_t& output, mla_int16_t value) {

    mla_string_t tmp = mla_string_from_int64(static_cast<mla_int64_t>(value));
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_uint32(const mla_stream_output_t& output, mla_uint32_t value) {

    mla_string_t tmp = mla_string_from_uint64(static_cast<mla_uint64_t>(value));
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_int32(const mla_stream_output_t& output, mla_int32_t value) {

    mla_string_t tmp = mla_string_from_int64(static_cast<mla_int64_t>(value));
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_uint64(const mla_stream_output_t& output, mla_uint64_t value) {

    mla_string_t tmp = mla_string_from_uint64(value);
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_int64(const mla_stream_output_t& output, mla_int64_t value) {

    mla_string_t tmp = mla_string_from_int64(value);
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_float(const mla_stream_output_t& output, mla_float_t value) {

    mla_string_t tmp = mla_string_from_float(value, 2);
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_double(const mla_stream_output_t& output, mla_double_t value) {

    mla_string_t tmp = mla_string_from_double(value, 2);
    mla_bool_t result = mla_stream_output_write_string(output, tmp);
    mla_string_destroy(tmp);
    return result;
}

mla_bool_t mla_ui_control_svg_write_escaped_text(const mla_stream_output_t& output, const mla_string_t& text) {

    const mla_char_t* data = mla_string_data(text);
    const mla_size_t len = mla_string_length(text);

    for (mla_size_t i = 0; i < len; i++) {
        const mla_char_t c = data[i];
        switch (c) {
            case '&': mla_stream_output_write_string(output, mla_string_const("&amp;")); break;
            case '<': mla_stream_output_write_string(output, mla_string_const("&lt;")); break;
            case '>': mla_stream_output_write_string(output, mla_string_const("&gt;")); break;
            case '\"': mla_stream_output_write_string(output, mla_string_const("&quot;")); break;
            case '\'': mla_stream_output_write_string(output, mla_string_const("&apos;")); break;
            default: {
                mla_byte_t b = static_cast<mla_byte_t>(c);
                output.write(output, 0, 1, &b);
                break;
            }
        }
    }
    return true;
}

mla_bool_t mla_ui_control_svg_write_bool(const mla_stream_output_t& output, mla_bool_t value) {
    if (value) {
        return mla_stream_output_write_string(output, mla_string_const("true"));
    } else {
        return mla_stream_output_write_string(output, mla_string_const("false"));
    }
}