//
// Created by copilot on 2/21/2026.
// Skia-based cross-platform UI Surface Implementation
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_SKIA_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_SKIA_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"
#include "../../core-os/ui/display/mla_ui_display_surface.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"

#include <new> // For placement new

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define mla_global_ui_surface_skia_font_cache_size 16

// ============================================================================
// Font cache
// ============================================================================

struct mla_global_ui_surface_skia_font_cache_item {
    mla_ui_surface_font_type_t font_type;
    sk_sp<SkTypeface> typeface;
    SkFont font;
};

struct mla_global_ui_surface_skia_font_cache_item_initializer {
    static mla_global_ui_surface_skia_font_cache_item init() {
        return {
            mla_ui_surface_font_type_empty(),
            nullptr,
            SkFont()
        };
    }
};

// ============================================================================
// Render cache for performance optimization
// ============================================================================

struct mla_global_ui_surface_skia_cache_t {
    // Reusable paint objects
    SkPaint fillPaint;
    SkPaint strokePaint;

    // Font cache
    mla_array_list_t<mla_global_ui_surface_skia_font_cache_item,
        mla_global_ui_surface_skia_font_cache_item_initializer> fontCache;

    // Font manager
    sk_sp<SkFontMgr> fontManager;
};

mla_global_ui_surface_skia_cache_t __mla_global_ui_surface_skia_cache_empty() {
    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);

    SkPaint stroke;
    stroke.setAntiAlias(true);
    stroke.setStyle(SkPaint::kStroke_Style);

    return {
        fill,
        stroke,
        mla_array_list_empty<mla_global_ui_surface_skia_font_cache_item,
            mla_global_ui_surface_skia_font_cache_item_initializer>(),
        SkFontMgr::RefEmpty()
    };
}

// ============================================================================
// Font cache helpers
// ============================================================================

SkFont *__mla_global_ui_surface_skia_font_cache_getOrCreateFont(
    mla_global_ui_surface_skia_cache_t &cache, const mla_ui_surface_font_type_t &fontType) {
    if (mla_string_is_empty(fontType.family))
        return nullptr;

    // Check if font already exists in cache
    for (mla_size_t i = 0; i < mla_array_list_size(cache.fontCache); i++) {
        mla_global_ui_surface_skia_font_cache_item &item = mla_array_list_get_unsafe(
            cache.fontCache, i);

        if (!mla_ui_surface_font_type_equals(item.font_type, fontType))
            continue;

        return &item.font;
    }

    // Create new typeface
    mla_c_string_t familyCStr = mla_string_to_cString(fontType.family);
    if (familyCStr.c_str == nullptr) return nullptr;

    SkFontStyle style(
        fontType.bold ? SkFontStyle::kBold_Weight : SkFontStyle::kNormal_Weight,
        SkFontStyle::kNormal_Width,
        fontType.italic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant
    );

    sk_sp<SkTypeface> typeface;
    if (cache.fontManager) {
        typeface = cache.fontManager->matchFamilyStyle(familyCStr.c_str, style);
    }
    if (!typeface) {
        typeface = SkTypeface::MakeDefault();
    }

    if (familyCStr.isOwner) {
        mla_free(const_cast<mla_char_t *>(familyCStr.c_str));
    }

    SkFont font(typeface, (SkScalar) fontType.size);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    mla_global_ui_surface_skia_font_cache_item newItem = {
        fontType,
        typeface,
        font
    };

    if (mla_array_list_size(cache.fontCache) < mla_global_ui_surface_skia_font_cache_size) {
        mla_array_list_add(cache.fontCache, newItem);
        return &mla_array_list_get_unsafe(cache.fontCache,
            mla_array_list_size(cache.fontCache) - 1).font;
    } else {
        // Overwrite oldest entry (index 0) when cache is full
        mla_array_list_get_unsafe(cache.fontCache, 0) = newItem;
        return &mla_array_list_get_unsafe(cache.fontCache, 0).font;
    }
}

// ============================================================================
// Color conversion
// ============================================================================

static inline SkColor __skia_convert_color(const mla_ui_surface_draw_command_color_t &color) {
    return SkColorSetARGB(color.a, color.r, color.g, color.b);
}

// ============================================================================
// Platform-specific window surface
// ============================================================================

#ifdef _WIN32

// Windows platform surface
struct mla_skia_window_surface_t {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;

    // Skia rendering resources
    sk_sp<GrDirectContext> grContext;
    sk_sp<SkSurface> skSurface;
    mla_global_ui_surface_skia_cache_t renderCache;

    mla_uint32_t currentWidth;
    mla_uint32_t currentHeight;

#ifdef mla_debug_build
    mla_uint64_t DEBUG_last_fps_time;
    mla_int32_t DEBUG_frames_accumulated;
    mla_int32_t DEBUG_current_fps;
#endif
};

LRESULT CALLBACK __skia_surface_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

