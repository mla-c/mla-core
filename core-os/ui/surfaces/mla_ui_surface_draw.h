//
// Created by chris on 1/22/2026.
//

#ifndef COREOS_MLA_UI_SURFACE_DRAW_H
#define COREOS_MLA_UI_SURFACE_DRAW_H

#include "../../mla_data_types.h"
#include "../../system/mla_string.h"
#include "../../system/mla_array_list.h"

struct mla_ui_surface_draw_command_color_t {
    mla_uint8_t r;
    mla_uint8_t g;
    mla_uint8_t b;
    mla_uint8_t a;
};

struct mla_ui_surface_draw_point_t {
    mla_double_t x;
    mla_double_t y;
};

struct mla_ui_surface_draw_size_t {
    mla_double_t width;
    mla_double_t height;
};

struct mla_ui_surface_input_states_t {
    mla_ui_surface_draw_point_t cursorPosition;
    mla_bool_t leftMouseButtonDown;
    mla_bool_t rightMouseButtonDown;
    mla_bool_t middleMouseButtonDown;
    mla_bool_t shiftKeyDown;
    mla_bool_t ctrlKeyDown;
    mla_bool_t altKeyDown;
    mla_bool_t metaKeyDown;
    mla_uint32_t keyCodeDown;
};

mla_ui_surface_input_states_t mla_ui_surface_input_states_empty();

enum mla_ui_surface_draw_command_kind: mla_uint8_t {
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_NONE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT,
    MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP
};

struct mla_ui_surface_draw_command_rect_t {
    mla_double_t x;
    mla_double_t y;
    mla_double_t width;
    mla_double_t height;
    mla_double_t rx;
    mla_double_t ry;
    mla_ui_surface_draw_command_color_t color; // acts as fill
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

struct mla_ui_surface_draw_command_circle_t {
    mla_double_t cx;
    mla_double_t cy;
    mla_double_t r;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

struct mla_ui_surface_draw_command_ellipse_t {
    mla_double_t cx;
    mla_double_t cy;
    mla_double_t rx;
    mla_double_t ry;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

struct mla_ui_surface_draw_command_line_t {
    mla_double_t x1;
    mla_double_t y1;
    mla_double_t x2;
    mla_double_t y2;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

struct mla_ui_surface_draw_command_polyline_t {
    mla_array_list_t<mla_ui_surface_draw_point_t> points;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

mla_ui_surface_draw_command_polyline_t mla_ui_surface_draw_command_polyline_empty();

struct mla_ui_surface_draw_command_polyline_initializer_t {
    static mla_ui_surface_draw_command_polyline_t init() {
        return mla_ui_surface_draw_command_polyline_empty();
    }
};

struct mla_ui_surface_draw_command_polygon_t {
    mla_array_list_t<mla_ui_surface_draw_point_t> points;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

mla_ui_surface_draw_command_polygon_t mla_ui_surface_draw_command_polygon_empty();

struct mla_ui_surface_draw_command_polygon_initializer_t {
    static mla_ui_surface_draw_command_polygon_t init() {
        return mla_ui_surface_draw_command_polygon_empty();
    }
};

// Path Command Structures

enum mla_ui_surface_draw_path_command_kind_t: mla_uint8_t {
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO = 0,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO,
    MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH
};

struct mla_ui_surface_draw_path_command_move_to_t {
    mla_double_t x;
    mla_double_t y;
};

struct mla_ui_surface_draw_path_command_line_to_t {
    mla_double_t x;
    mla_double_t y;
};

struct mla_ui_surface_draw_path_command_quadratic_curve_to_t {
    mla_double_t cpx;
    mla_double_t cpy;
    mla_double_t x;
    mla_double_t y;
};

struct mla_ui_surface_draw_path_command_cubic_curve_to_t {
    mla_double_t cp1x;
    mla_double_t cp1y;
    mla_double_t cp2x;
    mla_double_t cp2y;
    mla_double_t x;
    mla_double_t y;
};

struct mla_ui_surface_draw_path_command_arc_to_t {
    mla_double_t rx;
    mla_double_t ry;
    mla_double_t x_axis_rotation;
    mla_bool_t large_arc_flag;
    mla_bool_t sweep_flag;
    mla_double_t x;
    mla_double_t y;
};

struct mla_ui_surface_draw_path_command_t {
    mla_ui_surface_draw_path_command_kind_t kind;

