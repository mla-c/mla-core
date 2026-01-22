//
// Created by chris on 1/21/2026.
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"
#include <windows.h>
#include <gdiplus.h>


using namespace Gdiplus;

// Global GDI+ token
static ULONG_PTR g_gdiplusToken = 0;

// Initialize GDI+ (call once at startup)
void __windows_gdiplus_init() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);
}

// Cleanup GDI+ (call at shutdown)
void __windows_gdiplus_shutdown() {
    Gdiplus::GdiplusShutdown(g_gdiplusToken);
}

struct MlaGDIPlusAutoInit {
    MlaGDIPlusAutoInit() { __windows_gdiplus_init(); }
    ~MlaGDIPlusAutoInit() { __windows_gdiplus_shutdown(); }
};

// Single global instance
static MlaGDIPlusAutoInit g_mlaGDIAutoInit;


struct mla_windows_window_surface_t {
    HWND hwnd;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;

    // Cached rendering resources
    HDC memDC;
    HBITMAP memBitmap;
    Graphics *graphics;
    int cached_width;
    int cached_height;

#ifdef mla_debug
    DWORD last_fps_time;
    int frames_accumulated;
    int current_fps;
#endif
};

// Stub implementations for the surface function pointers
mla_ui_surface_size_t __windows_surface_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};

    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(surface.resource);

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
        size.width = (mla_uint32_t) (rect.right - rect.left);
        size.height = (mla_uint32_t) (rect.bottom - rect.top);
    }

    return size;
}

