#include "mla_ui_window.h"
#include "../../system/mla_string.h"
#include "../../system/mla_stream.h"
#include "mla_ui_style.h"

// Style constants (moveable to a shared header later)
#define MLA_UI_WINDOW_RADIUS                 8
#define MLA_UI_WINDOW_BORDER_WIDTH           1

#define MLA_UI_WINDOW_BG_COLOR               "#FFFFFF"
#define MLA_UI_WINDOW_BORDER_COLOR           "rgba(0,0,0,0.08)"
#define MLA_UI_WINDOW_SHADOW1_COLOR          "rgba(0,0,0,0.10)"
#define MLA_UI_WINDOW_SHADOW2_COLOR          "rgba(0,0,0,0.06)"

#define MLA_UI_WINDOW_SHADOW_FILTER_X        MLA_UI_SVG_SHADOW_FILTER_X
#define MLA_UI_WINDOW_SHADOW_FILTER_Y        MLA_UI_SVG_SHADOW_FILTER_Y
#define MLA_UI_WINDOW_SHADOW_FILTER_W        MLA_UI_SVG_SHADOW_FILTER_W
#define MLA_UI_WINDOW_SHADOW_FILTER_H        MLA_UI_SVG_SHADOW_FILTER_H
#define MLA_UI_WINDOW_SHADOW_FILTER_CIF      MLA_UI_SVG_SHADOW_FILTER_CIF

#define MLA_UI_WINDOW_SHADOW1_DX             0
#define MLA_UI_WINDOW_SHADOW1_DY             4
#define MLA_UI_WINDOW_SHADOW1_STDDEV         8

#define MLA_UI_WINDOW_SHADOW2_DX             0
#define MLA_UI_WINDOW_SHADOW2_DY             1
#define MLA_UI_WINDOW_SHADOW2_STDDEV         2

#define MLA_UI_WINDOW_HEADER_HEIGHT          44
#define MLA_UI_WINDOW_HEADER_BG_COLOR        "rgba(0,0,0,0.03)"
#define MLA_UI_WINDOW_HEADER_SEP_COLOR       "rgba(0,0,0,0.06)"

#define MLA_UI_WINDOW_TITLE_PADDING_X        16
#define MLA_UI_WINDOW_TITLE_BASELINE_Y       28
#define MLA_UI_WINDOW_TITLE_FILL             MLA_UI_COLOR_TEXT_PRIMARY
#define MLA_UI_WINDOW_TITLE_FONT_FAMILY      MLA_UI_FONT_FAMILY_DEFAULT
#define MLA_UI_WINDOW_TITLE_FONT_SIZE        MLA_UI_FONT_SIZE_DEFAULT
#define MLA_UI_WINDOW_TITLE_FONT_WEIGHT      MLA_UI_FONT_WEIGHT_SEMIBOLD

#define MLA_UI_WINDOW_CLOSE_STROKE_COLOR     "rgba(0,0,0,0.70)"
#define MLA_UI_WINDOW_CLOSE_SIZE             28
#define MLA_UI_WINDOW_CLOSE_PAD_RIGHT        10
#define MLA_UI_WINDOW_CLOSE_STROKE_WIDTH     2
#define MLA_UI_WINDOW_CLOSE_RX               6
#define MLA_UI_WINDOW_CLOSE_RY               6
#define MLA_UI_WINDOW_CLOSE_HIT_FILL         "rgba(0,0,0,0.0)"
#define MLA_UI_WINDOW_CLOSE_GLYPH_PAD        9

