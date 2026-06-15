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



mla_ui_control_t mla_ui_window() {

    mla_ui_control_t window = mla_ui_control();
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
        mla_ui_control_get_value_as_pointer(window, mla_string_const("on_close"), nullptr)
    );
}

mla_bool_t mla_ui_window_set_close(mla_ui_control_t &window, mla_ui_window_on_close_callback_t callback) {
    return mla_ui_control_set_value_as_pointer(
        window,
        mla_string_const("on_close"),
        reinterpret_cast<mla_platform_pointer_t>(callback)
    );
}