mla_bool_t __windows_surface_set_size(mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(surface.resource);

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

    if (SetWindowPos(window_surface->hwnd, nullptr, 0, 0, (int) size.width, (int) size.height,
                     SWP_NOMOVE | SWP_NOZORDER)) {
        ;
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

mla_bool_t __windows_create_windows_surface(mla_windows_window_surface_t *surface) {
    const char CLASS_NAME[] = "MLA_Window_Class";

    WNDCLASSEXA wc = {};
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

// Helper to convert color struct to GDI+ Color
Color __convert_color(const mla_ui_surface_draw_command_color_t &color) {
    return Color(color.a, color.r, color.g, color.b);
}

mla_bool_t __windows_surface_render_draw_commands(mla_ui_surface_t &surface,
                                                  const mla_array_list_t<mla_ui_surface_draw_command_t,
                                                      mla_ui_surface_draw_command_initializer_t> &drawCommands) {
    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }

    if (!window_surface->is_initialized) {
        if (!__windows_create_windows_surface(window_surface)) {
            mla_warning("Failed to initialize Windows UI surface for drawing.");
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

    RECT clientRect;
    GetClientRect(window_surface->hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // Check if we need to recreate the bitmap (size changed)
    if (window_surface->memBitmap == nullptr || window_surface->cached_width != width || window_surface->cached_height != height) {
        // Cleanup old resources
        if (window_surface->graphics)
            delete window_surface->graphics;
        if (window_surface->memBitmap)
            DeleteObject(window_surface->memBitmap);
        if (window_surface->memDC)
            DeleteDC(window_surface->memDC);

        // Create new resources
        window_surface->memDC = CreateCompatibleDC(hdc);
        window_surface->memBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(window_surface->memDC, window_surface->memBitmap);

        window_surface->graphics = new Graphics(window_surface->memDC);
        // Set rendering hints once
        window_surface->graphics->SetSmoothingMode(SmoothingModeAntiAlias);
        window_surface->graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        window_surface->graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        window_surface->graphics->SetPixelOffsetMode(PixelOffsetModeHighQuality);
        window_surface->graphics->SetCompositingQuality(CompositingQualityHighQuality);

        window_surface->cached_width = width;
        window_surface->cached_height = height;
    }

    Graphics *graphics = window_surface->graphics;


    // Clear background to memory DC
    SolidBrush bgBrush(Color(255, 255, 255));
    graphics->FillRectangle(&bgBrush, 0, 0, width, height);

    // Track gradient state
    LinearGradientBrush *currentLinearGradient = nullptr;
    PathGradientBrush *currentRadialGradient = nullptr;

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                SolidBrush fillBrush(__convert_color(rect.color));
                Pen strokePen(__convert_color(rect.stroke), (REAL) rect.stroke_width);

                if (rect.rx > 0 || rect.ry > 0) {
                    GraphicsPath path;
                    REAL x = (REAL) rect.x;
                    REAL y = (REAL) rect.y;
                    REAL w = (REAL) rect.width;
                    REAL h = (REAL) rect.height;
                    REAL rx = (REAL) rect.rx;
                    REAL ry = (REAL) rect.ry;

                    path.AddArc(x, y, rx * 2, ry * 2, 180, 90);
                    path.AddArc(x + w - rx * 2, y, rx * 2, ry * 2, 270, 90);
                    path.AddArc(x + w - rx * 2, y + h - ry * 2, rx * 2, ry * 2, 0, 90);
                    path.AddArc(x, y + h - ry * 2, rx * 2, ry * 2, 90, 90);
                    path.CloseFigure();

                    graphics->FillPath(&fillBrush, &path);
                    graphics->DrawPath(&strokePen, &path);
                } else {
                    graphics->FillRectangle(&fillBrush, (REAL) rect.x, (REAL) rect.y, (REAL) rect.width,
                                           (REAL) rect.height);
                    graphics->DrawRectangle(&strokePen, (REAL) rect.x, (REAL) rect.y, (REAL) rect.width,
                                           (REAL) rect.height);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;
                SolidBrush fillBrush(__convert_color(circle.fill));
                Pen strokePen(__convert_color(circle.stroke), (REAL) circle.stroke_width);

                REAL diameter = (REAL) (circle.r * 2);
                graphics->FillEllipse(&fillBrush, (REAL) (circle.cx - circle.r), (REAL) (circle.cy - circle.r), diameter,
                                     diameter);
                graphics->DrawEllipse(&strokePen, (REAL) (circle.cx - circle.r), (REAL) (circle.cy - circle.r), diameter,
                                     diameter);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipse = cmd.ellipse;
                SolidBrush fillBrush(__convert_color(ellipse.fill));
                Pen strokePen(__convert_color(ellipse.stroke), (REAL) ellipse.stroke_width);

                graphics->FillEllipse(&fillBrush, (REAL) (ellipse.cx - ellipse.rx), (REAL) (ellipse.cy - ellipse.ry),
                                     (REAL) (ellipse.rx * 2), (REAL) (ellipse.ry * 2));
                graphics->DrawEllipse(&strokePen, (REAL) (ellipse.cx - ellipse.rx), (REAL) (ellipse.cy - ellipse.ry),
                                     (REAL) (ellipse.rx * 2), (REAL) (ellipse.ry * 2));
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                Pen strokePen(__convert_color(line.stroke), (REAL) line.stroke_width);
                graphics->DrawLine(&strokePen, (REAL) line.x1, (REAL) line.y1, (REAL) line.x2, (REAL) line.y2);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;

                PointF *points = new PointF[mla_array_list_size(polyline.points)];
                for (mla_size_t j = 0; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polyline.points, j);
                    points[j] = PointF((REAL) point.x, (REAL) point.y);
                }
                Pen strokePen(__convert_color(polyline.stroke), (REAL) polyline.stroke_width);
                graphics->DrawLines(&strokePen, points, (INT) mla_array_list_size(polyline.points));
                delete[] points;
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;

                PointF *points = new PointF[mla_array_list_size(polygon.points)];
                for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                    points[j] = PointF((REAL) point.x, (REAL) point.y);
                }
                SolidBrush fillBrush(__convert_color(polygon.fill));
                Pen strokePen(__convert_color(polygon.stroke), (REAL) polygon.stroke_width);
                graphics->FillPolygon(&fillBrush, points, (INT) mla_array_list_size(polygon.points));
                graphics->DrawPolygon(&strokePen, points, (INT) mla_array_list_size(polygon.points));
                delete[] points;
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) break;

                GraphicsPath gdiPath;
                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);
                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            gdiPath.StartFigure();
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            gdiPath.AddLine((REAL) pathCmd.line_to.x, (REAL) pathCmd.line_to.y,
                                            (REAL) pathCmd.line_to.x, (REAL) pathCmd.line_to.y);
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO: {
                            PointF ctrlPt((REAL) pathCmd.quadratic_curve_to.cpx, (REAL) pathCmd.quadratic_curve_to.cpy);
                            PointF endPt((REAL) pathCmd.quadratic_curve_to.x, (REAL) pathCmd.quadratic_curve_to.y);
                            PointF lastPt;
                            gdiPath.GetLastPoint(&lastPt);
                            gdiPath.AddBezier(lastPt, ctrlPt, ctrlPt, endPt);
                            break;
                        }
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO: {
                            PointF startPt;
                            gdiPath.GetLastPoint(&startPt);
                            gdiPath.AddBezier(startPt,
                                              PointF((REAL) pathCmd.cubic_curve_to.cp1x,
                                                     (REAL) pathCmd.cubic_curve_to.cp1y),
                                              PointF((REAL) pathCmd.cubic_curve_to.cp2x,
                                                     (REAL) pathCmd.cubic_curve_to.cp2y),
                                              PointF((REAL) pathCmd.cubic_curve_to.x, (REAL) pathCmd.cubic_curve_to.y));
                            break;
                        }
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            gdiPath.CloseFigure();
                            break;
                        default:
                            break;
                    }
                }
                SolidBrush fillBrush(__convert_color(path.fill));
                Pen strokePen(__convert_color(path.stroke), (REAL) path.stroke_width);
                graphics->FillPath(&fillBrush, &gdiPath);
                graphics->DrawPath(&strokePen, &gdiPath);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content))
                    break;

                if (mla_string_is_empty(cmd.text.font_family))
                    break;

                // Convert font family to UTF-16
                mla_string_utf16_buffer_t fontFamilyWide = mla_string_to_utf16_buffer(cmd.text.font_family);
                FontFamily fontFamily((const WCHAR *) fontFamilyWide.data);
                Font font(&fontFamily, (REAL) cmd.text.font_size, FontStyleRegular, UnitPixel);
                SolidBrush textBrush(__convert_color(cmd.text.fill));
                PointF origin((REAL) cmd.text.x, (REAL) cmd.text.y);

                // Convert content to UTF-16
                mla_string_utf16_buffer_t contentWide = mla_string_to_utf16_buffer(cmd.text.content);
                graphics->DrawString((const WCHAR *) contentWide.data, -1, &font, origin, &textBrush);

                // Cleanup
                mla_string_utf16_buffer_destroy(fontFamilyWide);
                mla_string_utf16_buffer_destroy(contentWide);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT: {
                const auto &gradient = cmd.linear_gradient;
                if (currentLinearGradient) delete currentLinearGradient;

                PointF pt1((REAL) gradient.x1, (REAL) gradient.y1);
                PointF pt2((REAL) gradient.x2, (REAL) gradient.y2);

                // Create gradient with default colors (will be updated by stop commands)
                currentLinearGradient = new LinearGradientBrush(pt1, pt2,
                                                                Color(255, 0, 0, 0), Color(255, 255, 255, 255));
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT: {
                const auto &gradient = cmd.radial_gradient;
                if (currentRadialGradient) delete currentRadialGradient;

                // Create a circular path for the radial gradient
                GraphicsPath path;
                REAL diameter = (REAL) (gradient.r * 2);
                path.AddEllipse((REAL) (gradient.cx - gradient.r),
                                (REAL) (gradient.cy - gradient.r),
                                diameter, diameter);

                currentRadialGradient = new PathGradientBrush(&path);
                currentRadialGradient->SetCenterPoint(PointF((REAL) gradient.cx, (REAL) gradient.cy));
                currentRadialGradient->SetCenterColor(Color(255, 255, 255, 255));

                // Set default surrounding colors
                Color surroundColors[] = {Color(255, 0, 0, 0)};
                INT count = 1;
                currentRadialGradient->SetSurroundColors(surroundColors, &count);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP: {
                const auto &stop = cmd.stop;
                if (currentLinearGradient) {
                    Color *colors = new Color[1];
                    colors[0] = __convert_color(stop.stop_color);
                    REAL *positions = new REAL[1];
                    positions[0] = (REAL) stop.offset;
                    currentLinearGradient->SetInterpolationColors(colors, positions, 1);
                    delete[] colors;
                    delete[] positions;
                }
                break;
            }

            default:
                break;
        }
    }

    // Cleanup gradients
    if (currentLinearGradient != nullptr)
        delete currentLinearGradient;
    if (currentRadialGradient != nullptr)
        delete currentRadialGradient;

    // FPS Counter (debug only)