mla_bool_t __mla_ui_window_render_to_svg(const mla_ui_control_context_t &context,
                                        const mla_ui_control_t &element,
                                        const mla_stream_output_t& output) {

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

    const mla_uint8_t radius = MLA_UI_WINDOW_RADIUS;
    const mla_uint8_t borderWidth = MLA_UI_WINDOW_BORDER_WIDTH;
    const mla_string_t bg = mla_string_const(MLA_UI_WINDOW_BG_COLOR);
    const mla_string_t border = mla_string_const(MLA_UI_WINDOW_BORDER_COLOR);
    const mla_string_t shadow1 = mla_string_const(MLA_UI_WINDOW_SHADOW1_COLOR);
    const mla_string_t shadow2 = mla_string_const(MLA_UI_WINDOW_SHADOW2_COLOR);

    // Header + button styling
    const mla_size_t headerHeight = MLA_UI_WINDOW_HEADER_HEIGHT;
    const mla_string_t headerBg = mla_string_const(MLA_UI_WINDOW_HEADER_BG_COLOR);
    const mla_string_t headerSep = mla_string_const(MLA_UI_WINDOW_HEADER_SEP_COLOR);
    const mla_string_t closeStroke = mla_string_const(MLA_UI_WINDOW_CLOSE_STROKE_COLOR);
    const mla_size_t closeSize = MLA_UI_WINDOW_CLOSE_SIZE;
    const mla_size_t closePaddingRight = MLA_UI_WINDOW_CLOSE_PAD_RIGHT;
    const mla_size_t closeStrokeWidth = MLA_UI_WINDOW_CLOSE_STROKE_WIDTH;

    mla_bool_t ok = true;

    // <defs> with clipPath and shadow filter
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<defs>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<clipPath id=\"mla_win_clip_"));
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
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" rx=\""));
    ok &= mla_ui_control_svg_write_uint8(output, radius);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" ry=\""));
    ok &= mla_ui_control_svg_write_uint8(output, radius);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\"/></clipPath>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<filter id=\"mla_win_shadow_"));
    ok &= mla_ui_control_svg_write_raw_string(output, element.id);
    ok &= mla_ui_control_svg_write_raw_string(
        output,
        mla_string_const(
            "\" x=\"" MLA_UI_WINDOW_SHADOW_FILTER_X
            "\" y=\"" MLA_UI_WINDOW_SHADOW_FILTER_Y
            "\" width=\"" MLA_UI_WINDOW_SHADOW_FILTER_W
            "\" height=\"" MLA_UI_WINDOW_SHADOW_FILTER_H
            "\" color-interpolation-filters=\"" MLA_UI_WINDOW_SHADOW_FILTER_CIF "\">")
    );

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<feDropShadow dx=\"0\" dy=\"4\" stdDeviation=\"8\" flood-color=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, shadow1);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\"/>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<feDropShadow dx=\"0\" dy=\"1\" stdDeviation=\"2\" flood-color=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, shadow2);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\"/>"));

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</filter>"));
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</defs>"));

    // Group wrapper
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<g clip-path=\"url(#mla_win_clip_"));
    ok &= mla_ui_control_svg_write_raw_string(output, element.id);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const(")\">"));

    // Background
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<rect x=\""));
    ok &= mla_ui_control_svg_write_uint32(output, x);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y=\""));
    ok &= mla_ui_control_svg_write_uint32(output, y);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" width=\""));
    ok &= mla_ui_control_svg_write_uint32(output, w);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" height=\""));
    ok &= mla_ui_control_svg_write_uint32(output, h);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" rx=\""));
    ok &= mla_ui_control_svg_write_uint8(output, radius);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" ry=\""));
    ok &= mla_ui_control_svg_write_uint8(output, radius);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" fill=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, bg);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, border);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke-width=\""));
    ok &= mla_ui_control_svg_write_uint8(output, borderWidth);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" filter=\"url(#mla_win_shadow_"));
    ok &= mla_ui_control_svg_write_raw_string(output, element.id);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const(")\"/>"));

    // Header bar
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<rect x=\""));
    ok &= mla_ui_control_svg_write_uint32(output, x);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y=\""));
    ok &= mla_ui_control_svg_write_uint32(output, y);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" width=\""));
    ok &= mla_ui_control_svg_write_uint32(output, w);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" height=\""));
    ok &= mla_ui_control_svg_write_uint32(output, headerHeight);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" fill=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, headerBg);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\"/>"));

    // Header bottom separator
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<line x1=\""));
    ok &= mla_ui_control_svg_write_uint32(output, x);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y1=\""));
    ok &= mla_ui_control_svg_write_uint32(output, y + headerHeight);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" x2=\""));
    ok &= mla_ui_control_svg_write_uint32(output, x + w);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y2=\""));
    ok &= mla_ui_control_svg_write_uint32(output, y + headerHeight);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke=\""));
    ok &= mla_ui_control_svg_write_raw_string(output, headerSep);
    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke-width=\"1\"/>"));

    // Optional title
    const mla_string_t title = mla_ui_window_get_title(element);
    if (!mla_string_is_empty(title)) {
        const mla_size_t paddingX = MLA_UI_WINDOW_TITLE_PADDING_X;
        const mla_size_t titleBaselineY = MLA_UI_WINDOW_TITLE_BASELINE_Y;

        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<text x=\""));
        ok &= mla_ui_control_svg_write_uint32(output, x + paddingX);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y=\""));
        ok &= mla_ui_control_svg_write_uint32(output, y + titleBaselineY);
        ok &= mla_ui_control_svg_write_raw_string(
            output,
            mla_string_const(
                "\" fill=\"" MLA_UI_WINDOW_TITLE_FILL
                "\" font-family=\"" MLA_UI_WINDOW_TITLE_FONT_FAMILY
                "\" font-size=\"14\" font-weight=\"600\">")
        );
        ok &= mla_ui_control_svg_write_escaped_text(output, title);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</text>"));
    }

    // Close button (only if callback is defined)
    if (mla_ui_window_get_close(element) != nullptr) {
        const mla_size_t bx = x + w - closePaddingRight - closeSize;
        const mla_size_t by = y + (headerHeight - closeSize) / 2;

        // Button hit area (transparent)
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<rect x=\""));
        ok &= mla_ui_control_svg_write_uint32(output, bx);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y=\""));
        ok &= mla_ui_control_svg_write_uint32(output, by);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" width=\""));
        ok &= mla_ui_control_svg_write_uint32(output, closeSize);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" height=\""));
        ok &= mla_ui_control_svg_write_uint32(output, closeSize);
        ok &= mla_ui_control_svg_write_raw_string(
            output,
            mla_string_const("\" rx=\"6\" ry=\"6\" fill=\"" MLA_UI_WINDOW_CLOSE_HIT_FILL "\"/>")
        );

        // X glyph
        const mla_size_t pad = MLA_UI_WINDOW_CLOSE_GLYPH_PAD;
        const mla_size_t x1 = bx + pad;
        const mla_size_t y1 = by + pad;
        const mla_size_t x2 = bx + closeSize - pad;
        const mla_size_t y2 = by + closeSize - pad;

        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<line x1=\""));
        ok &= mla_ui_control_svg_write_uint32(output, x1);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y1=\""));
        ok &= mla_ui_control_svg_write_uint32(output, y1);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" x2=\""));
        ok &= mla_ui_control_svg_write_uint32(output, x2);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y2=\""));
        ok &= mla_ui_control_svg_write_uint32(output, y2);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke=\""));
        ok &= mla_ui_control_svg_write_raw_string(output, closeStroke);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke-width=\""));
        ok &= mla_ui_control_svg_write_uint32(output, closeStrokeWidth);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke-linecap=\"round\"/>"));

        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("<line x1=\""));
        ok &= mla_ui_control_svg_write_uint32(output, x1);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y1=\""));
        ok &= mla_ui_control_svg_write_uint32(output, y2);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" x2=\""));
        ok &= mla_ui_control_svg_write_uint32(output, x2);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" y2=\""));
        ok &= mla_ui_control_svg_write_uint32(output, y1);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke=\""));
        ok &= mla_ui_control_svg_write_raw_string(output, closeStroke);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke-width=\""));
        ok &= mla_ui_control_svg_write_uint32(output, closeStrokeWidth);
        ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("\" stroke-linecap=\"round\"/>"));
    }

    // Child render does not return an error -- ignore it.
    mla_ui_control_render_children_to_svg(context, element, output);

    ok &= mla_ui_control_svg_write_raw_string(output, mla_string_const("</g>"));
    return ok;
}

