//
// Created by chris on 1/21/2026.
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"
#include <windows.h>

struct mla_windows_window_surface_t {
    HWND hwnd;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;
};

// Stub implementations for the surface function pointers
mla_ui_surface_size_t __windows_surface_get_size(const mla_ui_surface_t& surface) {

    mla_ui_surface_size_t size = {0, 0};

    mla_windows_window_surface_t* window_surface = static_cast<mla_windows_window_surface_t*>(surface.resource);

    if (window_surface == nullptr) {
        return size;
    }

    if (!window_surface->is_initialized) {
        return window_surface->default_size;
    }

    if (!IsWindow(window_surface->hwnd)) {
        return size;
    }

    RECT rect;

    if (GetClientRect(window_surface->hwnd, &rect)) {
        size.width = (mla_uint32_t)(rect.right - rect.left);
        size.height = (mla_uint32_t)(rect.bottom - rect.top);
    }

    return size;
}

mla_bool_t __windows_surface_set_size(mla_ui_surface_t& surface, mla_ui_surface_size_t size) {

    mla_windows_window_surface_t* window_surface = static_cast<mla_windows_window_surface_t*>(surface.resource);

    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized) {
        window_surface->default_size = size;
        return true;
    }

    if (!IsWindow(window_surface->hwnd)) {
        return false;
    }

    if (SetWindowPos(window_surface->hwnd, nullptr, 0, 0, (int)size.width, (int)size.height, SWP_NOMOVE | SWP_NOZORDER)) {;
        return true;
    }

    return false;

}

LRESULT CALLBACK __windows_surface_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

mla_bool_t __windows_create_windows_surface(mla_windows_window_surface_t* surface) {

    const char CLASS_NAME[] = "MLA_Window_Class";

    WNDCLASSEXA wc = { };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = __windows_surface_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);

    // Register class, ignoring error if already registered
    RegisterClassExA(&wc);

    int width = surface->default_size.width;

    if (width == 0) {
        width = CW_USEDEFAULT;
    }

    int height = surface->default_size.height;

    if (height == 0) {
        height = CW_USEDEFAULT;
    }

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "MLA App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr
    );

    if (hwnd == nullptr) {
        return false; // False
    }

    surface->is_initialized = true;
    surface->hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    return true; // True

}