#ifdef mla_debug
    DWORD current_time = GetTickCount();
    if (window_surface->last_fps_time == 0) {
        window_surface->last_fps_time = current_time;
    }

    window_surface->frames_accumulated++;

    if (current_time - window_surface->last_fps_time >= 1000) {
        window_surface->current_fps = window_surface->frames_accumulated;
        window_surface->frames_accumulated = 0;
        window_surface->last_fps_time = current_time;
    }

    // Draw FPS counter to memory DC before blitting
    HFONT hFont = CreateFontA(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                              DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT hOldFont = (HFONT) SelectObject(window_surface->memDC, hFont);
    SetBkMode(window_surface->memDC, TRANSPARENT);
    SetTextColor(window_surface->memDC, RGB(255, 0, 0));

    char buffer[64];
    wsprintfA(buffer, "FPS: %d", window_surface->current_fps);

    SIZE textSize;
    GetTextExtentPoint32A(window_surface->memDC, buffer, lstrlenA(buffer), &textSize);

    int padding = 10;
    int x = width - textSize.cx - padding;
    int y = height - textSize.cy - padding;

    TextOutA(window_surface->memDC, x, y, buffer, lstrlenA(buffer));

    SelectObject(window_surface->memDC, hOldFont);
    DeleteObject(hFont);
#endif

    // Single blit from memory to screen - fast and flicker-free
    BitBlt(hdc, 0, 0, width, height, window_surface->memDC, 0, 0, SRCCOPY);

    // Cleanup
    ReleaseDC(window_surface->hwnd, hdc);
    return true;
}

