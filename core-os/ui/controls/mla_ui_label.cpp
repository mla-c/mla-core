//
// Created by chris on 1/19/2026.
//

#include "mla_ui_label.h"
#include "mla_ui_style.h"

mla_string_t __mla_ui_label_resolve_fill(mla_ui_text_kind_t kind) {
    switch (kind) {
        case MLA_UI_TEXT_KIND_PRIMARY:
            return mla_string_const(MLA_UI_COLOR_TEXT_PRIMARY);
        case MLA_UI_TEXT_KIND_SECONDARY:
            return mla_string_const(MLA_UI_COLOR_TEXT_SECONDARY);
        case MLA_UI_TEXT_KIND_DISABLED:
            return mla_string_const(MLA_UI_COLOR_TEXT_DISABLED);
        case MLA_UI_TEXT_KIND_LINK:
            return mla_string_const(MLA_UI_COLOR_TEXT_LINK);
        default:
            return mla_string_const(MLA_UI_COLOR_TEXT_PRIMARY);
    }
}

mla_bool_t __mla_ui_label_render_to_svg(const mla_ui_control_context_t &context,
                                       const mla_ui_control_t &element,
                                       const mla_stream_output_t& output) {

    // Resolve position and size (same approach as window: 0 means "use remaining context")
    mla_size_t x = element.layout.x;
    mla_size_t y = element.layout.y;

    mla_size_t w = element.layout.width;
    if (w == 0) {
        // If x is absolute, this mirrors the window behavior.
        w = context.width - x;
    }

    mla_size_t h = element.layout.height;
    if (h == 0) {
        h = context.height - y;
    }

    const mla_string_t text = mla_ui_label_get_text(element);
    if (mla_string_is_empty(text)) {
        return true; // Nothing to render is not an error
    }

    const mla_uint16_t fontSize = mla_ui_label_get_font_size(element);
    const mla_ui_text_kind_t kind = mla_ui_label_get_text_kind(element);
    const mla_string_t fill = __mla_ui_label_resolve_fill(kind);

    // Baseline inside the label box (simple single-line baseline)
    const mla_size_t baselineY = y + static_cast<mla_size_t>(fontSize);

    mla_bool_t ok = true;

    // <defs> with clipPath to ensure text does not overflow the label box
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<defs>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<clipPath id=\"mla_lbl_clip_"));
    ok &= mla_ui_control_svg_write_raw_string(output, element.id);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\">"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<rect x=\""));
    ok &= mla_ui_control_svg_write_uint32(output, x);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y=\""));
    ok &= mla_ui_control_svg_write_uint32(output, y);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" width=\""));
    ok &= mla_ui_control_svg_write_uint32(output, w);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" height=\""));
    ok &= mla_ui_control_svg_write_uint32(output, h);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\"/>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</clipPath>"));
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</defs>"));

    // Group wrapper applying clip
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<g clip-path=\"url(#mla_lbl_clip_"));
    ok &= mla_ui_control_svg_write_raw_string(output, element.id);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const(")\">"));

    // Text
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<text x=\""));
    ok &= mla_ui_control_svg_write_uint32(output, x);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y=\""));
    ok &= mla_ui_control_svg_write_uint32(output, baselineY);

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" fill=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, fill);

    ok &= mla_ui_control_svg_write_raw_string(
        output,
        mla_string_const("\" font-family=\"" MLA_UI_FONT_FAMILY_DEFAULT "\" font-size=\"")
    );
    ok &= mla_ui_control_svg_write_uint16(output, fontSize);

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" font-weight=\""));
    ok &= mla_ui_control_svg_write_uint16(output, MLA_UI_FONT_WEIGHT_SEMIBOLD);

    // Keep it single-line; overflow is handled via clipPath
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" xml:space=\"preserve\">"));

    ok &= mla_ui_control_svg_write_escaped_text(output, text);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</text>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</g>"));

    // No children for a label
    return ok;
}

mla_ui_control_t mla_ui_label() {

    mla_ui_control_t label = mla_ui_control();
    label.renderToSvg = __mla_ui_label_render_to_svg;
    return label;
}

mla_string_t mla_ui_label_get_text(const mla_ui_control_t &label) {
    return mla_ui_control_get_value_as_string(label, mla_string_const("text"), mla_string_empty());
}
mla_bool_t mla_ui_label_set_text(mla_ui_control_t &label, const mla_string& text) {
    return mla_ui_control_set_value_as_string(label, mla_string_const("text"), text);
}

mla_uint16_t mla_ui_label_get_font_size(const mla_ui_control_t &label) {
    return static_cast<mla_uint16_t>(mla_ui_control_get_value_as_uint16(label, mla_string_const("font_size"), MLA_UI_FONT_SIZE_DEFAULT));
}

mla_bool_t mla_ui_label_set_font_size(mla_ui_control_t &label, mla_uint16_t fontSize) {
    return mla_ui_control_set_value_as_uint16(label, mla_string_const("font_size"), fontSize);
}

mla_ui_text_kind_t mla_ui_label_get_text_kind(const mla_ui_control_t &label) {
    return static_cast<mla_ui_text_kind_t>(mla_ui_control_get_value_as_uint8(label, mla_string_const("text_kind"), MLA_UI_TEXT_KIND_PRIMARY));
}

mla_bool_t mla_ui_label_set_text_kind(mla_ui_control_t &label, mla_ui_text_kind_t kind) {
    return mla_ui_control_set_value_as_uint8(label, mla_string_const("text_kind"), static_cast<mla_uint8_t>(kind));
}