mla_bool_t __skia_create_platform_window(mla_skia_window_surface_t *surface) {
    const char CLASS_NAME[] = "MLA_Skia_Window_Class";

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = __skia_surface_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.style = CS_OWNDC;

    RegisterClassExA(&wc);

    int width = surface->default_size.width;
    if (width == 0) width = CW_USEDEFAULT;
    int height = surface->default_size.height;
    if (height == 0) height = CW_USEDEFAULT;

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "MLA App (Skia)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, GetModuleHandleA(nullptr), nullptr
    );

    if (hwnd == nullptr) return false;

    surface->hwnd = hwnd;
    surface->hdc = GetDC(hwnd);

    // Set up OpenGL pixel format
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(surface->hdc, &pfd);
    SetPixelFormat(surface->hdc, pixelFormat, &pfd);

    surface->hglrc = wglCreateContext(surface->hdc);
    wglMakeCurrent(surface->hdc, surface->hglrc);

    surface->is_initialized = true;
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    return true;
}

mla_ui_surface_size_t __skia_surface_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);

    if (window_surface == nullptr) return size;
    if (!window_surface->is_initialized) return window_surface->default_size;
    if (!IsWindow(window_surface->hwnd)) return size;

    RECT rect;
    if (GetClientRect(window_surface->hwnd, &rect)) {
        size.width = (mla_uint32_t)(rect.right - rect.left);
        size.height = (mla_uint32_t)(rect.bottom - rect.top);
    }

    return size;
}

mla_bool_t __skia_surface_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);

    if (window_surface == nullptr) return false;
    if (!window_surface->is_initialized) {
        window_surface->default_size = size;
        return true;
    }
    if (!IsWindow(window_surface->hwnd)) return false;

    if (SetWindowPos(window_surface->hwnd, nullptr, 0, 0, (int) size.width, (int) size.height,
                     SWP_NOMOVE | SWP_NOZORDER)) {
        return true;
    }

    return false;
}

mla_bool_t __skia_ScreenPosition_to_client_position(const HWND &hwnd, POINT &cursorPos,
                                                     mla_ui_surface_draw_point_t &out_clientPosition) {
    RECT clientRect;
    if (GetClientRect(hwnd, &clientRect)) {
        if (PtInRect(&clientRect, cursorPos)) {
            out_clientPosition.x = (mla_double_t) cursorPos.x;
            out_clientPosition.y = (mla_double_t) cursorPos.y;
            return true;
        }
    }
    return false;
}

mla_ui_surface_input_states_t __skia_surface_input_states(const mla_ui_surface_t &surface) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);

    if (window_surface == nullptr || !window_surface->is_initialized || !IsWindow(window_surface->hwnd)) {
        return inputStates;
    }

    POINT cursorPos;
    if (GetCursorPos(&cursorPos)) {
        if (ScreenToClient(window_surface->hwnd, &cursorPos)) {
            __skia_ScreenPosition_to_client_position(window_surface->hwnd, cursorPos, inputStates.cursorPosition);
        }
    }

    inputStates.leftMouseButtonDown = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
    inputStates.rightMouseButtonDown = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;
    inputStates.middleMouseButtonDown = (GetKeyState(VK_MBUTTON) & 0x8000) != 0;

    inputStates.shiftKeyDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    inputStates.ctrlKeyDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    inputStates.altKeyDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
    inputStates.metaKeyDown = ((GetKeyState(VK_LWIN) & 0x8000) != 0) ||
                              ((GetKeyState(VK_RWIN) & 0x8000) != 0);

    inputStates.keyCodeDown = 0;
    for (int key = 0x08; key <= 0xFE; ++key) {
        if ((key == VK_SHIFT) || (key == VK_CONTROL) || (key == VK_MENU)) continue;
        if ((GetKeyState(key) & 0x8000) != 0) {
            inputStates.keyCodeDown = (mla_uint32_t) key;
            break;
        }
    }

    return inputStates;
}

void __skia_platform_swap_buffers(mla_skia_window_surface_t *surface) {
    SwapBuffers(surface->hdc);
}

void __skia_platform_make_current(mla_skia_window_surface_t *surface) {
    wglMakeCurrent(surface->hdc, surface->hglrc);
}

mla_bool_t __skia_platform_get_window_size(mla_skia_window_surface_t *surface, mla_uint32_t &outWidth, mla_uint32_t &outHeight) {
    RECT rc;
    if (!GetClientRect(surface->hwnd, &rc)) return false;
    outWidth = (mla_uint32_t)(rc.right - rc.left);
    outHeight = (mla_uint32_t)(rc.bottom - rc.top);
    return true;
}

#else // Linux / X11

// Linux/X11 platform surface
struct mla_skia_window_surface_t {
    Display *display;
    Window window;
    GLXContext glxContext;
    Colormap colormap;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;

    // Skia rendering resources
    sk_sp<GrDirectContext> grContext;
    sk_sp<SkSurface> skSurface;
    mla_global_ui_surface_skia_cache_t renderCache;

    mla_uint32_t currentWidth;
    mla_uint32_t currentHeight;

#ifdef mla_debug_build
    mla_uint64_t DEBUG_last_fps_time;
    mla_int32_t DEBUG_frames_accumulated;
    mla_int32_t DEBUG_current_fps;
#endif
};

