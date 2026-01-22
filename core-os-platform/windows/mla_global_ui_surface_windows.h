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

// Render cache structure for performance optimization
// Render cache structure for performance optimization
struct WindowsRenderCache {
    // Reusable objects
    SolidBrush* solidBrush;
    Pen* pen;
    GraphicsPath* path;

    // Font cache
    struct FontCacheEntry {
        WCHAR family[256];
        REAL size;
        Font* font;
        bool used;
    };
    FontCacheEntry fontCache[16];  // Fixed size cache
    int fontCacheSize;

    // Current state to avoid redundant updates
    Color currentFillColor;
    Color currentStrokeColor;
    REAL currentStrokeWidth;

    WindowsRenderCache() {
        solidBrush = new SolidBrush(Color(255, 255, 255, 255));
        pen = new Pen(Color(255, 0, 0, 0), 1.0f);
        path = new GraphicsPath();
        currentFillColor = Color(255, 255, 255, 255);
        currentStrokeColor = Color(255, 0, 0, 0);
        currentStrokeWidth = 1.0f;
        fontCacheSize = 0;
        for (int i = 0; i < 16; i++) {
            fontCache[i].used = false;
            fontCache[i].font = nullptr;
        }
    }

    ~WindowsRenderCache() {
        delete solidBrush;
        delete pen;
        delete path;
        for (int i = 0; i < fontCacheSize; i++) {
            if (fontCache[i].font) {
                delete fontCache[i].font;
            }
        }
    }

    Font* GetOrCreateFont(const WCHAR* familyName, REAL size) {
        // Check if font already exists in cache
        for (int i = 0; i < fontCacheSize; i++) {
            if (fontCache[i].used &&
                wcscmp(fontCache[i].family, familyName) == 0 &&
                fontCache[i].size == size) {
                return fontCache[i].font;
            }
        }

        // Create new font
        FontFamily fontFamily(familyName);
        Font* font = new Font(&fontFamily, size, FontStyleRegular, UnitPixel);

        // Add to cache if there's room
        if (fontCacheSize < 16) {
            wcscpy_s(fontCache[fontCacheSize].family, 256, familyName);
            fontCache[fontCacheSize].size = size;
            fontCache[fontCacheSize].font = font;
            fontCache[fontCacheSize].used = true;
            fontCacheSize++;
        }

        return font;
    }

    void UpdateFillBrush(const Color& color) {
        if (currentFillColor.GetValue() != color.GetValue()) {
            solidBrush->SetColor(color);
            currentFillColor = color;
        }
    }

