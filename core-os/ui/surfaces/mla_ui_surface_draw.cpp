#include "mla_ui_surface_draw.h"

mla_ui_surface_draw_command_polyline_t mla_ui_surface_draw_command_polyline_empty() {
    return { mla_array_list_empty<mla_ui_surface_draw_point_t>(), {0,0,0,0}, {0,0,0,0}, 0.0 };
}

mla_ui_surface_draw_command_polygon_t mla_ui_surface_draw_command_polygon_empty() {
    return { mla_array_list_empty<mla_ui_surface_draw_point_t>(), {0,0,0,0}, {0,0,0,0}, 0.0 };
}

mla_ui_surface_draw_command_path_t mla_ui_surface_draw_command_path_empty() {
    return { mla_array_list_empty<mla_ui_surface_draw_path_command_t>(), {0,0,0,0}, {0,0,0,0}, 0 };
}

mla_ui_surface_draw_command_text_t mla_ui_surface_draw_command_text_empty() {
    return { 0.0, 0.0, mla_string_empty(), mla_string_empty(), 0, {0,0,0,0} };
}

mla_ui_surface_draw_command_t mla_ui_surface_draw_command_empty() {
    return  {MLA_UI_SURFACE_DRAW_COMMAND_KIND_NONE, {{0, 0, 0, 0, 0, 0, {0, 0, 0, 0}, {0, 0, 0, 0}, 0}}, mla_ui_surface_draw_command_text_empty(), mla_ui_surface_draw_command_path_empty(), mla_ui_surface_draw_command_polyline_empty(), mla_ui_surface_draw_command_polygon_empty() };
}

void mla_ui_surface_draw_commands_scale(mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_double_t scaleX, mla_double_t scaleY) {

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        mla_ui_surface_draw_command_t& command = mla_array_list_get_unsafe(drawCommands, i);
        mla_ui_surface_draw_command_scale(command, scaleX, scaleY);
    }

}


void mla_ui_surface_draw_command_scale(mla_ui_surface_draw_command_t& command, mla_double_t scaleX, mla_double_t scaleY) {

    switch (command.kind) {
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT:
            command.rect.x *= scaleX;
            command.rect.y *= scaleY;
            command.rect.width *= scaleX;
            command.rect.height *= scaleY;
            command.rect.rx *= scaleX;
            command.rect.ry *= scaleY;
            command.rect.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE:
            command.circle.cx *= scaleX;
            command.circle.cy *= scaleY;
            command.circle.r *= (scaleX + scaleY) / 2.0;
            command.circle.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE:
            command.ellipse.cx *= scaleX;
            command.ellipse.cy *= scaleY;
            command.ellipse.rx *= scaleX;
            command.ellipse.ry *= scaleY;
            command.ellipse.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE:
            command.line.x1 *= scaleX;
            command.line.y1 *= scaleY;
            command.line.x2 *= scaleX;
            command.line.y2 *= scaleY;
            command.line.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE:
            for (mla_size_t i = 0; i < mla_array_list_size(command.polyline.points); ++i) {
                mla_ui_surface_draw_point_t& p = mla_array_list_get_unsafe(command.polyline.points, i);
                p.x *= scaleX;
                p.y *= scaleY;
            }
            command.polyline.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON:
            for (mla_size_t i = 0; i < mla_array_list_size(command.polygon.points); ++i) {
                mla_ui_surface_draw_point_t& p = mla_array_list_get_unsafe(command.polygon.points, i);
                p.x *= scaleX;
                p.y *= scaleY;
            }
            command.polygon.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH:
            for (mla_size_t i = 0; i < mla_array_list_size(command.path.commands); ++i) {
                mla_ui_surface_draw_path_command_t& cmd = mla_array_list_get_unsafe(command.path.commands, i);
                switch (cmd.kind) {
                    case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                        cmd.move_to.x *= scaleX;
                        cmd.move_to.y *= scaleY;
                        break;
                    case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                        cmd.line_to.x *= scaleX;
                        cmd.line_to.y *= scaleY;
                        break;
                    case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO:
                        cmd.quadratic_curve_to.cpx *= scaleX;
                        cmd.quadratic_curve_to.cpy *= scaleY;
                        cmd.quadratic_curve_to.x *= scaleX;
                        cmd.quadratic_curve_to.y *= scaleY;
                        break;
                    case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO:
                        cmd.cubic_curve_to.cp1x *= scaleX;
                        cmd.cubic_curve_to.cp1y *= scaleY;
                        cmd.cubic_curve_to.cp2x *= scaleX;
                        cmd.cubic_curve_to.cp2y *= scaleY;
                        cmd.cubic_curve_to.x *= scaleX;
                        cmd.cubic_curve_to.y *= scaleY;
                        break;
                    case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO:
                        cmd.arc_to.x *= scaleX;
                        cmd.arc_to.y *= scaleY;
                        cmd.arc_to.rx *= scaleX;
                        cmd.arc_to.ry *= scaleY;
                        break;
                    default:
                        break;
                }
            }
            command.path.stroke_width *= (scaleX + scaleY) / 2.0;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT:
            command.text.x *= scaleX;
            command.text.y *= scaleY;
            command.text.font_size *= scaleY;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT:
            command.linear_gradient.x1 *= scaleX;
            command.linear_gradient.y1 *= scaleY;
            command.linear_gradient.x2 *= scaleX;
            command.linear_gradient.y2 *= scaleY;
            break;
        case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT:
            command.radial_gradient.cx *= scaleX;
            command.radial_gradient.cy *= scaleY;
            command.radial_gradient.r *= (scaleX + scaleY) / 2.0;
            command.radial_gradient.fx *= scaleX;
            command.radial_gradient.fy *= scaleY;
            break;
        default:
            break;
    }
}