mla_bool_t __skia_create_platform_window(mla_skia_window_surface_t *surface) {
    Display *display = XOpenDisplay(nullptr);
    if (!display) return false;

    int screen = DefaultScreen(display);

    // Choose GLX visual
    static int visualAttribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo *vi = glXChooseVisual(display, screen, visualAttribs);
    if (!vi) {
        XCloseDisplay(display);
        return false;
    }

    Colormap colormap = XCreateColormap(display, RootWindow(display, screen), vi->visual, AllocNone);

    XSetWindowAttributes swa;
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask |
                     PointerMotionMask | StructureNotifyMask;

    int width = surface->default_size.width;
    if (width == 0) width = 800;
    int height = surface->default_size.height;
    if (height == 0) height = 600;

    Window window = XCreateWindow(
        display, RootWindow(display, screen),
        0, 0, width, height, 0,
        vi->depth, InputOutput, vi->visual,
        CWColormap | CWEventMask, &swa
    );

    XStoreName(display, window, "MLA App (Skia)");
    XMapWindow(display, window);

    // Set up WM_DELETE_WINDOW protocol
    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    GLXContext glxContext = glXCreateContext(display, vi, nullptr, GL_TRUE);
    glXMakeCurrent(display, window, glxContext);

    XFree(vi);

    surface->display = display;
    surface->window = window;
    surface->glxContext = glxContext;
    surface->colormap = colormap;
    surface->is_initialized = true;
    surface->currentWidth = (mla_uint32_t) width;
    surface->currentHeight = (mla_uint32_t) height;

    return true;
}

mla_ui_surface_size_t __skia_surface_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);

    if (window_surface == nullptr) return size;
    if (!window_surface->is_initialized) return window_surface->default_size;

    XWindowAttributes attrs;
    if (XGetWindowAttributes(window_surface->display, window_surface->window, &attrs)) {
        size.width = (mla_uint32_t) attrs.width;
        size.height = (mla_uint32_t) attrs.height;
    }

    return size;
}

mla_bool_t __skia_surface_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);

    if (window_surface == nullptr) return false;
    if (!window_surface->is_initialized) {
        window_surface->default_size = size;
        return true;
    }

    XResizeWindow(window_surface->display, window_surface->window, size.width, size.height);
    return true;
}

mla_ui_surface_input_states_t __skia_surface_input_states(const mla_ui_surface_t &surface) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);

    if (window_surface == nullptr || !window_surface->is_initialized) {
        return inputStates;
    }

    Window root_return, child_return;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

    if (XQueryPointer(window_surface->display, window_surface->window,
                      &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask)) {
        inputStates.cursorPosition.x = (mla_double_t) win_x;
        inputStates.cursorPosition.y = (mla_double_t) win_y;

        inputStates.leftMouseButtonDown = (mask & Button1Mask) != 0;
        inputStates.middleMouseButtonDown = (mask & Button2Mask) != 0;
        inputStates.rightMouseButtonDown = (mask & Button3Mask) != 0;
        inputStates.shiftKeyDown = (mask & ShiftMask) != 0;
        inputStates.ctrlKeyDown = (mask & ControlMask) != 0;
        inputStates.altKeyDown = (mask & Mod1Mask) != 0;
        inputStates.metaKeyDown = (mask & Mod4Mask) != 0;
    }

    return inputStates;
}

void __skia_platform_swap_buffers(mla_skia_window_surface_t *surface) {
    glXSwapBuffers(surface->display, surface->window);
}

void __skia_platform_make_current(mla_skia_window_surface_t *surface) {
    glXMakeCurrent(surface->display, surface->window, surface->glxContext);
}

mla_bool_t __skia_platform_get_window_size(mla_skia_window_surface_t *surface, mla_uint32_t &outWidth, mla_uint32_t &outHeight) {
    XWindowAttributes attrs;
    if (!XGetWindowAttributes(surface->display, surface->window, &attrs)) return false;
    outWidth = (mla_uint32_t) attrs.width;
    outHeight = (mla_uint32_t) attrs.height;
    return true;
}

#endif // _WIN32

// ============================================================================
// Skia GPU context and surface management
// ============================================================================

mla_bool_t __skia_ensure_gpu_context(mla_skia_window_surface_t *surface) {
    if (surface->grContext) return true;

    __skia_platform_make_current(surface);

    sk_sp<const GrGLInterface> glInterface = GrGLMakeNativeInterface();
    if (!glInterface) return false;

    surface->grContext = GrDirectContexts::MakeGL(glInterface);
    return surface->grContext != nullptr;
}

mla_bool_t __skia_ensure_render_surface(mla_skia_window_surface_t *surface, mla_uint32_t width, mla_uint32_t height) {
    if (width == 0 || height == 0) return false;

    // Recreate surface if size changed
    if (surface->skSurface && surface->currentWidth == width && surface->currentHeight == height) {
        return true;
    }

    if (!__skia_ensure_gpu_context(surface)) return false;

    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = 0; // Default framebuffer
    fbInfo.fFormat = GL_RGBA8;

    auto backendRT = GrBackendRenderTargets::MakeGL(
        (int) width, (int) height,
        0, // sample count
        8, // stencil bits
        fbInfo
    );

    surface->skSurface = SkSurfaces::WrapBackendRenderTarget(
        surface->grContext.get(),
        backendRT,
        kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        nullptr, // color space
        nullptr  // surface props
    );

    if (!surface->skSurface) return false;

    surface->currentWidth = width;
    surface->currentHeight = height;

    return true;
}