mla_bool_t __windows_surface_render_draw_commands(mla_ui_surface_t& surface, const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands) {

    mla_windows_window_surface_t* window_surface = static_cast<mla_windows_window_surface_t*>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized) {
        if (!__windows_create_windows_surface(window_surface)) {
            mla_warning("Failed to initialize Windows UI surface for SVG rendering.");
            return false;
        }
    }

    MSG msg;
    while (PeekMessage(&msg, window_surface->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    HDC hdc = GetDC(window_surface->hwnd);
    if (!hdc) {
        return false;
    }

    // Clear background
    RECT clientRect;
    GetClientRect(window_surface->hwnd, &clientRect);
    HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    // Sc

    // Process each draw command
    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {

        const mla_ui_surface_draw_command_t& cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {

                const auto& rect = cmd.rect;
                HBRUSH fillBrush = CreateSolidBrush(RGB(rect.color.r, rect.color.g, rect.color.b));
                HPEN strokePen = CreatePen(PS_SOLID, (int)rect.stroke_width, RGB(rect.stroke.r, rect.stroke.g, rect.stroke.b));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);

                if (rect.rx > 0 || rect.ry > 0) {
                    RoundRect(hdc, (int)rect.x, (int)rect.y,
                              (int)(rect.x + rect.width), (int)(rect.y + rect.height),
                              (int)(rect.rx * 2), (int)(rect.ry * 2));
                } else {
                    Rectangle(hdc, (int)rect.x, (int)rect.y,
                              (int)(rect.x + rect.width), (int)(rect.y + rect.height));
                }

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(fillBrush);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto& circle = cmd.circle;
                HBRUSH fillBrush = CreateSolidBrush(RGB(circle.fill.r, circle.fill.g, circle.fill.b));
                HPEN strokePen = CreatePen(PS_SOLID, (int)circle.stroke_width, RGB(circle.stroke.r, circle.stroke.g, circle.stroke.b));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);

                Ellipse(hdc, (int)(circle.cx - circle.r), (int)(circle.cy - circle.r),
                        (int)(circle.cx + circle.r), (int)(circle.cy + circle.r));

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(fillBrush);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto& ellipse = cmd.ellipse;
                HBRUSH fillBrush = CreateSolidBrush(RGB(ellipse.fill.r, ellipse.fill.g, ellipse.fill.b));
                HPEN strokePen = CreatePen(PS_SOLID, (int)ellipse.stroke_width, RGB(ellipse.stroke.r, ellipse.stroke.g, ellipse.stroke.b));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);

                Ellipse(hdc, (int)(ellipse.cx - ellipse.rx), (int)(ellipse.cy - ellipse.ry),
                        (int)(ellipse.cx + ellipse.rx), (int)(ellipse.cy + ellipse.ry));

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(fillBrush);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto& line = cmd.line;
                HPEN strokePen = CreatePen(PS_SOLID, (int)line.stroke_width, RGB(line.stroke.r, line.stroke.g, line.stroke.b));
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);

                MoveToEx(hdc, (int)line.x1, (int)line.y1, nullptr);
                LineTo(hdc, (int)line.x2, (int)line.y2);

                SelectObject(hdc, oldPen);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto& polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;

                HPEN strokePen = CreatePen(PS_SOLID, (int)polyline.stroke_width, RGB(polyline.stroke.r, polyline.stroke.g, polyline.stroke.b));
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

                POINT* points = (POINT*)mla_malloc(sizeof(POINT) * mla_array_list_size(polyline.points));
                for (mla_size_t j = 0; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t& point = mla_array_list_get_unsafe(polyline.points, j);
                    points[j].x = (LONG)point.x;
                    points[j].y = (LONG)point.y;
                }
                Polyline(hdc, points, (int)mla_array_list_size(polyline.points));
                mla_free(points);

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto& polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;

                HBRUSH fillBrush = CreateSolidBrush(RGB(polygon.fill.r, polygon.fill.g, polygon.fill.b));
                HPEN strokePen = CreatePen(PS_SOLID, (int)polygon.stroke_width, RGB(polygon.stroke.r, polygon.stroke.g, polygon.stroke.b));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);

                POINT* points = (POINT*)mla_malloc(sizeof(POINT) * mla_array_list_size(polygon.points));
                for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                    const mla_ui_surface_draw_point_t& point = mla_array_list_get_unsafe(polygon.points, j);
                    points[j].x = (LONG)point.x;
                    points[j].y = (LONG)point.y;
                }
                Polygon(hdc, points, (int)mla_array_list_size(polygon.points));
                mla_free(points);

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(fillBrush);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto& path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) break;

                HBRUSH fillBrush = CreateSolidBrush(RGB(path.fill.r, path.fill.g, path.fill.b));
                HPEN strokePen = CreatePen(PS_SOLID, (int)path.stroke_width, RGB(path.stroke.r, path.stroke.g, path.stroke.b));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
                HPEN oldPen = (HPEN)SelectObject(hdc, strokePen);

                BeginPath(hdc);
                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto& pathCmd = mla_array_list_get_unsafe(path.commands, j);
                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            MoveToEx(hdc, (int)pathCmd.move_to.x, (int)pathCmd.move_to.y, nullptr);
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            LineTo(hdc, (int)pathCmd.line_to.x, (int)pathCmd.line_to.y);
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO: {
                            POINT pts[3];
                            POINT currentPos;
                            GetCurrentPositionEx(hdc, &currentPos);
                            // Approximate quadratic with cubic (GDI only has cubic bezier)
                            pts[0].x = (LONG)(currentPos.x + 2.0/3.0 * (pathCmd.quadratic_curve_to.cpx - currentPos.x));
                            pts[0].y = (LONG)(currentPos.y + 2.0/3.0 * (pathCmd.quadratic_curve_to.cpy - currentPos.y));
                            pts[1].x = (LONG)(pathCmd.quadratic_curve_to.x + 2.0/3.0 * (pathCmd.quadratic_curve_to.cpx - pathCmd.quadratic_curve_to.x));
                            pts[1].y = (LONG)(pathCmd.quadratic_curve_to.y + 2.0/3.0 * (pathCmd.quadratic_curve_to.cpy - pathCmd.quadratic_curve_to.y));
                            pts[2].x = (LONG)pathCmd.quadratic_curve_to.x;
                            pts[2].y = (LONG)pathCmd.quadratic_curve_to.y;
                            PolyBezierTo(hdc, pts, 3);
                            break;
                        }
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO: {
                            POINT pts[3];
                            pts[0].x = (LONG)pathCmd.cubic_curve_to.cp1x;
                            pts[0].y = (LONG)pathCmd.cubic_curve_to.cp1y;
                            pts[1].x = (LONG)pathCmd.cubic_curve_to.cp2x;
                            pts[1].y = (LONG)pathCmd.cubic_curve_to.cp2y;
                            pts[2].x = (LONG)pathCmd.cubic_curve_to.x;
                            pts[2].y = (LONG)pathCmd.cubic_curve_to.y;
                            PolyBezierTo(hdc, pts, 3);
                            break;
                        }
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO:
                            // GDI arc approximation - simplified
                            LineTo(hdc, (int)pathCmd.arc_to.x, (int)pathCmd.arc_to.y);
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            CloseFigure(hdc);
                            break;
                    }
                }
                EndPath(hdc);
                StrokeAndFillPath(hdc);

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(fillBrush);
                DeleteObject(strokePen);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                const auto& text = cmd.text;
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(text.fill.r, text.fill.g, text.fill.b));

                mla_c_string_t c_font_family = mla_string_to_cString(text.font_family);

                if (c_font_family.c_str == nullptr) {
                    break;
                }

                HFONT hFont = CreateFontA((int)text.font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                          ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
                                          c_font_family.c_str);
                HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

                TextOutA(hdc, (int)text.x, (int)text.y, mla_string_data(text.content), (int)mla_string_length(text.content));

                SelectObject(hdc, oldFont);
                DeleteObject(hFont);

                mla_destroy_c_string(c_font_family);
                break;
            }

            default:
                // Gradients and stops would require more complex GDI+ implementation
                break;
        }
    }

    ReleaseDC(window_surface->hwnd, hdc);
    return true;
}