    void UpdateStrokePen(const Color& color, REAL width) {
        bool colorChanged = currentStrokeColor.GetValue() != color.GetValue();
        bool widthChanged = currentStrokeWidth != width;

        if (colorChanged) {
            pen->SetColor(color);
            currentStrokeColor = color;
        }
        if (widthChanged) {
            pen->SetWidth(width);
            currentStrokeWidth = width;
        }
    }
};

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

    WindowsRenderCache* renderCache;

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
        return false;
    }

    surface->is_initialized = true;
    surface->hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    return true;
}

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
        window_surface->renderCache = new WindowsRenderCache();
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

    if (window_surface->memBitmap == nullptr || window_surface->cached_width != width || window_surface->cached_height != height) {

        if (window_surface->graphics)
            delete window_surface->graphics;
        if (window_surface->memBitmap)
            DeleteObject(window_surface->memBitmap);
        if (window_surface->memDC)
            DeleteDC(window_surface->memDC);

        window_surface->memDC = CreateCompatibleDC(hdc);
        window_surface->memBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(window_surface->memDC, window_surface->memBitmap);

        window_surface->graphics = new Graphics(window_surface->memDC);
        window_surface->graphics->SetSmoothingMode(SmoothingModeAntiAlias);
        window_surface->graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        window_surface->graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        window_surface->graphics->SetPixelOffsetMode(PixelOffsetModeHighQuality);
        window_surface->graphics->SetCompositingQuality(CompositingQualityHighQuality);

        window_surface->cached_width = width;
        window_surface->cached_height = height;
    }

    Graphics *graphics = window_surface->graphics;
    WindowsRenderCache* cache = window_surface->renderCache;

    cache->UpdateFillBrush(Color(255, 255, 255, 255));
    graphics->FillRectangle(cache->solidBrush, 0, 0, width, height);

    LinearGradientBrush *currentLinearGradient = nullptr;
    PathGradientBrush *currentRadialGradient = nullptr;

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                cache->UpdateFillBrush(__convert_color(rect.color));
                cache->UpdateStrokePen(__convert_color(rect.stroke), (REAL)rect.stroke_width);

                if (rect.rx > 0 || rect.ry > 0) {
                    cache->path->Reset();
                    REAL x = (REAL)rect.x, y = (REAL)rect.y;
                    REAL w = (REAL)rect.width, h = (REAL)rect.height;
                    REAL rx = (REAL)rect.rx, ry = (REAL)rect.ry;

                    cache->path->AddArc(x, y, rx * 2, ry * 2, 180, 90);
                    cache->path->AddArc(x + w - rx * 2, y, rx * 2, ry * 2, 270, 90);
                    cache->path->AddArc(x + w - rx * 2, y + h - ry * 2, rx * 2, ry * 2, 0, 90);
                    cache->path->AddArc(x, y + h - ry * 2, rx * 2, ry * 2, 90, 90);
                    cache->path->CloseFigure();

                    graphics->FillPath(cache->solidBrush, cache->path);
                    graphics->DrawPath(cache->pen, cache->path);
                } else {
                    graphics->FillRectangle(cache->solidBrush, (REAL)rect.x, (REAL)rect.y, (REAL)rect.width, (REAL)rect.height);
                    graphics->DrawRectangle(cache->pen, (REAL)rect.x, (REAL)rect.y, (REAL)rect.width, (REAL)rect.height);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;
                cache->UpdateFillBrush(__convert_color(circle.fill));
                cache->UpdateStrokePen(__convert_color(circle.stroke), (REAL)circle.stroke_width);

                REAL diameter = (REAL)(circle.r * 2);
                graphics->FillEllipse(cache->solidBrush, (REAL)(circle.cx - circle.r), (REAL)(circle.cy - circle.r), diameter, diameter);
                graphics->DrawEllipse(cache->pen, (REAL)(circle.cx - circle.r), (REAL)(circle.cy - circle.r), diameter, diameter);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipse = cmd.ellipse;
                cache->UpdateFillBrush(__convert_color(ellipse.fill));
                cache->UpdateStrokePen(__convert_color(ellipse.stroke), (REAL)ellipse.stroke_width);

                graphics->FillEllipse(cache->solidBrush, (REAL)(ellipse.cx - ellipse.rx), (REAL)(ellipse.cy - ellipse.ry),
                                     (REAL)(ellipse.rx * 2), (REAL)(ellipse.ry * 2));
                graphics->DrawEllipse(cache->pen, (REAL)(ellipse.cx - ellipse.rx), (REAL)(ellipse.cy - ellipse.ry),
                                     (REAL)(ellipse.rx * 2), (REAL)(ellipse.ry * 2));
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                cache->UpdateStrokePen(__convert_color(line.stroke), (REAL)line.stroke_width);
                graphics->DrawLine(cache->pen, (REAL)line.x1, (REAL)line.y1, (REAL)line.x2, (REAL)line.y2);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;

                PointF *points = new PointF[mla_array_list_size(polyline.points)];
                for (mla_size_t j = 0; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polyline.points, j);
                    points[j] = PointF((REAL)point.x, (REAL)point.y);
                }
                cache->UpdateStrokePen(__convert_color(polyline.stroke), (REAL)polyline.stroke_width);
                graphics->DrawLines(cache->pen, points, (INT)mla_array_list_size(polyline.points));
                delete[] points;
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;

                PointF *points = new PointF[mla_array_list_size(polygon.points)];
                for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                    points[j] = PointF((REAL)point.x, (REAL)point.y);
                }
                cache->UpdateFillBrush(__convert_color(polygon.fill));
                cache->UpdateStrokePen(__convert_color(polygon.stroke), (REAL)polygon.stroke_width);
                graphics->FillPolygon(cache->solidBrush, points, (INT)mla_array_list_size(polygon.points));
                graphics->DrawPolygon(cache->pen, points, (INT)mla_array_list_size(polygon.points));
                delete[] points;
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) break;

                cache->path->Reset();
                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);
                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            cache->path->StartFigure();
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            cache->path->AddLine((REAL)pathCmd.line_to.x, (REAL)pathCmd.line_to.y,
                                                (REAL)pathCmd.line_to.x, (REAL)pathCmd.line_to.y);
                            break;
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO: {
                            PointF ctrlPt((REAL)pathCmd.quadratic_curve_to.cpx, (REAL)pathCmd.quadratic_curve_to.cpy);
                            PointF endPt((REAL)pathCmd.quadratic_curve_to.x, (REAL)pathCmd.quadratic_curve_to.y);
                            PointF lastPt;
                            cache->path->GetLastPoint(&lastPt);
                            cache->path->AddBezier(lastPt, ctrlPt, ctrlPt, endPt);
                            break;
                        }
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO: {
                            PointF startPt;
                            cache->path->GetLastPoint(&startPt);
                            cache->path->AddBezier(startPt,
                                                  PointF((REAL)pathCmd.cubic_curve_to.cp1x, (REAL)pathCmd.cubic_curve_to.cp1y),
                                                  PointF((REAL)pathCmd.cubic_curve_to.cp2x, (REAL)pathCmd.cubic_curve_to.cp2y),
                                                  PointF((REAL)pathCmd.cubic_curve_to.x, (REAL)pathCmd.cubic_curve_to.y));
                            break;
                        }
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            cache->path->CloseFigure();
                            break;
                        default:
                            break;
                    }
                }
                cache->UpdateFillBrush(__convert_color(path.fill));
                cache->UpdateStrokePen(__convert_color(path.stroke), (REAL)path.stroke_width);
                graphics->FillPath(cache->solidBrush, cache->path);
                graphics->DrawPath(cache->pen, cache->path);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content))
                    break;
                if (mla_string_is_empty(cmd.text.font_family))
                    break;

                mla_string_utf16_buffer_t fontFamilyWide = mla_string_to_utf16_buffer(cmd.text.font_family);
                Font* font = cache->GetOrCreateFont((const WCHAR*)fontFamilyWide.data, (REAL)cmd.text.font_size);
                cache->UpdateFillBrush(__convert_color(cmd.text.fill));
                PointF origin((REAL)cmd.text.x, (REAL)cmd.text.y);

                mla_string_utf16_buffer_t contentWide = mla_string_to_utf16_buffer(cmd.text.content);
                graphics->DrawString((const WCHAR*)contentWide.data, -1, font, origin, cache->solidBrush);

                mla_string_utf16_buffer_destroy(fontFamilyWide);
                mla_string_utf16_buffer_destroy(contentWide);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT: {
                const auto &gradient = cmd.linear_gradient;
                if (currentLinearGradient) delete currentLinearGradient;

                PointF pt1((REAL)gradient.x1, (REAL)gradient.y1);
                PointF pt2((REAL)gradient.x2, (REAL)gradient.y2);

                currentLinearGradient = new LinearGradientBrush(pt1, pt2,
                                                                Color(255, 0, 0, 0), Color(255, 255, 255, 255));
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT: {
                const auto &gradient = cmd.radial_gradient;
                if (currentRadialGradient) delete currentRadialGradient;

                GraphicsPath gradPath;
                REAL diameter = (REAL)(gradient.r * 2);
                gradPath.AddEllipse((REAL)(gradient.cx - gradient.r),
                                   (REAL)(gradient.cy - gradient.r),
                                   diameter, diameter);

                currentRadialGradient = new PathGradientBrush(&gradPath);
                currentRadialGradient->SetCenterPoint(PointF((REAL)gradient.cx, (REAL)gradient.cy));
                currentRadialGradient->SetCenterColor(Color(255, 255, 255, 255));

                Color surroundColors[] = {Color(255, 0, 0, 0)};
                INT count = 1;
                currentRadialGradient->SetSurroundColors(surroundColors, &count);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP: {
                const auto &stop = cmd.stop;
                if (currentLinearGradient) {
                    Color colors[] = {__convert_color(stop.stop_color)};
                    REAL positions[] = {(REAL)stop.offset};
                    currentLinearGradient->SetInterpolationColors(colors, positions, 1);
                }
                break;
            }

            default:
                break;
        }
    }

    if (currentLinearGradient != nullptr)
        delete currentLinearGradient;
    if (currentRadialGradient != nullptr)
        delete currentRadialGradient;

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

    // Use cached font and brush
    Font* font = cache->GetOrCreateFont(L"Arial", 20.0f);
    cache->UpdateFillBrush(Color(255, 255, 0, 0));

    char buffer[64];
    wsprintfA(buffer, "FPS: %d", window_surface->current_fps);

    // Convert to wide string
    WCHAR wBuffer[64];
    MultiByteToWideChar(CP_ACP, 0, buffer, -1, wBuffer, 64);

    // Measure text
    RectF layoutRect;
    RectF boundingBox;
    graphics->MeasureString(wBuffer, -1, font, layoutRect, &boundingBox);

    int padding = 10;
    REAL x = (REAL)(width - boundingBox.Width - padding);
    REAL y = (REAL)(height - boundingBox.Height - padding);

    graphics->DrawString(wBuffer, -1, font, PointF(x, y), cache->solidBrush);
#endif

    BitBlt(hdc, 0, 0, width, height, window_surface->memDC, 0, 0, SRCCOPY);

    ReleaseDC(window_surface->hwnd, hdc);
    return true;
}

mla_buffer_cleanup_mode __windows_surface_buffer_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;

    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(data);

    if (window_surface != nullptr) {

        if (window_surface->renderCache)
            delete window_surface->renderCache;

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
        return false;
    }

    mla_memset(window_surface, 0, sizeof(mla_windows_window_surface_t));
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};

#ifdef mla_debug
    window_surface->last_fps_time = 0;
    window_surface->frames_accumulated = 0;
    window_surface->current_fps = 0;
#endif

    outSurface.resource = window_surface;
    outSurface.resourceOwner = mla_buffer_reference(window_surface, true, __windows_surface_buffer_cleanup);
    outSurface.get_size = __windows_surface_get_size;
    outSurface.set_size = __windows_surface_set_size;
    outSurface.render_draw_commands = __windows_surface_render_draw_commands;

    return true;
}

mla_ui_surface_low_level_access_t g_ui_surface_low_level_access = {
    __windows_create_surface
};

#endif