// ============================================================================
// Text size calculation
// ============================================================================

mla_ui_surface_draw_size_t __skia_surface_calc_text_size(const mla_ui_surface_t &surface,
                                                          const mla_ui_surface_font_type_t &font_type,
                                                          const mla_string_t &text) {
    mla_ui_surface_draw_size_t size = {0, 0};

    if (mla_string_is_empty(text)) return size;

    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);
    if (window_surface == nullptr) return size;

    SkFont *font = __mla_global_ui_surface_skia_font_cache_getOrCreateFont(
        window_surface->renderCache, font_type);

    if (!font) return size;

    mla_c_string_t cStr = mla_string_to_cString(text);
    if (cStr.c_str == nullptr) return size;

    SkRect bounds;
    mla_size_t textLen = mla_strlen(cStr.c_str);
    font->measureText(cStr.c_str, textLen, SkTextEncoding::kUTF8, &bounds);

    size.width = (mla_double_t) bounds.width();
    size.height = (mla_double_t)(font->getMetrics(nullptr));
    if (size.height < 0) size.height = -size.height;

    if (cStr.isOwner) {
        mla_free(const_cast<mla_char_t *>(cStr.c_str));
    }

    return size;
}

// ============================================================================
// Draw command rendering helpers
// ============================================================================

static inline void __skia_set_fill_color(SkPaint &paint, const mla_ui_surface_draw_command_color_t &color) {
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(__skia_convert_color(color));
    paint.setShader(nullptr);
}

static inline void __skia_set_stroke(SkPaint &paint, const mla_ui_surface_draw_command_color_t &color, mla_double_t width) {
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(__skia_convert_color(color));
    paint.setStrokeWidth((SkScalar) width);
    paint.setShader(nullptr);
}

// ============================================================================
// Arc endpoint parameterization helper (SVG arc to center parameterization)
// Skia uses SVG-style arcs via SkPath::arcTo, which accepts the same parameters.
// ============================================================================

// ============================================================================
// Event processing helpers
// ============================================================================

#ifdef _WIN32

void __skia_process_win32_events(mla_skia_window_surface_t *window_surface,
                                  mla_array_list_t<mla_ui_surface_input_event_t,
                                      mla_ui_surface_input_event_initializer_t> &eventsSinceLastFrame) {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.hwnd == window_surface->hwnd) {
            // Handle Mouse Release Events (Clicks)
            if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_MBUTTONUP) {
                int physicalX = (short) LOWORD(msg.lParam);
                int physicalY = (short) HIWORD(msg.lParam);
                POINT cursorPos = {physicalX, physicalY};
                mla_ui_surface_input_event_t clickEvent = mla_ui_surface_input_event_empty();
                clickEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK;
                __skia_ScreenPosition_to_client_position(window_surface->hwnd, cursorPos, clickEvent.click.position);

                if (msg.message == WM_LBUTTONUP) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT;
                } else if (msg.message == WM_RBUTTONUP) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_RIGHT;
                } else if (msg.message == WM_MBUTTONUP) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_MIDDLE;
                }

                mla_array_list_add(eventsSinceLastFrame, clickEvent);
            } else if (msg.message == WM_KEYDOWN) {
                surface_input_event_spical_control_char_kind pressedControlKeys = MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_NONE;

                if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
                    pressedControlKeys = (surface_input_event_spical_control_char_kind)
                        (pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_SHIFT);
                }
                if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                    pressedControlKeys = (surface_input_event_spical_control_char_kind)
                        (pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_CTRL);
                }
                if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
                    pressedControlKeys = (surface_input_event_spical_control_char_kind)
                        (pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_ALT);
                }

                mla_ui_surface_input_event_char_input_kind specialKeyKind = (mla_ui_surface_input_event_char_input_kind) 0xFF;

                switch (msg.wParam) {
                    case VK_RETURN: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ENTER; break;
                    case VK_BACK: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_BACKSPACE; break;
                    case VK_DELETE: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_DELETE; break;
                    case VK_TAB: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_TAB; break;
                    case VK_ESCAPE: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ESCAPE; break;
                    case VK_UP: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_UP; break;
                    case VK_DOWN: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_DOWN; break;
                    case VK_LEFT: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_LEFT; break;
                    case VK_RIGHT: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_RIGHT; break;
                }

                if (specialKeyKind != 0xFF) {
                    mla_ui_surface_input_event_t event = mla_ui_surface_input_event_empty();
                    event.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR;
                    event.char_input.kind = specialKeyKind;
                    event.char_input.pressedControlKeys = pressedControlKeys;
                    mla_memset(event.char_input.character, 0, 4);
                    mla_array_list_add(eventsSinceLastFrame, event);
                } else {
                    BYTE keyboardState[256];
                    GetKeyboardState(keyboardState);

                    WCHAR unicodeChar[4];
                    int result = ToUnicode((UINT) msg.wParam, (UINT) ((msg.lParam >> 16) & 0xFF), keyboardState,
                                           unicodeChar, 4, 0);
                    if (result > 0) {
                        mla_ui_surface_input_event_t charInputEvent = mla_ui_surface_input_event_empty();
                        charInputEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR;
                        charInputEvent.char_input.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_INPUT;
                        charInputEvent.char_input.pressedControlKeys = pressedControlKeys;

                        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, unicodeChar, result, nullptr, 0, nullptr, nullptr);
                        if (utf8Length > 0 && utf8Length < 4) {
                            WideCharToMultiByte(CP_UTF8, 0, unicodeChar, result, charInputEvent.char_input.character,
                                                utf8Length, nullptr, nullptr);
                            charInputEvent.char_input.character[utf8Length] = '\0';
                            mla_array_list_add(eventsSinceLastFrame, charInputEvent);
                        }
                    }
                }
            }
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#else // Linux / X11