    union {
        mla_ui_surface_draw_path_command_move_to_t move_to;
        mla_ui_surface_draw_path_command_line_to_t line_to;
        mla_ui_surface_draw_path_command_quadratic_curve_to_t quadratic_curve_to;
        mla_ui_surface_draw_path_command_cubic_curve_to_t cubic_curve_to;
        mla_ui_surface_draw_path_command_arc_to_t arc_to;
        // Close path takes no parameters
    };
};

struct mla_ui_surface_draw_command_path_t {
    mla_array_list_t<mla_ui_surface_draw_path_command_t> commands;
    mla_ui_surface_draw_command_color_t fill;
    mla_ui_surface_draw_command_color_t stroke;
    mla_double_t stroke_width;
};

mla_ui_surface_draw_command_path_t mla_ui_surface_draw_command_path_empty();

struct mla_ui_surface_draw_command_path_initializer_t {
    static mla_ui_surface_draw_command_path_t init() {
        return mla_ui_surface_draw_command_path_empty();
    }
};

struct mla_ui_surface_draw_command_text_t {
    mla_double_t x;
    mla_double_t y;
    mla_string_t content;
    mla_string_t font_family;
    mla_double_t font_size;
    mla_ui_surface_draw_command_color_t fill;
};

mla_ui_surface_draw_command_text_t mla_ui_surface_draw_command_text_empty();

struct mla_ui_surface_draw_command_text_initializer_t {
    static mla_ui_surface_draw_command_text_t init() {
        return mla_ui_surface_draw_command_text_empty();
    }
};

struct mla_ui_surface_draw_command_linear_gradient_t {
    mla_double_t x1;
    mla_double_t y1;
    mla_double_t x2;
    mla_double_t y2;
};

struct mla_ui_surface_draw_command_radial_gradient_t {
    mla_double_t cx;
    mla_double_t cy;
    mla_double_t r;
    mla_double_t fx;
    mla_double_t fy;
};

struct mla_ui_surface_draw_command_stop_t {
    mla_double_t offset;
    mla_ui_surface_draw_command_color_t stop_color;
    mla_double_t stop_opacity;
};

struct mla_ui_surface_draw_command_t {
    mla_ui_surface_draw_command_kind kind;

    union {
        mla_ui_surface_draw_command_rect_t rect;
        mla_ui_surface_draw_command_circle_t circle;
        mla_ui_surface_draw_command_ellipse_t ellipse;
        mla_ui_surface_draw_command_line_t line;
        mla_ui_surface_draw_command_linear_gradient_t linear_gradient;
        mla_ui_surface_draw_command_radial_gradient_t radial_gradient;
        mla_ui_surface_draw_command_stop_t stop;
    };

    mla_ui_surface_draw_command_text_t text;
    mla_ui_surface_draw_command_path_t path;
    mla_ui_surface_draw_command_polyline_t polyline;
    mla_ui_surface_draw_command_polygon_t polygon;
};

mla_ui_surface_draw_command_t mla_ui_surface_draw_command_empty();

struct mla_ui_surface_draw_command_initializer_t {
    static mla_ui_surface_draw_command_t init() {
        return  mla_ui_surface_draw_command_empty();
    }
};

void mla_ui_surface_draw_commands_scale(mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_double_t scaleX, mla_double_t scaleY);
void mla_ui_surface_draw_command_scale(mla_ui_surface_draw_command_t& command, mla_double_t scaleX, mla_double_t scaleY);

mla_bool_t mla_ui_surface_parse_color_from_hex_string(const mla_string_t& colorStr, mla_ui_surface_draw_command_color_t& outColor);

#endif