mla_ui_control_t mla_ui_window() {

    mla_ui_control_t window = mla_ui_control();
    window.renderToSvg = __mla_ui_window_render_to_svg;
    return window;
}

mla_string_t mla_ui_window_get_title(const mla_ui_control_t &window) {
    return mla_ui_control_get_value_as_string(window, mla_string_const("title"), mla_string_empty());
}

mla_bool_t mla_ui_window_set_title(mla_ui_control_t &window, const mla_string_t &title) {
    return mla_ui_control_set_value_as_string(window, mla_string_const("title"), title);
}

mla_bool_t mla_ui_window_get_resizable(const mla_ui_control_t &window) {
    return mla_ui_control_get_value_as_bool(window, mla_string_const("resizable"), true);
}

mla_bool_t mla_ui_window_set_resizable(mla_ui_control_t &window, mla_bool_t resizable) {
    return mla_ui_control_set_value_as_bool(window, mla_string_const("resizable"), resizable);
}

mla_ui_window_on_close_callback_t mla_ui_window_get_close(const mla_ui_control_t &window) {

    return reinterpret_cast<mla_ui_window_on_close_callback_t>(
        mla_ui_control_get_value_as_uint64(window, mla_string_const("on_close"), 0)
    );
}

mla_bool_t mla_ui_window_set_close(mla_ui_control_t &window, mla_ui_window_on_close_callback_t callback) {
    return mla_ui_control_set_value_as_uint64(
        window,
        mla_string_const("on_close"),
        reinterpret_cast<mla_uint64_t>(callback)
    );
}