void __skia_process_x11_events(mla_skia_window_surface_t *window_surface,
                                mla_array_list_t<mla_ui_surface_input_event_t,
                                    mla_ui_surface_input_event_initializer_t> &eventsSinceLastFrame) {
    while (XPending(window_surface->display)) {
        XEvent event;
        XNextEvent(window_surface->display, &event);

        switch (event.type) {
            case ButtonRelease: {
                mla_ui_surface_input_event_t clickEvent = mla_ui_surface_input_event_empty();
                clickEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CLICK;
                clickEvent.click.position.x = (mla_double_t) event.xbutton.x;
                clickEvent.click.position.y = (mla_double_t) event.xbutton.y;

                if (event.xbutton.button == Button1) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT;
                } else if (event.xbutton.button == Button3) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_RIGHT;
                } else if (event.xbutton.button == Button2) {
                    clickEvent.click.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_MIDDLE;
                }

                mla_array_list_add(eventsSinceLastFrame, clickEvent);
                break;
            }

            case KeyPress: {
                surface_input_event_spical_control_char_kind pressedControlKeys = MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_NONE;

                if (event.xkey.state & ShiftMask) {
                    pressedControlKeys = (surface_input_event_spical_control_char_kind)
                        (pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_SHIFT);
                }
                if (event.xkey.state & ControlMask) {
                    pressedControlKeys = (surface_input_event_spical_control_char_kind)
                        (pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_CTRL);
                }
                if (event.xkey.state & Mod1Mask) {
                    pressedControlKeys = (surface_input_event_spical_control_char_kind)
                        (pressedControlKeys | MLA_UI_SURFACE_INPUT_EVENT_KIND_CONTROL_ALT);
                }

                KeySym keySym = XLookupKeysym(&event.xkey, 0);
                mla_ui_surface_input_event_char_input_kind specialKeyKind = (mla_ui_surface_input_event_char_input_kind) 0xFF;

                switch (keySym) {
                    case XK_Return: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ENTER; break;
                    case XK_BackSpace: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_BACKSPACE; break;
                    case XK_Delete: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_DELETE; break;
                    case XK_Tab: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_TAB; break;
                    case XK_Escape: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ESCAPE; break;
                    case XK_Up: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_UP; break;
                    case XK_Down: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_DOWN; break;
                    case XK_Left: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_LEFT; break;
                    case XK_Right: specialKeyKind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_ARROW_RIGHT; break;
                }

                if (specialKeyKind != 0xFF) {
                    mla_ui_surface_input_event_t charEvent = mla_ui_surface_input_event_empty();
                    charEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR;
                    charEvent.char_input.kind = specialKeyKind;
                    charEvent.char_input.pressedControlKeys = pressedControlKeys;
                    mla_memset(charEvent.char_input.character, 0, 4);
                    mla_array_list_add(eventsSinceLastFrame, charEvent);
                } else {
                    // Regular character input
                    char buffer[8];
                    int count = XLookupString(&event.xkey, buffer, sizeof(buffer) - 1, nullptr, nullptr);
                    if (count > 0 && count < 4) {
                        mla_ui_surface_input_event_t charInputEvent = mla_ui_surface_input_event_empty();
                        charInputEvent.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR;
                        charInputEvent.char_input.kind = MLA_UI_SURFACE_INPUT_EVENT_KIND_CHAR_INPUT;
                        charInputEvent.char_input.pressedControlKeys = pressedControlKeys;
                        mla_memcpy(charInputEvent.char_input.character, buffer, count);
                        charInputEvent.char_input.character[count] = '\0';
                        mla_array_list_add(eventsSinceLastFrame, charInputEvent);
                    }
                }
                break;
            }

            case ConfigureNotify: {
                window_surface->currentWidth = (mla_uint32_t) event.xconfigure.width;
                window_surface->currentHeight = (mla_uint32_t) event.xconfigure.height;
                break;
            }

            default:
                break;
        }
    }
}

#endif // _WIN32

// ============================================================================
// Main render function
// ============================================================================

