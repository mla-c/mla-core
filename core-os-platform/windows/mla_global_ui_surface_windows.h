//
// Created by chris on 1/21/2026.
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

// Global Direct2D factories
static ID2D1Factory* g_pD2DFactory = nullptr;
static IDWriteFactory* g_pDWriteFactory = nullptr;

// Initialize Direct2D (call once at startup)
void __windows_d2d_init() {
    if (g_pD2DFactory == nullptr) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    }
    if (g_pDWriteFactory == nullptr) {
        DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
        );
    }
}

// Cleanup Direct2D (call at shutdown)
void __windows_d2d_shutdown() {
    if (g_pDWriteFactory) {
        g_pDWriteFactory->Release();
        g_pDWriteFactory = nullptr;
    }
    if (g_pD2DFactory) {
        g_pD2DFactory->Release();
        g_pD2DFactory = nullptr;
    }
}

struct MlaD2DAutoInit {
    MlaD2DAutoInit() { __windows_d2d_init(); }
    ~MlaD2DAutoInit() { __windows_d2d_shutdown(); }
};

// Single global instance
static MlaD2DAutoInit g_mlaD2DAutoInit;

// Render cache structure for performance optimization
// Render cache structure for performance optimization
struct WindowsRenderCache {
    // Reusable brushes
    ID2D1SolidColorBrush* solidBrush;

    // Font cache
    struct FontCacheEntry {
        WCHAR family[256];
        FLOAT size;
        IDWriteTextFormat* textFormat;
        bool used;
    };
    FontCacheEntry fontCache[16];  // Fixed size cache
    int fontCacheSize;

    // Current state to avoid redundant updates
    D2D1_COLOR_F currentFillColor;
    D2D1_COLOR_F currentStrokeColor;
    FLOAT currentStrokeWidth;

    WindowsRenderCache() {
        solidBrush = nullptr;

        currentFillColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
        currentStrokeColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);
        currentStrokeWidth = 1.0f;
        fontCacheSize = 0;

        for (int i = 0; i < 16; i++) {
            fontCache[i].used = false;
            fontCache[i].textFormat = nullptr;
        }
    }

    ~WindowsRenderCache() {
        ResetDeviceResources();

        for (int i = 0; i < 16; i++) { // Check all slots, as eviction might leave holes or used flags set
            if (fontCache[i].used && fontCache[i].textFormat) {
                fontCache[i].textFormat->Release();
                fontCache[i].textFormat = nullptr;
            }
        }
    }

    void ResetDeviceResources() {
        if (solidBrush) {
            solidBrush->Release();
            solidBrush = nullptr;
        }
        // Force color update next time
        currentFillColor = D2D1::ColorF(0, 0, 0, 0);
    }

    IDWriteTextFormat* GetOrCreateTextFormat(const WCHAR* familyName, FLOAT size) {
        // Check if font already exists in cache
        for (int i = 0; i < fontCacheSize; i++) {
            if (fontCache[i].used &&
                wcscmp(fontCache[i].family, familyName) == 0 &&
                fontCache[i].size == size) {
                return fontCache[i].textFormat;
            }
        }

        // Create new text format
        IDWriteTextFormat* textFormat = nullptr;
        g_pDWriteFactory->CreateTextFormat(
            familyName,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-us",
            &textFormat
        );

        if (textFormat) {
             // Logic Fix: Ensure we track this pointer even if cache is full
             int index;
             if (fontCacheSize < 16) {
                 index = fontCacheSize++;
             } else {
                 // Cache full: Evict the first slot (simple FIFO) to stop the memory leak
                 index = 0;
                 if (fontCache[index].used && fontCache[index].textFormat) {
                     fontCache[index].textFormat->Release();
                 }
             }

            wcscpy_s(fontCache[index].family, 256, familyName);
            fontCache[index].size = size;
            fontCache[index].textFormat = textFormat;
            fontCache[index].used = true;

            return textFormat;
        }

        return nullptr;
    }

    void UpdateSolidBrush(ID2D1RenderTarget* renderTarget, const D2D1_COLOR_F& color) {
        if (!solidBrush) {
            renderTarget->CreateSolidColorBrush(color, &solidBrush);
            currentFillColor = color;
        } else if (memcmp(&currentFillColor, &color, sizeof(D2D1_COLOR_F)) != 0) {
            solidBrush->SetColor(color);
            currentFillColor = color;
        }
    }
};