mla_buffer_cleanup_mode __windows_surface_buffer_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;

    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(data);
    if (window_surface != nullptr) {
        if (IsWindow(window_surface->hwnd)) {
            DestroyWindow(window_surface->hwnd);
        }

        if (window_surface->graphics)
            delete window_surface->graphics;

        if (window_surface->memBitmap)

            DeleteObject(window_surface->memBitmap);
        if (window_surface->memDC)

            DeleteDC(window_surface->memDC);
    }

    return CLEAN_UP_NEEDED;
}

mla_bool_t __windows_create_surface(mla_ui_surface_t &outSurface) {
    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(mla_malloc(
        sizeof(mla_windows_window_surface_t)));

    if (window_surface == nullptr) {
        return false; // False
    }

    mla_memset(window_surface, 0, sizeof(mla_windows_window_surface_t));
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};

#ifdef mla_debug
    window_surface->last_fps_time = 0;
    window_surface->frames_accumulated = 0;
    window_surface->current_fps = 0;
#endif

    // Populate the surface structure
    outSurface.resource = window_surface;
    outSurface.resourceOwner = mla_buffer_reference(window_surface, true, __windows_surface_buffer_cleanup);
    outSurface.get_size = __windows_surface_get_size;
    outSurface.set_size = __windows_surface_set_size;
    outSurface.render_draw_commands = __windows_surface_render_draw_commands;

    return true; // True
}

mla_ui_surface_low_level_access_t g_ui_surface_low_level_access = {
    __windows_create_surface
};

#endif