mla_bool_t __skia_surface_render_draw_commands(const mla_ui_surface_t &surface,
                                                const mla_array_list_t<mla_ui_surface_draw_command_t,
                                                    mla_ui_surface_draw_command_initializer_t> &drawCommands,
                                                mla_array_list_t<mla_ui_surface_input_event_t,
                                                    mla_ui_surface_input_event_initializer_t> &eventsSinceLastFrame) {
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(surface.resource);
    if (window_surface == nullptr) return false;

    if (!window_surface->is_initialized) {
        if (!__skia_create_platform_window(window_surface)) {
            mla_warning("Failed to initialize Skia UI surface for drawing.");
            return false;
        }
    }

    // Process platform events
#ifdef _WIN32
    __skia_process_win32_events(window_surface, eventsSinceLastFrame);
#else
    __skia_process_x11_events(window_surface, eventsSinceLastFrame);
#endif

    // Get current window size
    mla_uint32_t width = 0, height = 0;
    if (!__skia_platform_get_window_size(window_surface, width, height)) return false;
    if (width == 0 || height == 0) return true;

    // Ensure GL context is current
    __skia_platform_make_current(window_surface);

    // Set viewport
    glViewport(0, 0, (int) width, (int) height);

    // Ensure GPU context and render surface
    if (!__skia_ensure_render_surface(window_surface, width, height)) return false;

    SkCanvas *canvas = window_surface->skSurface->getCanvas();
    mla_global_ui_surface_skia_cache_t &cache = window_surface->renderCache;

    // Clear background
    canvas->clear(SK_ColorWHITE);

    // Reusable paint objects from cache
    SkPaint &fillPaint = cache.fillPaint;
    SkPaint &strokePaint = cache.strokePaint;

    // Gradient state tracking
    sk_sp<SkShader> currentLinearGradient = nullptr;
    sk_sp<SkShader> currentRadialGradient = nullptr;

    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                SkRect skRect = SkRect::MakeXYWH(
                    (SkScalar) rect.x, (SkScalar) rect.y,
                    (SkScalar) rect.width, (SkScalar) rect.height
                );

                // Fill
                __skia_set_fill_color(fillPaint, rect.color);
                if (rect.rx > 0 || rect.ry > 0) {
                    canvas->drawRRect(
                        SkRRect::MakeRectXY(skRect, (SkScalar) rect.rx, (SkScalar) rect.ry),
                        fillPaint
                    );
                } else {
                    canvas->drawRect(skRect, fillPaint);
                }

                // Stroke
                if (rect.stroke_width > 0) {
                    __skia_set_stroke(strokePaint, rect.stroke, rect.stroke_width);
                    if (rect.rx > 0 || rect.ry > 0) {
                        canvas->drawRRect(
                            SkRRect::MakeRectXY(skRect, (SkScalar) rect.rx, (SkScalar) rect.ry),
                            strokePaint
                        );
                    } else {
                        canvas->drawRect(skRect, strokePaint);
                    }
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;

                // Fill
                __skia_set_fill_color(fillPaint, circle.fill);
                canvas->drawCircle(
                    (SkScalar) circle.cx, (SkScalar) circle.cy, (SkScalar) circle.r,
                    fillPaint
                );

                // Stroke
                if (circle.stroke_width > 0) {
                    __skia_set_stroke(strokePaint, circle.stroke, circle.stroke_width);
                    canvas->drawCircle(
                        (SkScalar) circle.cx, (SkScalar) circle.cy, (SkScalar) circle.r,
                        strokePaint
                    );
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipseCmd = cmd.ellipse;
                SkRect ellipseRect = SkRect::MakeXYWH(
                    (SkScalar)(ellipseCmd.cx - ellipseCmd.rx),
                    (SkScalar)(ellipseCmd.cy - ellipseCmd.ry),
                    (SkScalar)(ellipseCmd.rx * 2),
                    (SkScalar)(ellipseCmd.ry * 2)
                );

                // Fill
                __skia_set_fill_color(fillPaint, ellipseCmd.fill);
                canvas->drawOval(ellipseRect, fillPaint);

                // Stroke
                if (ellipseCmd.stroke_width > 0) {
                    __skia_set_stroke(strokePaint, ellipseCmd.stroke, ellipseCmd.stroke_width);
                    canvas->drawOval(ellipseRect, strokePaint);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                __skia_set_stroke(strokePaint, line.stroke, line.stroke_width);
                canvas->drawLine(
                    (SkScalar) line.x1, (SkScalar) line.y1,
                    (SkScalar) line.x2, (SkScalar) line.y2,
                    strokePaint
                );
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;

                SkPath path;
                const mla_ui_surface_draw_point_t &firstPt = mla_array_list_get_unsafe(polyline.points, 0);
                path.moveTo((SkScalar) firstPt.x, (SkScalar) firstPt.y);

                for (mla_size_t j = 1; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t &pt = mla_array_list_get_unsafe(polyline.points, j);
                    path.lineTo((SkScalar) pt.x, (SkScalar) pt.y);
                }

                // Stroke only for polyline
                __skia_set_stroke(strokePaint, polyline.stroke, polyline.stroke_width);
                canvas->drawPath(path, strokePaint);
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;

                SkPath path;
                const mla_ui_surface_draw_point_t &firstPt = mla_array_list_get_unsafe(polygon.points, 0);
                path.moveTo((SkScalar) firstPt.x, (SkScalar) firstPt.y);

                for (mla_size_t j = 1; j < mla_array_list_size(polygon.points); j++) {
                    const mla_ui_surface_draw_point_t &pt = mla_array_list_get_unsafe(polygon.points, j);
                    path.lineTo((SkScalar) pt.x, (SkScalar) pt.y);
                }
                path.close();

                // Fill
                __skia_set_fill_color(fillPaint, polygon.fill);
                canvas->drawPath(path, fillPaint);

                // Stroke
                if (polygon.stroke_width > 0) {
                    __skia_set_stroke(strokePaint, polygon.stroke, polygon.stroke_width);
                    canvas->drawPath(path, strokePaint);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &pathCmd = cmd.path;
                if (mla_array_list_size(pathCmd.commands) == 0) break;

                SkPath skPath;

                for (mla_size_t j = 0; j < mla_array_list_size(pathCmd.commands); j++) {
                    const auto &subCmd = mla_array_list_get_unsafe(pathCmd.commands, j);

                    switch (subCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            skPath.moveTo((SkScalar) subCmd.move_to.x, (SkScalar) subCmd.move_to.y);
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            skPath.lineTo((SkScalar) subCmd.line_to.x, (SkScalar) subCmd.line_to.y);
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO:
                            skPath.quadTo(
                                (SkScalar) subCmd.quadratic_curve_to.cpx,
                                (SkScalar) subCmd.quadratic_curve_to.cpy,
                                (SkScalar) subCmd.quadratic_curve_to.x,
                                (SkScalar) subCmd.quadratic_curve_to.y
                            );
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO:
                            skPath.cubicTo(
                                (SkScalar) subCmd.cubic_curve_to.cp1x,
                                (SkScalar) subCmd.cubic_curve_to.cp1y,
                                (SkScalar) subCmd.cubic_curve_to.cp2x,
                                (SkScalar) subCmd.cubic_curve_to.cp2y,
                                (SkScalar) subCmd.cubic_curve_to.x,
                                (SkScalar) subCmd.cubic_curve_to.y
                            );
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO:
                            skPath.arcTo(
                                (SkScalar) subCmd.arc_to.rx,
                                (SkScalar) subCmd.arc_to.ry,
                                (SkScalar) subCmd.arc_to.x_axis_rotation,
                                subCmd.arc_to.large_arc_flag ? SkPath::kLarge_ArcSize : SkPath::kSmall_ArcSize,
                                subCmd.arc_to.sweep_flag ? SkPathDirection::kCW : SkPathDirection::kCCW,
                                (SkScalar) subCmd.arc_to.x,
                                (SkScalar) subCmd.arc_to.y
                            );
                            break;

                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            skPath.close();
                            break;
                    }
                }

                // Fill
                __skia_set_fill_color(fillPaint, pathCmd.fill);
                canvas->drawPath(skPath, fillPaint);

                // Stroke
                if (pathCmd.stroke_width > 0) {
                    __skia_set_stroke(strokePaint, pathCmd.stroke, pathCmd.stroke_width);
                    canvas->drawPath(skPath, strokePaint);
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content)) break;

                SkFont *font = __mla_global_ui_surface_skia_font_cache_getOrCreateFont(
                    cache, cmd.text.font_type);

                if (font) {
                    __skia_set_fill_color(fillPaint, cmd.text.fill);

                    mla_c_string_t cStr = mla_string_to_cString(cmd.text.content);
                    if (cStr.c_str != nullptr) {
                        mla_size_t textLen = mla_strlen(cStr.c_str);

                        // Skia draws text with baseline at y, offset by ascent for top-left positioning
                        SkFontMetrics metrics;
                        font->getMetrics(&metrics);

                        canvas->drawSimpleText(
                            cStr.c_str, textLen, SkTextEncoding::kUTF8,
                            (SkScalar) cmd.text.x,
                            (SkScalar) cmd.text.y - metrics.fAscent,
                            *font, fillPaint
                        );

                        if (cStr.isOwner) {
                            mla_free(const_cast<mla_char_t *>(cStr.c_str));
                        }
                    }
                }
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT: {
                const auto &gradient = cmd.linear_gradient;
                SkPoint points[2] = {
                    SkPoint::Make((SkScalar) gradient.x1, (SkScalar) gradient.y1),
                    SkPoint::Make((SkScalar) gradient.x2, (SkScalar) gradient.y2)
                };
                SkColor colors[2] = {SK_ColorBLACK, SK_ColorWHITE};

                currentLinearGradient = SkGradientShader::MakeLinear(
                    points, colors, nullptr, 2, SkTileMode::kClamp
                );
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT: {
                const auto &gradient = cmd.radial_gradient;
                SkColor colors[2] = {SK_ColorWHITE, SK_ColorBLACK};

                currentRadialGradient = SkGradientShader::MakeRadial(
                    SkPoint::Make((SkScalar) gradient.cx, (SkScalar) gradient.cy),
                    (SkScalar) gradient.r,
                    colors, nullptr, 2, SkTileMode::kClamp
                );
                break;
            }

            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP: {
                // Note: Skia gradient stops are set during gradient creation
                // This simplified implementation matches the D2D behavior
                break;
            }

            default:
                break;
        }
    }

    // Draw FPS counter in debug builds
#ifdef mla_debug_build
    mla_uint64_t current_time = mla_system_time_ms();
    if (window_surface->DEBUG_last_fps_time == 0) {
        window_surface->DEBUG_last_fps_time = current_time;
    }

    window_surface->DEBUG_frames_accumulated++;

    if (current_time - window_surface->DEBUG_last_fps_time >= 1000) {
        window_surface->DEBUG_current_fps = window_surface->DEBUG_frames_accumulated;
        window_surface->DEBUG_frames_accumulated = 0;
        window_surface->DEBUG_last_fps_time = current_time;
    }

    // Draw FPS text
    mla_ui_surface_font_type_t debugFontType = mla_ui_surface_font_type_empty();
    debugFontType.family = mla_string_const("Arial");
    debugFontType.size = 15.0;

    SkFont *debugFont = __mla_global_ui_surface_skia_font_cache_getOrCreateFont(cache, debugFontType);
    if (debugFont) {
        SkPaint debugPaint;
        debugPaint.setAntiAlias(true);
        debugPaint.setColor(SK_ColorRED);
        debugPaint.setStyle(SkPaint::kFill_Style);

        char buffer[64];
        mla_size_t len = 0;
        const char *prefix = "FPS: ";
        while (prefix[len] != '\0') {
            buffer[len] = prefix[len];
            len++;
        }

        // Simple integer to string conversion
        mla_int32_t fps = window_surface->DEBUG_current_fps;
        if (fps == 0) {
            buffer[len++] = '0';
        } else {
            char digits[16];
            mla_size_t dLen = 0;
            while (fps > 0) {
                digits[dLen++] = (char) ('0' + (fps % 10));
                fps /= 10;
            }
            for (mla_size_t d = dLen; d > 0; d--) {
                buffer[len++] = digits[d - 1];
            }
        }
        buffer[len] = '\0';

        SkFontMetrics fpsMetrics;
        debugFont->getMetrics(&fpsMetrics);

        canvas->drawSimpleText(
            buffer, len, SkTextEncoding::kUTF8,
            (SkScalar)(width - 75),
            (SkScalar)(height - 10),
            *debugFont, debugPaint
        );
    }
#endif

    // Flush and present
    window_surface->grContext->flushAndSubmit();
    __skia_platform_swap_buffers(window_surface);

    return true;
}

// ============================================================================
// Surface cleanup
// ============================================================================

mla_buffer_cleanup_mode __skia_surface_buffer_cleanup(mla_pointer_t data, const mla_dynamic_data_t &userData) {
    (void) userData;

    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(data);

    if (window_surface != nullptr) {
        // Release Skia resources first (before destroying platform resources)
        window_surface->skSurface.reset();
        window_surface->grContext.reset();

#ifdef _WIN32
        if (window_surface->hglrc) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(window_surface->hglrc);
        }
        if (window_surface->hwnd && window_surface->hdc) {
            ReleaseDC(window_surface->hwnd, window_surface->hdc);
        }
        if (IsWindow(window_surface->hwnd)) {
            DestroyWindow(window_surface->hwnd);
        }
#else
        if (window_surface->glxContext) {
            glXMakeCurrent(window_surface->display, None, nullptr);
            glXDestroyContext(window_surface->display, window_surface->glxContext);
        }
        if (window_surface->display && window_surface->window) {
            XDestroyWindow(window_surface->display, window_surface->window);
            XFreeColormap(window_surface->display, window_surface->colormap);
            XCloseDisplay(window_surface->display);
        }
#endif

        // Call destructor for C++ members (sk_sp, SkPaint, font cache)
        window_surface->~mla_skia_window_surface_t();
    }

    return CLEAN_UP_NEEDED;
}

// ============================================================================
// Surface creation
// ============================================================================

mla_bool_t __skia_create_surface(mla_ui_surface_t &outSurface) {
    mla_skia_window_surface_t *window_surface = static_cast<mla_skia_window_surface_t *>(mla_malloc(
        sizeof(mla_skia_window_surface_t)));

    if (window_surface == nullptr) return false;

    // Use placement new to properly construct C++ members (sk_sp, SkPaint, etc.)
    new (window_surface) mla_skia_window_surface_t();
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};
    window_surface->renderCache = __mla_global_ui_surface_skia_cache_empty();
    window_surface->currentWidth = 0;
    window_surface->currentHeight = 0;

#ifdef mla_debug_build
    window_surface->DEBUG_last_fps_time = 0;
    window_surface->DEBUG_frames_accumulated = 0;
    window_surface->DEBUG_current_fps = 0;
#endif

    outSurface.resource = window_surface;
    outSurface.resourceOwner = mla_buffer_reference_create(window_surface, true, __skia_surface_buffer_cleanup, mla_dynamic_data_empty());
    outSurface.get_size = __skia_surface_get_size;
    outSurface.set_size = __skia_surface_set_size;
    outSurface.render_draw_commands = __skia_surface_render_draw_commands;
    outSurface.calc_text_size = __skia_surface_calc_text_size;
    outSurface.get_input_states = __skia_surface_input_states;

    return true;
}

// Global registration
mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    __skia_create_surface
};

// ============================================================================
// Startup and Shutdown
// ============================================================================

void __skia_init() {
    SkGraphics::Init();
}

void __skia_shutdown() {
    // Skia cleanup is handled by smart pointers
}

struct MlaSkiaAutoInit {
    MlaSkiaAutoInit() { __skia_init(); }
    ~MlaSkiaAutoInit() { __skia_shutdown(); }
};

static MlaSkiaAutoInit g_mlaSkiaAutoInit;

#endif // COREOS_MLA_GLOBAL_UI_SURFACE_SKIA_H