struct mla_windows_window_surface_t {
    HWND hwnd;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;

    // Direct2D rendering resources
    ID2D1HwndRenderTarget* renderTarget;
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

    surface->renderCache = new WindowsRenderCache();
    surface->is_initialized = true;
    surface->hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    return true;
}

D2D1_COLOR_F __convert_color(const mla_ui_surface_draw_command_color_t &color) {
    return D2D1::ColorF(
        (FLOAT)color.r / 255.0f,
        (FLOAT)color.g / 255.0f,
        (FLOAT)color.b / 255.0f,
        (FLOAT)color.a / 255.0f
    );
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

    // Create render target if needed
    if (!window_surface->renderTarget) {
        RECT rc;
        GetClientRect(window_surface->hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        g_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(window_surface->hwnd, size),
            &window_surface->renderTarget
        );

        if (!window_surface->renderTarget) {
            return false;
        }

        window_surface->renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        window_surface->renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    } else {
        // Fix: Ensure RenderTarget matches Window Client Size
        // If we don't do this, D2D scales the output, making it look blurry.
        RECT rc;
        GetClientRect(window_surface->hwnd, &rc);
        D2D1_SIZE_U windowSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        D2D1_SIZE_U targetSize = window_surface->renderTarget->GetPixelSize();

        if (windowSize.width != targetSize.width || windowSize.height != targetSize.height) {
            window_surface->renderTarget->Resize(windowSize);
        }
    }

    ID2D1HwndRenderTarget* renderTarget = window_surface->renderTarget;
    WindowsRenderCache* cache = window_surface->renderCache;

    // Begin drawing
    renderTarget->BeginDraw();

    // Clear background
    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // Temporary brushes for gradients
    ID2D1LinearGradientBrush* currentLinearGradient = nullptr;
    ID2D1RadialGradientBrush* currentRadialGradient = nullptr;
    ID2D1SolidColorBrush* strokeBrush = nullptr;

    // Track current path being built
    D2D1_POINT_2F currentPathPoint = {0, 0};

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                D2D1_RECT_F d2dRect = D2D1::RectF(
                    (FLOAT)rect.x, (FLOAT)rect.y,
                    (FLOAT)(rect.x + rect.width), (FLOAT)(rect.y + rect.height)
                );

                // Fill
                cache->UpdateSolidBrush(renderTarget, __convert_color(rect.color));
                if (rect.rx > 0 || rect.ry > 0) {
                    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)rect.rx, (FLOAT)rect.ry);
                    renderTarget->FillRoundedRectangle(roundedRect, cache->solidBrush);
                } else {
                    renderTarget->FillRectangle(d2dRect, cache->solidBrush);
                }

                // Stroke
                if (rect.stroke_width > 0) {
                    if (!strokeBrush) {
                        renderTarget->CreateSolidColorBrush(__convert_color(rect.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(__convert_color(rect.stroke));
                    }

                    if (rect.rx > 0 || rect.ry > 0) {
                        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)rect.rx, (FLOAT)rect.ry);
                        renderTarget->DrawRoundedRectangle(roundedRect, strokeBrush, (FLOAT)rect.stroke_width);
                    } else {
                        renderTarget->DrawRectangle(d2dRect, strokeBrush, (FLOAT)rect.stroke_width);
                    }
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;
                D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                    D2D1::Point2F((FLOAT)circle.cx, (FLOAT)circle.cy),
                    (FLOAT)circle.r, (FLOAT)circle.r
                );

                cache->UpdateSolidBrush(renderTarget, __convert_color(circle.fill));
                renderTarget->FillEllipse(ellipse, cache->solidBrush);

                if (circle.stroke_width > 0) {
                    if (!strokeBrush) {
                        renderTarget->CreateSolidColorBrush(__convert_color(circle.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(__convert_color(circle.stroke));
                    }
                    renderTarget->DrawEllipse(ellipse, strokeBrush, (FLOAT)circle.stroke_width);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipseCmd = cmd.ellipse;
                D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                    D2D1::Point2F((FLOAT)ellipseCmd.cx, (FLOAT)ellipseCmd.cy),
                    (FLOAT)ellipseCmd.rx, (FLOAT)ellipseCmd.ry
                );

                cache->UpdateSolidBrush(renderTarget, __convert_color(ellipseCmd.fill));
                renderTarget->FillEllipse(ellipse, cache->solidBrush);

                if (ellipseCmd.stroke_width > 0) {
                    if (!strokeBrush) {
                        renderTarget->CreateSolidColorBrush(__convert_color(ellipseCmd.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(__convert_color(ellipseCmd.stroke));
                    }
                    renderTarget->DrawEllipse(ellipse, strokeBrush, (FLOAT)ellipseCmd.stroke_width);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                if (!strokeBrush) {
                    renderTarget->CreateSolidColorBrush(__convert_color(line.stroke), &strokeBrush);
                } else {
                    strokeBrush->SetColor(__convert_color(line.stroke));
                }

                renderTarget->DrawLine(
                    D2D1::Point2F((FLOAT)line.x1, (FLOAT)line.y1),
                    D2D1::Point2F((FLOAT)line.x2, (FLOAT)line.y2),
                    strokeBrush,
                    (FLOAT)line.stroke_width
                );
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;

                ID2D1PathGeometry* geometry = nullptr;
                ID2D1GeometrySink* sink = nullptr;

                g_pD2DFactory->CreatePathGeometry(&geometry);
                geometry->Open(&sink);

                const mla_ui_surface_draw_point_t &firstPoint = mla_array_list_get_unsafe(polyline.points, 0);
                sink->BeginFigure(D2D1::Point2F((FLOAT)firstPoint.x, (FLOAT)firstPoint.y), D2D1_FIGURE_BEGIN_HOLLOW);

                for (mla_size_t j = 1; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polyline.points, j);
                    sink->AddLine(D2D1::Point2F((FLOAT)point.x, (FLOAT)point.y));
                }

                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                sink->Close();

                if (!strokeBrush) {
                    renderTarget->CreateSolidColorBrush(__convert_color(polyline.stroke), &strokeBrush);
                } else {
                    strokeBrush->SetColor(__convert_color(polyline.stroke));
                }
                renderTarget->DrawGeometry(geometry, strokeBrush, (FLOAT)polyline.stroke_width);

                sink->Release();
                geometry->Release();
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;

                ID2D1PathGeometry* geometry = nullptr;
                ID2D1GeometrySink* sink = nullptr;

                g_pD2DFactory->CreatePathGeometry(&geometry);
                geometry->Open(&sink);

                const mla_ui_surface_draw_point_t &firstPoint = mla_array_list_get_unsafe(polygon.points, 0);
                sink->BeginFigure(D2D1::Point2F((FLOAT)firstPoint.x, (FLOAT)firstPoint.y), D2D1_FIGURE_BEGIN_FILLED);

                for (mla_size_t j = 1; j < mla_array_list_size(polygon.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                    sink->AddLine(D2D1::Point2F((FLOAT)point.x, (FLOAT)point.y));
                }

                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                sink->Close();

                cache->UpdateSolidBrush(renderTarget, __convert_color(polygon.fill));
                renderTarget->FillGeometry(geometry, cache->solidBrush);

                if (polygon.stroke_width > 0) {
                    if (!strokeBrush) {
                        renderTarget->CreateSolidColorBrush(__convert_color(polygon.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(__convert_color(polygon.stroke));
                    }
                    renderTarget->DrawGeometry(geometry, strokeBrush, (FLOAT)polygon.stroke_width);
                }

                sink->Release();
                geometry->Release();
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) break;

                ID2D1PathGeometry* geometry = nullptr;
                ID2D1GeometrySink* sink = nullptr;

                g_pD2DFactory->CreatePathGeometry(&geometry);
                geometry->Open(&sink);

                bool figureStarted = false;

                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);

                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            if (figureStarted) {
                                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                            }
                            currentPathPoint = D2D1::Point2F((FLOAT)pathCmd.move_to.x, (FLOAT)pathCmd.move_to.y);
                            sink->BeginFigure(currentPathPoint, D2D1_FIGURE_BEGIN_FILLED);
                            figureStarted = true;
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            if (figureStarted) {
                                currentPathPoint = D2D1::Point2F((FLOAT)pathCmd.line_to.x, (FLOAT)pathCmd.line_to.y);
                                sink->AddLine(currentPathPoint);
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO:
                            if (figureStarted) {
                                D2D1_QUADRATIC_BEZIER_SEGMENT segment;
                                segment.point1 = D2D1::Point2F((FLOAT)pathCmd.quadratic_curve_to.cpx, (FLOAT)pathCmd.quadratic_curve_to.cpy);
                                segment.point2 = D2D1::Point2F((FLOAT)pathCmd.quadratic_curve_to.x, (FLOAT)pathCmd.quadratic_curve_to.y);
                                sink->AddQuadraticBezier(segment);
                                currentPathPoint = segment.point2;
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO:
                            if (figureStarted) {
                                D2D1_BEZIER_SEGMENT segment;
                                segment.point1 = D2D1::Point2F((FLOAT)pathCmd.cubic_curve_to.cp1x, (FLOAT)pathCmd.cubic_curve_to.cp1y);
                                segment.point2 = D2D1::Point2F((FLOAT)pathCmd.cubic_curve_to.cp2x, (FLOAT)pathCmd.cubic_curve_to.cp2y);
                                segment.point3 = D2D1::Point2F((FLOAT)pathCmd.cubic_curve_to.x, (FLOAT)pathCmd.cubic_curve_to.y);
                                sink->AddBezier(segment);
                                currentPathPoint = segment.point3;
                            }
                            break;

                        case  MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO:
                            if (figureStarted) {
                                D2D1_ARC_SEGMENT segment;
                                segment.point = D2D1::Point2F((FLOAT)pathCmd.arc_to.x, (FLOAT)pathCmd.arc_to.y);
                                segment.size = D2D1::SizeF((FLOAT)pathCmd.arc_to.rx, (FLOAT)pathCmd.arc_to.ry);
                                segment.rotationAngle = (FLOAT)pathCmd.arc_to.x_axis_rotation;
                                segment.sweepDirection = pathCmd.arc_to.sweep_flag ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
                                segment.arcSize = pathCmd.arc_to.large_arc_flag ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL;
                                sink->AddArc(segment);
                                currentPathPoint = segment.point;
                            }
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            if (figureStarted) {
                                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                                figureStarted = false;
                            }
                            break;
                    }
                }

                if (figureStarted) {
                    sink->EndFigure(D2D1_FIGURE_END_OPEN);
                }

                sink->Close();

                cache->UpdateSolidBrush(renderTarget, __convert_color(path.fill));
                renderTarget->FillGeometry(geometry, cache->solidBrush);

                if (path.stroke_width > 0) {
                    if (!strokeBrush) {
                        renderTarget->CreateSolidColorBrush(__convert_color(path.stroke), &strokeBrush);
                    } else {
                        strokeBrush->SetColor(__convert_color(path.stroke));
                    }
                    renderTarget->DrawGeometry(geometry, strokeBrush, (FLOAT)path.stroke_width);
                }

                sink->Release();
                geometry->Release();
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content))
                    break;
                if (mla_string_is_empty(cmd.text.font_family))
                    break;

                mla_string_utf16_buffer_t fontFamilyWide = mla_string_to_utf16_buffer(cmd.text.font_family);
                IDWriteTextFormat* textFormat = cache->GetOrCreateTextFormat((const WCHAR*)fontFamilyWide.data, (FLOAT)cmd.text.font_size);

                if (textFormat) {
                    cache->UpdateSolidBrush(renderTarget, __convert_color(cmd.text.fill));

                    mla_string_utf16_buffer_t contentWide = mla_string_to_utf16_buffer(cmd.text.content);

                    // Calculate text length
                    int textLength = 0;
                    while (((const WCHAR*)contentWide.data)[textLength] != 0) textLength++;

                    renderTarget->DrawText(
                        (const WCHAR*)contentWide.data,
                        textLength,
                        textFormat,
                        D2D1::RectF((FLOAT)cmd.text.x, (FLOAT)cmd.text.y, 10000.0f, 10000.0f),
                        cache->solidBrush
                    );

                    mla_string_utf16_buffer_destroy(contentWide);
                }

                mla_string_utf16_buffer_destroy(fontFamilyWide);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT: {
                const auto &gradient = cmd.linear_gradient;

                if (currentLinearGradient) {
                    currentLinearGradient->Release();
                    currentLinearGradient = nullptr;
                }

                D2D1_GRADIENT_STOP stops[2];
                stops[0].color = D2D1::ColorF(0, 0, 0, 1);
                stops[0].position = 0.0f;
                stops[1].color = D2D1::ColorF(1, 1, 1, 1);
                stops[1].position = 1.0f;

                ID2D1GradientStopCollection* stopCollection = nullptr;
                renderTarget->CreateGradientStopCollection(stops, 2, &stopCollection);

                if (stopCollection) {
                    renderTarget->CreateLinearGradientBrush(
                        D2D1::LinearGradientBrushProperties(
                            D2D1::Point2F((FLOAT)gradient.x1, (FLOAT)gradient.y1),
                            D2D1::Point2F((FLOAT)gradient.x2, (FLOAT)gradient.y2)
                        ),
                        stopCollection,
                        &currentLinearGradient
                    );
                    stopCollection->Release();
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT: {
                const auto &gradient = cmd.radial_gradient;

                if (currentRadialGradient) {
                    currentRadialGradient->Release();
                    currentRadialGradient = nullptr;
                }

                D2D1_GRADIENT_STOP stops[2];
                stops[0].color = D2D1::ColorF(1, 1, 1, 1);
                stops[0].position = 0.0f;
                stops[1].color = D2D1::ColorF(0, 0, 0, 1);
                stops[1].position = 1.0f;

                ID2D1GradientStopCollection* stopCollection = nullptr;
                renderTarget->CreateGradientStopCollection(stops, 2, &stopCollection);

                if (stopCollection) {
                    renderTarget->CreateRadialGradientBrush(
                        D2D1::RadialGradientBrushProperties(
                            D2D1::Point2F((FLOAT)gradient.cx, (FLOAT)gradient.cy),
                            D2D1::Point2F(0, 0),
                            (FLOAT)gradient.r,
                            (FLOAT)gradient.r
                        ),
                        stopCollection,
                        &currentRadialGradient
                    );
                    stopCollection->Release();
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP: {
                // Note: Direct2D gradient stops need to be set during gradient creation
                // This simplified implementation doesn't support dynamic stop updates
                break;
            }

            default:
                break;
        }
    }

    // Cleanup temporary brushes
    if (currentLinearGradient)
        currentLinearGradient->Release();
    if (currentRadialGradient)
        currentRadialGradient->Release();

    if (strokeBrush)
        strokeBrush->Release();

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

    // Draw FPS counter
    IDWriteTextFormat* debugFont = cache->GetOrCreateTextFormat(L"Arial", 15.0f);
    if (debugFont) {
        ID2D1SolidColorBrush* debugBrush = nullptr;
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &debugBrush);

        char buffer[64];
        wsprintfA(buffer, "FPS: %d", window_surface->current_fps);

        WCHAR wBuffer[64];
        MultiByteToWideChar(CP_ACP, 0, buffer, -1, wBuffer, 64);

        D2D1_SIZE_F rtSize = renderTarget->GetSize();

        renderTarget->DrawText(
            wBuffer,
            wcslen(wBuffer),
            debugFont,
            D2D1::RectF(rtSize.width - 75, rtSize.height - 25, rtSize.width, rtSize.height),
            debugBrush
        );
    }
#endif

    HRESULT hr = renderTarget->EndDraw();

    // Handle device lost
    if (hr == D2DERR_RECREATE_TARGET) {

        if (window_surface->renderCache) {
            window_surface->renderCache->ResetDeviceResources();
        }

        renderTarget->Release();
        window_surface->renderTarget = nullptr;

        return true;  // Will recreate on next frame
    }

    return SUCCEEDED(hr);
}

mla_buffer_cleanup_mode __windows_surface_buffer_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;

    mla_windows_window_surface_t *window_surface = static_cast<mla_windows_window_surface_t *>(data);

    if (window_surface != nullptr) {
        if (window_surface->renderCache)
            delete window_surface->renderCache;

        if (window_surface->renderTarget) {
            window_surface->renderTarget->Release();
            window_surface->renderTarget = nullptr;
        }

        if (IsWindow(window_surface->hwnd)) {
            DestroyWindow(window_surface->hwnd);
        }
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