mla_bool_t __windows_surface_render_svg(mla_ui_surface_t& surface, mla_string_t svgContent) {

    mla_windows_window_surface_t* window_surface = static_cast<mla_windows_window_surface_t*>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized) {

        if (!__windows_create_windows_surface(window_surface)) {
            mla_warning("Failed to initialize Windows UI surface for SVG rendering.");
            return false;
        }
    }


    MSG msg;
    // PeekMessage loop handles input but returns immediately if empty
    while (PeekMessage(&msg, window_surface->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    (void)svgContent;

    // Basic GDI Implementation to replace the "Not implemented yet" stub
    // This allows visual verification that the window loop is working.
    HDC hdc = GetDC(window_surface->hwnd);
    if (hdc) {
        // --- FPS Calculation Start ---
        static DWORD last_time = 0;
        static int frames_accumulated = 0;
        static int current_fps = 0;

        DWORD current_time = GetTickCount();
        if (last_time == 0) {
            last_time = current_time;
        }

        frames_accumulated++;

        // If 1 second (1000ms) has passed, update FPS and reset
        if (current_time - last_time >= 1000) {
            current_fps = frames_accumulated;
            frames_accumulated = 0;
            last_time = current_time;
        }
        // --- FPS Calculation End ---

        RECT rect;
        GetClientRect(window_surface->hwnd, &rect);

        // Fill background (Matching #282c34 from your SVG)
        HBRUSH bgBrush = CreateSolidBrush(RGB(40, 44, 52));
        FillRect(hdc, &rect, bgBrush);
        DeleteObject(bgBrush);

        // Draw Text (Matching #61dafb)
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(97, 218, 251));

        // Use a larger font for visibility
        HFONT hFont = CreateFontA(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, "Arial");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        // Format the string to include the FPS
        char buffer[256];
        wsprintfA(buffer, "Main Application Window - FPS: %d", current_fps);

        DrawTextA(hdc, buffer, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        ReleaseDC(window_surface->hwnd, hdc);
    }

    return true;
}

mla_buffer_cleanup_mode __windows_surface_buffer_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;

    mla_windows_window_surface_t* window_surface = static_cast<mla_windows_window_surface_t*>(data);
    if (window_surface != nullptr) {

        if (IsWindow(window_surface->hwnd)) {
            DestroyWindow(window_surface->hwnd);
        }
    }

    return CLEAN_UP_NEEDED;
}

mla_bool_t __windows_create_surface(mla_ui_surface_t& outSurface) {

    mla_windows_window_surface_t* window_surface = static_cast<mla_windows_window_surface_t*>(mla_malloc(sizeof(mla_windows_window_surface_t)));

    if (window_surface == nullptr) {
        return false; // False
    }

    mla_memset(window_surface, 0, sizeof(mla_windows_window_surface_t));
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};

    // Populate the surface structure
    outSurface.resource = window_surface;
    outSurface.resourceOwner = mla_buffer_reference(window_surface, true, __windows_surface_buffer_cleanup);
    outSurface.get_size = __windows_surface_get_size;
    outSurface.set_size = __windows_surface_set_size;
    outSurface.render_svg = __windows_surface_render_svg;
    outSurface.render_draw_commands = __windows_surface_render_draw_commands;

    return true; // True
}

mla_ui_surface_low_level_access_t g_ui_surface_low_level_access = {
    __windows_create_surface
};

#endif