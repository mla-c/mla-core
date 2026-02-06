//
// Created by chris on 1/23/2026.
// Windows OpenGL-based UI Surface Implementation
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_OPENGL_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_OPENGL_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"
#include "../../core-os/ui/display/mla_ui_display_surface.h"
#include <windows.h>
#include <gl/GL.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define mla_global_ui_surface_windows_opengl_font_cache_size 16

// Curve tessellation quality
#define MLA_BEZIER_SEGMENTS 32
#define MLA_ARC_SEGMENTS 32

// Global OpenGL context
static HGLRC g_hGLRC = nullptr;

// Gradient state
struct mla_opengl_gradient_state {
    mla_bool_t has_linear_gradient;
    mla_bool_t has_radial_gradient;
    
    // Linear gradient
    mla_double_t linear_x1, linear_y1, linear_x2, linear_y2;
    mla_ui_surface_draw_command_color_t linear_start_color;
    mla_ui_surface_draw_command_color_t linear_end_color;
    
    // Radial gradient
    mla_double_t radial_cx, radial_cy, radial_r;
    mla_ui_surface_draw_command_color_t radial_center_color;
    mla_ui_surface_draw_command_color_t radial_edge_color;
};

mla_opengl_gradient_state __mla_opengl_gradient_state_empty() {
    return {
        false, false,
        0, 0, 0, 0,
        {255, 255, 255, 255},
        {255, 255, 255, 255},
        0, 0, 0,
        {255, 255, 255, 255},
        {255, 255, 255, 255}
    };
}

// Wrapper to hold both font resources for cleanup
struct mla_opengl_font_resources {
    HFONT hFont;
    GLuint displayListBase;
    GLYPHMETRICSFLOAT glyphMetrics[256];
    mla_double_t fontHeight;
};

struct mla_global_ui_surface_windows_opengl_font_cache_item {
    mla_ui_surface_font_type_t font_type;
    mla_buffer_reference_t fontOwner;
    mla_opengl_font_resources* resources;
};

struct mla_global_ui_surface_windows_opengl_font_cache_item_initializer {
    static mla_global_ui_surface_windows_opengl_font_cache_item init() {
        return {
            mla_ui_surface_font_type_empty(),
            mla_buffer_reference_noOwner(),
            nullptr
        };
    }
};

// Render cache structure for performance optimization
struct mla_global_ui_surface_windows_opengl_Cache {
    // Font cache
    mla_array_list_t<mla_global_ui_surface_windows_opengl_font_cache_item, mla_global_ui_surface_windows_opengl_font_cache_item_initializer> fontCache;
    
    // Current rendering state
    GLfloat currentColor[4];
    
    // Gradient state
    mla_opengl_gradient_state gradientState;
};

mla_global_ui_surface_windows_opengl_Cache __mla_global_ui_surface_windows_opengl_cache_empty() {
    return {
        mla_array_list_empty<mla_global_ui_surface_windows_opengl_font_cache_item, mla_global_ui_surface_windows_opengl_font_cache_item_initializer>(),
        {1.0f, 1.0f, 1.0f, 1.0f},
        __mla_opengl_gradient_state_empty()
    };
}

mla_buffer_cleanup_mode __mla_global_ui_surface_windows_opengl_font_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    
    mla_opengl_font_resources* resources = static_cast<mla_opengl_font_resources*>(data);
    if (resources) {
        if (resources->displayListBase != 0) {
            glDeleteLists(resources->displayListBase, 256);
        }
        if (resources->hFont) {
            DeleteObject(resources->hFont);
        }
    }
    
    return CLEAN_UP_NEEDED;
}

mla_global_ui_surface_windows_opengl_font_cache_item* __mla_global_ui_surface_windows_opengl_font_cache_getOrCreateFont(
    mla_global_ui_surface_windows_opengl_Cache& cache, HDC hdc, const mla_ui_surface_font_type_t& fontType) {
    
    if (mla_string_is_empty(fontType.family))
        return nullptr;
    
    // Check if font already exists in cache
    for (mla_size_t i = 0; i < mla_array_list_size(cache.fontCache); i++) {
        mla_global_ui_surface_windows_opengl_font_cache_item& item = mla_array_list_get_unsafe(cache.fontCache, i);
        
        if (!mla_ui_surface_font_type_equals(item.font_type, fontType))
            continue;
        
        return &item;
    }
    
    mla_string_utf16_buffer_t fontFamilyWide = mla_string_to_utf16_buffer(fontType.family);
    
    if (fontFamilyWide.data == nullptr) {
        return nullptr;
    }
    
    // Determine font weight and style
    int fontWeight = FW_NORMAL;
    if (fontType.bold) {
        fontWeight = FW_BOLD;
    }
    
    BOOL italic = fontType.italic ? TRUE : FALSE;
    
    // Create new font - use exact requested size
    HFONT hFont = CreateFontW(
        -(int)fontType.size,
        0,
        0,
        0,
        fontWeight,
        italic,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        (const WCHAR*)fontFamilyWide.data
    );
    
    mla_string_utf16_buffer_destroy(fontFamilyWide);
    
    if (hFont == nullptr) {
        return nullptr;
    }
    
    // Create OpenGL display lists
    GLuint displayListBase = glGenLists(256);
    
    if (displayListBase == 0) {
        DeleteObject(hFont);
        return nullptr;
    }
    
    // Allocate resources wrapper
    mla_opengl_font_resources* resources = (mla_opengl_font_resources*)mla_malloc(sizeof(mla_opengl_font_resources));
    mla_memset(resources, 0, sizeof(mla_opengl_font_resources));
    resources->hFont = hFont;
    resources->displayListBase = displayListBase;
    
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    
    // Get font metrics
    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    resources->fontHeight = (mla_double_t)(tm.tmHeight);
    
    // Use wglUseFontOutlines for high-quality vector fonts
    wglUseFontOutlinesW(
        hdc,
        0,
        256,
        displayListBase,
        0.0f,                           // deviation (0 = highest quality)
        0.0f,                           // extrusion (0 = flat 2D)
        WGL_FONT_POLYGONS,              // format (polygons for filled text)
        resources->glyphMetrics
    );
    
    SelectObject(hdc, oldFont);
    
    mla_global_ui_surface_windows_opengl_font_cache_item newItem = {
        fontType,
        mla_buffer_reference(resources, true, __mla_global_ui_surface_windows_opengl_font_cleanup),
        resources
    };
    
    if (mla_array_list_size(cache.fontCache) < mla_global_ui_surface_windows_opengl_font_cache_size) {
        mla_array_list_add(cache.fontCache, newItem);
    } else {
        mla_global_ui_surface_windows_opengl_font_cache_item& oldItem = mla_array_list_get_unsafe(cache.fontCache, 0);
        oldItem = newItem;
    }
    
    return &mla_array_list_get_unsafe(cache.fontCache, mla_array_list_size(cache.fontCache) - 1);
}

struct mla_windows_window_surface_opengl_t {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    mla_bool_t is_initialized;
    mla_ui_surface_size_t default_size;
    
    // OpenGL rendering resources
    mla_global_ui_surface_windows_opengl_Cache renderCache;
    
#ifdef mla_debug_build
    DWORD DEBUG_last_fps_time;
    int DEBUG_frames_accumulated;
    int DEBUG_current_fps;
#endif
};

// Helper functions for OpenGL rendering
void __gl_set_color(const mla_ui_surface_draw_command_color_t &color) {
    glColor4f(
        (GLfloat)color.r / 255.0f,
        (GLfloat)color.g / 255.0f,
        (GLfloat)color.b / 255.0f,
        (GLfloat)color.a / 255.0f
    );
}

// Bezier curve evaluation
struct mla_vec2 {
    mla_double_t x, y;
};

mla_vec2 __bezier_quadratic_point(mla_vec2 p0, mla_vec2 p1, mla_vec2 p2, mla_double_t t) {
    mla_double_t u = 1.0 - t;
    mla_double_t tt = t * t;
    mla_double_t uu = u * u;
    
    mla_vec2 result;
    result.x = uu * p0.x + 2.0 * u * t * p1.x + tt * p2.x;
    result.y = uu * p0.y + 2.0 * u * t * p1.y + tt * p2.y;
    return result;
}

mla_vec2 __bezier_cubic_point(mla_vec2 p0, mla_vec2 p1, mla_vec2 p2, mla_vec2 p3, mla_double_t t) {
    mla_double_t u = 1.0 - t;
    mla_double_t tt = t * t;
    mla_double_t uu = u * u;
    mla_double_t ttt = tt * t;
    mla_double_t uuu = uu * u;
    
    mla_vec2 result;
    result.x = uuu * p0.x + 3.0 * uu * t * p1.x + 3.0 * u * tt * p2.x + ttt * p3.x;
    result.y = uuu * p0.y + 3.0 * uu * t * p1.y + 3.0 * u * tt * p2.y + ttt * p3.y;
    return result;
}

// SVG arc conversion to center parameterization
struct mla_arc_params {
    mla_double_t cx, cy;
    mla_double_t rx, ry;
    mla_double_t start_angle;
    mla_double_t sweep_angle;
    mla_double_t x_axis_rotation;
};

mla_arc_params __svg_arc_to_center_param(mla_double_t x1, mla_double_t y1, mla_double_t x2, mla_double_t y2,
                                          mla_double_t rx, mla_double_t ry, mla_double_t x_axis_rotation,
                                          mla_bool_t large_arc, mla_bool_t sweep) {
    
    mla_arc_params result = {0, 0, rx, ry, 0, 0, x_axis_rotation};
    
    if (rx == 0 || ry == 0) {
        return result;
    }
    
    rx = fabs(rx);
    ry = fabs(ry);
    
    mla_double_t phi = x_axis_rotation * M_PI / 180.0;
    mla_double_t cos_phi = cos(phi);
    mla_double_t sin_phi = sin(phi);
    
    // Compute center point
    mla_double_t dx = (x1 - x2) / 2.0;
    mla_double_t dy = (y1 - y2) / 2.0;
    
    mla_double_t x1p = cos_phi * dx + sin_phi * dy;
    mla_double_t y1p = -sin_phi * dx + cos_phi * dy;
    
    // Correct radii
    mla_double_t lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1.0) {
        rx *= sqrt(lambda);
        ry *= sqrt(lambda);
    }
    
    mla_double_t sq = (rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p) /
                      (rx * rx * y1p * y1p + ry * ry * x1p * x1p);
    
    sq = sq < 0 ? 0 : sqrt(sq);
    
    if (large_arc == sweep) {
        sq = -sq;
    }
    
    mla_double_t cxp = sq * rx * y1p / ry;
    mla_double_t cyp = -sq * ry * x1p / rx;
    
    result.cx = cos_phi * cxp - sin_phi * cyp + (x1 + x2) / 2.0;
    result.cy = sin_phi * cxp + cos_phi * cyp + (y1 + y2) / 2.0;
    
    // Compute angles
    mla_double_t ux = (x1p - cxp) / rx;
    mla_double_t uy = (y1p - cyp) / ry;
    mla_double_t vx = (-x1p - cxp) / rx;
    mla_double_t vy = (-y1p - cyp) / ry;
    
    mla_double_t n = sqrt(ux * ux + uy * uy);
    mla_double_t p = ux;
    mla_double_t sign = (uy < 0) ? -1.0 : 1.0;
    
    result.start_angle = sign * acos(p / n);
    
    n = sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    p = ux * vx + uy * vy;
    sign = (ux * vy - uy * vx < 0) ? -1.0 : 1.0;
    
    mla_double_t angle_extent = sign * acos(p / n);
    
    if (!sweep && angle_extent > 0) {
        angle_extent -= 2.0 * M_PI;
    } else if (sweep && angle_extent < 0) {
        angle_extent += 2.0 * M_PI;
    }
    
    result.sweep_angle = angle_extent;
    result.rx = rx;
    result.ry = ry;
    
    return result;
}

void __gl_draw_arc(mla_double_t x1, mla_double_t y1, mla_double_t x2, mla_double_t y2,
                   mla_double_t rx, mla_double_t ry, mla_double_t x_axis_rotation,
                   mla_bool_t large_arc, mla_bool_t sweep, mla_bool_t use_line_to) {
    
    mla_arc_params arc = __svg_arc_to_center_param(x1, y1, x2, y2, rx, ry, x_axis_rotation, large_arc, sweep);
    
    mla_double_t phi = arc.x_axis_rotation * M_PI / 180.0;
    mla_double_t cos_phi = cos(phi);
    mla_double_t sin_phi = sin(phi);
    
    int segments = MLA_ARC_SEGMENTS;
    for (int i = 0; i <= segments; i++) {
        mla_double_t t = (mla_double_t)i / (mla_double_t)segments;
        mla_double_t angle = arc.start_angle + t * arc.sweep_angle;
        
        mla_double_t x = arc.rx * cos(angle);
        mla_double_t y = arc.ry * sin(angle);
        
        // Apply rotation
        mla_double_t xr = cos_phi * x - sin_phi * y + arc.cx;
        mla_double_t yr = sin_phi * x + cos_phi * y + arc.cy;
        
        if (use_line_to || i > 0) {
            glVertex2f((GLfloat)xr, (GLfloat)yr);
        }
    }
}

// Linear gradient rendering using vertex colors
void __gl_apply_linear_gradient(mla_opengl_gradient_state& gradient, mla_double_t x, mla_double_t y) {
    if (!gradient.has_linear_gradient) return;
    
    // Calculate gradient position (0 to 1)
    mla_double_t dx = gradient.linear_x2 - gradient.linear_x1;
    mla_double_t dy = gradient.linear_y2 - gradient.linear_y1;
    mla_double_t len = sqrt(dx * dx + dy * dy);
    
    if (len < 0.001) {
        __gl_set_color(gradient.linear_start_color);
        return;
    }
    
    mla_double_t px = x - gradient.linear_x1;
    mla_double_t py = y - gradient.linear_y1;
    
    mla_double_t t = (px * dx + py * dy) / (len * len);
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    
    // Interpolate color
    mla_ui_surface_draw_command_color_t color;
    color.r = (mla_uint8_t)(gradient.linear_start_color.r * (1.0 - t) + gradient.linear_end_color.r * t);
    color.g = (mla_uint8_t)(gradient.linear_start_color.g * (1.0 - t) + gradient.linear_end_color.g * t);
    color.b = (mla_uint8_t)(gradient.linear_start_color.b * (1.0 - t) + gradient.linear_end_color.b * t);
    color.a = (mla_uint8_t)(gradient.linear_start_color.a * (1.0 - t) + gradient.linear_end_color.a * t);
    
    __gl_set_color(color);
}

// Radial gradient rendering using vertex colors
void __gl_apply_radial_gradient(mla_opengl_gradient_state& gradient, mla_double_t x, mla_double_t y) {
    if (!gradient.has_radial_gradient) return;
    
    mla_double_t dx = x - gradient.radial_cx;
    mla_double_t dy = y - gradient.radial_cy;
    mla_double_t dist = sqrt(dx * dx + dy * dy);
    
    mla_double_t t = dist / gradient.radial_r;
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    
    // Interpolate color
    mla_ui_surface_draw_command_color_t color;
    color.r = (mla_uint8_t)(gradient.radial_center_color.r * (1.0 - t) + gradient.radial_edge_color.r * t);
    color.g = (mla_uint8_t)(gradient.radial_center_color.g * (1.0 - t) + gradient.radial_edge_color.g * t);
    color.b = (mla_uint8_t)(gradient.radial_center_color.b * (1.0 - t) + gradient.radial_edge_color.b * t);
    color.a = (mla_uint8_t)(gradient.radial_center_color.a * (1.0 - t) + gradient.radial_edge_color.a * t);
    
    __gl_set_color(color);
}

void __gl_draw_circle(GLfloat cx, GLfloat cy, GLfloat radius, int segments = 64) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        GLfloat angle = 2.0f * (GLfloat)M_PI * (GLfloat)i / (GLfloat)segments;
        glVertex2f(cx + radius * cosf(angle), cy + radius * sinf(angle));
    }
    glEnd();
}

void __gl_draw_circle_outline(GLfloat cx, GLfloat cy, GLfloat radius, int segments = 64) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        GLfloat angle = 2.0f * (GLfloat)M_PI * (GLfloat)i / (GLfloat)segments;
        glVertex2f(cx + radius * cosf(angle), cy + radius * sinf(angle));
    }
    glEnd();
}

void __gl_draw_ellipse(GLfloat cx, GLfloat cy, GLfloat rx, GLfloat ry, int segments = 64) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        GLfloat angle = 2.0f * (GLfloat)M_PI * (GLfloat)i / (GLfloat)segments;
        glVertex2f(cx + rx * cosf(angle), cy + ry * sinf(angle));
    }
    glEnd();
}

void __gl_draw_ellipse_outline(GLfloat cx, GLfloat cy, GLfloat rx, GLfloat ry, int segments = 64) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        GLfloat angle = 2.0f * (GLfloat)M_PI * (GLfloat)i / (GLfloat)segments;
        glVertex2f(cx + rx * cosf(angle), cy + ry * sinf(angle));
    }
    glEnd();
}

void __gl_draw_rounded_rect(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat rx, GLfloat ry) {
    const int segments = 16;
    
    glBegin(GL_POLYGON);
    
    // Top-left corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = (GLfloat)M_PI + (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + rx + rx * cosf(angle), y + ry + ry * sinf(angle));
    }
    
    // Top-right corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = 1.5f * (GLfloat)M_PI + (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + width - rx + rx * cosf(angle), y + ry + ry * sinf(angle));
    }
    
    // Bottom-right corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + width - rx + rx * cosf(angle), y + height - ry + ry * sinf(angle));
    }
    
    // Bottom-left corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = (GLfloat)M_PI / 2.0f + (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + rx + rx * cosf(angle), y + height - ry + ry * sinf(angle));
    }
    
    glEnd();
}

void __gl_draw_rounded_rect_outline(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat rx, GLfloat ry, GLfloat stroke_width) {
    glLineWidth(stroke_width);
    glBegin(GL_LINE_LOOP);
    
    const int segments = 16;
    
    // Top-left corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = (GLfloat)M_PI + (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + rx + rx * cosf(angle), y + ry + ry * sinf(angle));
    }
    
    // Top-right corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = 1.5f * (GLfloat)M_PI + (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + width - rx + rx * cosf(angle), y + ry + ry * sinf(angle));
    }
    
    // Bottom-right corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + width - rx + rx * cosf(angle), y + height - ry + ry * sinf(angle));
    }
    
    // Bottom-left corner
    for (int i = segments; i >= 0; i--) {
        GLfloat angle = (GLfloat)M_PI / 2.0f + (GLfloat)i * ((GLfloat)M_PI / 2.0f) / (GLfloat)segments;
        glVertex2f(x + rx + rx * cosf(angle), y + height - ry + ry * sinf(angle));
    }
    
    glEnd();
}

// Stub implementations for the surface function pointers
mla_ui_surface_size_t __windows_surface_opengl_get_size(const mla_ui_surface_t &surface) {
    mla_ui_surface_size_t size = {0, 0};
    
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(surface.resource);
    
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

mla_bool_t __windows_surface_opengl_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(surface.resource);
    
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
    
    if (SetWindowPos(window_surface->hwnd, nullptr, 0, 0, (int)size.width, (int)size.height,
                     SWP_NOMOVE | SWP_NOZORDER)) {
        return true;
    }
    
    return false;
}

LRESULT CALLBACK __windows_surface_opengl_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

mla_ui_surface_input_states_t __windows_surface_opengl_input_states(const mla_ui_surface_t &surface) {
    
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(surface.resource);
    
    // Validate surface state
    if (window_surface == nullptr || !window_surface->is_initialized || !IsWindow(window_surface->hwnd)) {
        return inputStates;
    }
    
    // 1. Mouse Position
    POINT cursorPos;
    RECT clientRect;
    if (GetCursorPos(&cursorPos) && ScreenToClient(window_surface->hwnd, &cursorPos) && GetClientRect(window_surface->hwnd, &clientRect)) {
        if (PtInRect(&clientRect, cursorPos)) {
            inputStates.cursorPosition.x = (mla_double_t)cursorPos.x;
            inputStates.cursorPosition.y = (mla_double_t)cursorPos.y;
        }
    }
    
    // 2. Mouse Buttons
    inputStates.leftMouseButtonDown   = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
    inputStates.rightMouseButtonDown  = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;
    inputStates.middleMouseButtonDown = (GetKeyState(VK_MBUTTON) & 0x8000) != 0;
    
    // 3. Modifier Keys
    inputStates.shiftKeyDown = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
    inputStates.ctrlKeyDown  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    inputStates.altKeyDown   = (GetKeyState(VK_MENU)    & 0x8000) != 0;
    inputStates.metaKeyDown  = ((GetKeyState(VK_LWIN)   & 0x8000) != 0) ||
                               ((GetKeyState(VK_RWIN)   & 0x8000) != 0);
    
    // 4. Key Code Down (basic polling)
    inputStates.keyCodeDown = 0;
    for (int key = 0x08; key <= 0xFE; ++key) {
        if ((key >= VK_LBUTTON && key <= VK_XBUTTON2)) {
            continue; // skip mouse buttons
        }
        if (GetKeyState(key) & 0x8000) {
            inputStates.keyCodeDown = key;
            break;
        }
    }
    
    return inputStates;
}

mla_bool_t __windows_create_windows_opengl_surface(mla_windows_window_surface_opengl_t *surface) {
    const char CLASS_NAME[] = "MLA_Window_OpenGL_Class";
    
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = __windows_surface_opengl_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.style = CS_OWNDC;
    
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
    
    // Get device context
    HDC hdc = GetDC(hwnd);
    
    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat) {
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return false;
    }
    
    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return false;
    }
    
    // Create OpenGL context
    HGLRC hglrc = wglCreateContext(hdc);
    if (!hglrc) {
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return false;
    }
    
    wglMakeCurrent(hdc, hglrc);
    
    surface->renderCache = __mla_global_ui_surface_windows_opengl_cache_empty();
    surface->is_initialized = true;
    surface->hwnd = hwnd;
    surface->hdc = hdc;
    surface->hglrc = hglrc;
    
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    
    return true;
}

mla_ui_surface_draw_size_t __windows_surface_opengl_calc_text_size(const mla_ui_surface_t &surface, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {
    mla_ui_surface_draw_size_t size = {0, 0};
    
    if (mla_string_is_empty(text)) {
        return size;
    }
    
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(surface.resource);
    
    if (window_surface == nullptr || !window_surface->is_initialized) {
        return size;
    }
    
    mla_global_ui_surface_windows_opengl_font_cache_item* fontItem = 
        __mla_global_ui_surface_windows_opengl_font_cache_getOrCreateFont(
            window_surface->renderCache, window_surface->hdc, font_type);
    
    if (fontItem && fontItem->resources && fontItem->resources->hFont) {
        HDC hdc = window_surface->hdc;
        HFONT oldFont = (HFONT)SelectObject(hdc, fontItem->resources->hFont);
        
        mla_string_utf16_buffer_t textWide = mla_string_to_utf16_buffer(text);
        
        SIZE textSize;
        if (GetTextExtentPoint32W(hdc, (const WCHAR*)textWide.data, (int)wcslen((const WCHAR*)textWide.data), &textSize)) {
            size.width = (mla_double_t)textSize.cx;
            size.height = (mla_double_t)textSize.cy;
        }
        
        SelectObject(hdc, oldFont);
        mla_string_utf16_buffer_destroy(textWide);
    }
    
    return size;
}

mla_bool_t __windows_surface_opengl_render_draw_commands(const mla_ui_surface_t &surface,
                                                         const mla_array_list_t<mla_ui_surface_draw_command_t,
                                                             mla_ui_surface_draw_command_initializer_t> &drawCommands) {
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(surface.resource);
    if (window_surface == nullptr) {
        return false;
    }
    
    if (!window_surface->is_initialized) {
        if (!__windows_create_windows_opengl_surface(window_surface)) {
            mla_warning("Failed to initialize Windows OpenGL UI surface for drawing.");
            return false;
        }
    }
    
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Make context current
    wglMakeCurrent(window_surface->hdc, window_surface->hglrc);
    
    // Get window size for viewport
    RECT rc;
    GetClientRect(window_surface->hwnd, &rc);
    int windowWidth = rc.right - rc.left;
    int windowHeight = rc.bottom - rc.top;
    
    // Setup viewport and projection
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Disable depth test for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Clear background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Process draw commands
    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);
        
        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                const auto &rect = cmd.rect;
                GLfloat x = (GLfloat)rect.x;
                GLfloat y = (GLfloat)rect.y;
                GLfloat w = (GLfloat)rect.width;
                GLfloat h = (GLfloat)rect.height;
                
                // Fill with gradient support
                if (window_surface->renderCache.gradientState.has_linear_gradient || 
                    window_surface->renderCache.gradientState.has_radial_gradient) {
                    
                    glShadeModel(GL_SMOOTH);
                    
                    if (rect.rx > 0 || rect.ry > 0) {
                        __gl_draw_rounded_rect(x, y, w, h, (GLfloat)rect.rx, (GLfloat)rect.ry);
                    } else {
                        glBegin(GL_QUADS);
                        
                        if (window_surface->renderCache.gradientState.has_linear_gradient) {
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, x, y);
                            glVertex2f(x, y);
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, x + w, y);
                            glVertex2f(x + w, y);
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, x + w, y + h);
                            glVertex2f(x + w, y + h);
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, x, y + h);
                            glVertex2f(x, y + h);
                        } else {
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, x, y);
                            glVertex2f(x, y);
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, x + w, y);
                            glVertex2f(x + w, y);
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, x + w, y + h);
                            glVertex2f(x + w, y + h);
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, x, y + h);
                            glVertex2f(x, y + h);
                        }
                        
                        glEnd();
                    }
                    
                    glShadeModel(GL_FLAT);
                } else {
                    // Solid color fill
                    __gl_set_color(rect.color);
                    if (rect.rx > 0 || rect.ry > 0) {
                        __gl_draw_rounded_rect(x, y, w, h, (GLfloat)rect.rx, (GLfloat)rect.ry);
                    } else {
                        glBegin(GL_QUADS);
                        glVertex2f(x, y);
                        glVertex2f(x + w, y);
                        glVertex2f(x + w, y + h);
                        glVertex2f(x, y + h);
                        glEnd();
                    }
                }
                
                // Stroke
                if (rect.stroke_width > 0) {
                    __gl_set_color(rect.stroke);
                    if (rect.rx > 0 || rect.ry > 0) {
                        __gl_draw_rounded_rect_outline(x, y, w, h, (GLfloat)rect.rx, (GLfloat)rect.ry, (GLfloat)rect.stroke_width);
                    } else {
                        glLineWidth((GLfloat)rect.stroke_width);
                        glBegin(GL_LINE_LOOP);
                        glVertex2f(x, y);
                        glVertex2f(x + w, y);
                        glVertex2f(x + w, y + h);
                        glVertex2f(x, y + h);
                        glEnd();
                    }
                }
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                const auto &circle = cmd.circle;
                
                if (window_surface->renderCache.gradientState.has_linear_gradient || 
                    window_surface->renderCache.gradientState.has_radial_gradient) {
                    
                    glShadeModel(GL_SMOOTH);
                    glBegin(GL_TRIANGLE_FAN);
                    
                    GLfloat cx = (GLfloat)circle.cx;
                    GLfloat cy = (GLfloat)circle.cy;
                    GLfloat r = (GLfloat)circle.r;
                    
                    if (window_surface->renderCache.gradientState.has_linear_gradient) {
                        __gl_apply_linear_gradient(window_surface->renderCache.gradientState, cx, cy);
                    } else {
                        __gl_apply_radial_gradient(window_surface->renderCache.gradientState, cx, cy);
                    }
                    glVertex2f(cx, cy);
                    
                    for (int i = 0; i <= 64; i++) {
                        GLfloat angle = 2.0f * (GLfloat)M_PI * (GLfloat)i / 64.0f;
                        GLfloat px = cx + r * cosf(angle);
                        GLfloat py = cy + r * sinf(angle);
                        
                        if (window_surface->renderCache.gradientState.has_linear_gradient) {
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, px, py);
                        } else {
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, px, py);
                        }
                        glVertex2f(px, py);
                    }
                    glEnd();
                    glShadeModel(GL_FLAT);
                } else {
                    __gl_set_color(circle.fill);
                    __gl_draw_circle((GLfloat)circle.cx, (GLfloat)circle.cy, (GLfloat)circle.r);
                }
                
                if (circle.stroke_width > 0) {
                    __gl_set_color(circle.stroke);
                    glLineWidth((GLfloat)circle.stroke_width);
                    __gl_draw_circle_outline((GLfloat)circle.cx, (GLfloat)circle.cy, (GLfloat)circle.r);
                }
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                const auto &ellipse = cmd.ellipse;
                
                if (window_surface->renderCache.gradientState.has_linear_gradient || 
                    window_surface->renderCache.gradientState.has_radial_gradient) {
                    
                    glShadeModel(GL_SMOOTH);
                    glBegin(GL_TRIANGLE_FAN);
                    
                    GLfloat cx = (GLfloat)ellipse.cx;
                    GLfloat cy = (GLfloat)ellipse.cy;
                    GLfloat rx = (GLfloat)ellipse.rx;
                    GLfloat ry = (GLfloat)ellipse.ry;
                    
                    if (window_surface->renderCache.gradientState.has_linear_gradient) {
                        __gl_apply_linear_gradient(window_surface->renderCache.gradientState, cx, cy);
                    } else {
                        __gl_apply_radial_gradient(window_surface->renderCache.gradientState, cx, cy);
                    }
                    glVertex2f(cx, cy);
                    
                    for (int i = 0; i <= 64; i++) {
                        GLfloat angle = 2.0f * (GLfloat)M_PI * (GLfloat)i / 64.0f;
                        GLfloat px = cx + rx * cosf(angle);
                        GLfloat py = cy + ry * sinf(angle);
                        
                        if (window_surface->renderCache.gradientState.has_linear_gradient) {
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, px, py);
                        } else {
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, px, py);
                        }
                        glVertex2f(px, py);
                    }
                    glEnd();
                    glShadeModel(GL_FLAT);
                } else {
                    __gl_set_color(ellipse.fill);
                    __gl_draw_ellipse((GLfloat)ellipse.cx, (GLfloat)ellipse.cy, (GLfloat)ellipse.rx, (GLfloat)ellipse.ry);
                }
                
                if (ellipse.stroke_width > 0) {
                    __gl_set_color(ellipse.stroke);
                    glLineWidth((GLfloat)ellipse.stroke_width);
                    __gl_draw_ellipse_outline((GLfloat)ellipse.cx, (GLfloat)ellipse.cy, (GLfloat)ellipse.rx, (GLfloat)ellipse.ry);
                }
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                const auto &line = cmd.line;
                __gl_set_color(line.stroke);
                glLineWidth((GLfloat)line.stroke_width);
                glBegin(GL_LINES);
                glVertex2f((GLfloat)line.x1, (GLfloat)line.y1);
                glVertex2f((GLfloat)line.x2, (GLfloat)line.y2);
                glEnd();
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYLINE: {
                const auto &polyline = cmd.polyline;
                if (mla_array_list_size(polyline.points) < 2) break;
                
                __gl_set_color(polyline.stroke);
                glLineWidth((GLfloat)polyline.stroke_width);
                glBegin(GL_LINE_STRIP);
                for (mla_size_t j = 0; j < mla_array_list_size(polyline.points); j++) {
                    const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polyline.points, j);
                    glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                }
                glEnd();
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_POLYGON: {
                const auto &polygon = cmd.polygon;
                if (mla_array_list_size(polygon.points) < 3) break;
                
                // Fill with gradient support
                if (window_surface->renderCache.gradientState.has_linear_gradient || 
                    window_surface->renderCache.gradientState.has_radial_gradient) {
                    
                    glShadeModel(GL_SMOOTH);
                    glBegin(GL_POLYGON);
                    for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                        const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                        
                        if (window_surface->renderCache.gradientState.has_linear_gradient) {
                            __gl_apply_linear_gradient(window_surface->renderCache.gradientState, point.x, point.y);
                        } else {
                            __gl_apply_radial_gradient(window_surface->renderCache.gradientState, point.x, point.y);
                        }
                        glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                    }
                    glEnd();
                    glShadeModel(GL_FLAT);
                } else {
                    // Solid fill
                    __gl_set_color(polygon.fill);
                    glBegin(GL_POLYGON);
                    for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                        const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                        glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                    }
                    glEnd();
                }
                
                // Stroke
                if (polygon.stroke_width > 0) {
                    __gl_set_color(polygon.stroke);
                    glLineWidth((GLfloat)polygon.stroke_width);
                    glBegin(GL_LINE_LOOP);
                    for (mla_size_t j = 0; j < mla_array_list_size(polygon.points); j++) {
                        const mla_ui_surface_draw_point_t &point = mla_array_list_get_unsafe(polygon.points, j);
                        glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                    }
                    glEnd();
                }
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_PATH: {
                const auto &path = cmd.path;
                if (mla_array_list_size(path.commands) == 0) break;
                
                GLfloat currentX = 0, currentY = 0;
                GLfloat startX = 0, startY = 0;
                mla_bool_t pathStarted = false;
                
                // Fill pass with proper bezier curves
                glShadeModel(GL_SMOOTH);
                glBegin(GL_POLYGON);
                
                for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                    const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);
                    
                    switch (pathCmd.kind) {
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                            if (pathStarted) {
                                glEnd();
                                glBegin(GL_POLYGON);
                            }
                            currentX = (GLfloat)pathCmd.move_to.x;
                            currentY = (GLfloat)pathCmd.move_to.y;
                            startX = currentX;
                            startY = currentY;
                            
                            if (window_surface->renderCache.gradientState.has_linear_gradient) {
                                __gl_apply_linear_gradient(window_surface->renderCache.gradientState, currentX, currentY);
                            } else if (window_surface->renderCache.gradientState.has_radial_gradient) {
                                __gl_apply_radial_gradient(window_surface->renderCache.gradientState, currentX, currentY);
                            } else {
                                __gl_set_color(path.fill);
                            }
                            
                            glVertex2f(currentX, currentY);
                            pathStarted = true;
                            break;
                        
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                            currentX = (GLfloat)pathCmd.line_to.x;
                            currentY = (GLfloat)pathCmd.line_to.y;
                            
                            if (window_surface->renderCache.gradientState.has_linear_gradient) {
                                __gl_apply_linear_gradient(window_surface->renderCache.gradientState, currentX, currentY);
                            } else if (window_surface->renderCache.gradientState.has_radial_gradient) {
                                __gl_apply_radial_gradient(window_surface->renderCache.gradientState, currentX, currentY);
                            }
                            
                            glVertex2f(currentX, currentY);
                            break;
                        
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO: {
                            mla_vec2 p0 = {currentX, currentY};
                            mla_vec2 p1 = {pathCmd.quadratic_curve_to.cpx, pathCmd.quadratic_curve_to.cpy};
                            mla_vec2 p2 = {pathCmd.quadratic_curve_to.x, pathCmd.quadratic_curve_to.y};
                            
                            for (int k = 1; k <= MLA_BEZIER_SEGMENTS; k++) {
                                mla_double_t t = (mla_double_t)k / (mla_double_t)MLA_BEZIER_SEGMENTS;
                                mla_vec2 point = __bezier_quadratic_point(p0, p1, p2, t);
                                
                                if (window_surface->renderCache.gradientState.has_linear_gradient) {
                                    __gl_apply_linear_gradient(window_surface->renderCache.gradientState, point.x, point.y);
                                } else if (window_surface->renderCache.gradientState.has_radial_gradient) {
                                    __gl_apply_radial_gradient(window_surface->renderCache.gradientState, point.x, point.y);
                                }
                                
                                glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                            }
                            
                            currentX = (GLfloat)p2.x;
                            currentY = (GLfloat)p2.y;
                            break;
                        }
                        
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO: {
                            mla_vec2 p0 = {currentX, currentY};
                            mla_vec2 p1 = {pathCmd.cubic_curve_to.cp1x, pathCmd.cubic_curve_to.cp1y};
                            mla_vec2 p2 = {pathCmd.cubic_curve_to.cp2x, pathCmd.cubic_curve_to.cp2y};
                            mla_vec2 p3 = {pathCmd.cubic_curve_to.x, pathCmd.cubic_curve_to.y};
                            
                            for (int k = 1; k <= MLA_BEZIER_SEGMENTS; k++) {
                                mla_double_t t = (mla_double_t)k / (mla_double_t)MLA_BEZIER_SEGMENTS;
                                mla_vec2 point = __bezier_cubic_point(p0, p1, p2, p3, t);
                                
                                if (window_surface->renderCache.gradientState.has_linear_gradient) {
                                    __gl_apply_linear_gradient(window_surface->renderCache.gradientState, point.x, point.y);
                                } else if (window_surface->renderCache.gradientState.has_radial_gradient) {
                                    __gl_apply_radial_gradient(window_surface->renderCache.gradientState, point.x, point.y);
                                }
                                
                                glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                            }
                            
                            currentX = (GLfloat)p3.x;
                            currentY = (GLfloat)p3.y;
                            break;
                        }
                        
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO: {
                            __gl_draw_arc(currentX, currentY,
                                        pathCmd.arc_to.x, pathCmd.arc_to.y,
                                        pathCmd.arc_to.rx, pathCmd.arc_to.ry,
                                        pathCmd.arc_to.x_axis_rotation,
                                        pathCmd.arc_to.large_arc_flag,
                                        pathCmd.arc_to.sweep_flag,
                                        true);
                            
                            currentX = (GLfloat)pathCmd.arc_to.x;
                            currentY = (GLfloat)pathCmd.arc_to.y;
                            break;
                        }
                        
                        case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                            if (pathStarted) {
                                if (window_surface->renderCache.gradientState.has_linear_gradient) {
                                    __gl_apply_linear_gradient(window_surface->renderCache.gradientState, startX, startY);
                                } else if (window_surface->renderCache.gradientState.has_radial_gradient) {
                                    __gl_apply_radial_gradient(window_surface->renderCache.gradientState, startX, startY);
                                }
                                glVertex2f(startX, startY);
                            }
                            break;
                    }
                }
                glEnd();
                
                // Stroke pass with proper bezier curves
                if (path.stroke_width > 0) {
                    __gl_set_color(path.stroke);
                    glLineWidth((GLfloat)path.stroke_width);
                    glBegin(GL_LINE_STRIP);
                    
                    pathStarted = false;
                    currentX = 0;
                    currentY = 0;
                    
                    for (mla_size_t j = 0; j < mla_array_list_size(path.commands); j++) {
                        const auto &pathCmd = mla_array_list_get_unsafe(path.commands, j);
                        
                        switch (pathCmd.kind) {
                            case MLA_UI_SURFACE_DRAW_PATH_COMMAND_MOVE_TO:
                                if (pathStarted) {
                                    glEnd();
                                    glBegin(GL_LINE_STRIP);
                                }
                                currentX = (GLfloat)pathCmd.move_to.x;
                                currentY = (GLfloat)pathCmd.move_to.y;
                                startX = currentX;
                                startY = currentY;
                                glVertex2f(currentX, currentY);
                                pathStarted = true;
                                break;
                            
                            case MLA_UI_SURFACE_DRAW_PATH_COMMAND_LINE_TO:
                                currentX = (GLfloat)pathCmd.line_to.x;
                                currentY = (GLfloat)pathCmd.line_to.y;
                                glVertex2f(currentX, currentY);
                                break;
                            
                            case MLA_UI_SURFACE_DRAW_PATH_COMMAND_QUADRATIC_CURVE_TO: {
                                mla_vec2 p0 = {currentX, currentY};
                                mla_vec2 p1 = {pathCmd.quadratic_curve_to.cpx, pathCmd.quadratic_curve_to.cpy};
                                mla_vec2 p2 = {pathCmd.quadratic_curve_to.x, pathCmd.quadratic_curve_to.y};
                                
                                for (int k = 1; k <= MLA_BEZIER_SEGMENTS; k++) {
                                    mla_double_t t = (mla_double_t)k / (mla_double_t)MLA_BEZIER_SEGMENTS;
                                    mla_vec2 point = __bezier_quadratic_point(p0, p1, p2, t);
                                    glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                                }
                                
                                currentX = (GLfloat)p2.x;
                                currentY = (GLfloat)p2.y;
                                break;
                            }
                            
                            case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CUBIC_CURVE_TO: {
                                mla_vec2 p0 = {currentX, currentY};
                                mla_vec2 p1 = {pathCmd.cubic_curve_to.cp1x, pathCmd.cubic_curve_to.cp1y};
                                mla_vec2 p2 = {pathCmd.cubic_curve_to.cp2x, pathCmd.cubic_curve_to.cp2y};
                                mla_vec2 p3 = {pathCmd.cubic_curve_to.x, pathCmd.cubic_curve_to.y};
                                
                                for (int k = 1; k <= MLA_BEZIER_SEGMENTS; k++) {
                                    mla_double_t t = (mla_double_t)k / (mla_double_t)MLA_BEZIER_SEGMENTS;
                                    mla_vec2 point = __bezier_cubic_point(p0, p1, p2, p3, t);
                                    glVertex2f((GLfloat)point.x, (GLfloat)point.y);
                                }
                                
                                currentX = (GLfloat)p3.x;
                                currentY = (GLfloat)p3.y;
                                break;
                            }
                            
                            case MLA_UI_SURFACE_DRAW_PATH_COMMAND_ARC_TO: {
                                __gl_draw_arc(currentX, currentY,
                                            pathCmd.arc_to.x, pathCmd.arc_to.y,
                                            pathCmd.arc_to.rx, pathCmd.arc_to.ry,
                                            pathCmd.arc_to.x_axis_rotation,
                                            pathCmd.arc_to.large_arc_flag,
                                            pathCmd.arc_to.sweep_flag,
                                            true);
                                
                                currentX = (GLfloat)pathCmd.arc_to.x;
                                currentY = (GLfloat)pathCmd.arc_to.y;
                                break;
                            }
                            
                            case MLA_UI_SURFACE_DRAW_PATH_COMMAND_CLOSE_PATH:
                                if (pathStarted) {
                                    glVertex2f(startX, startY);
                                }
                                break;
                        }
                    }
                    glEnd();
                }
                
                glShadeModel(GL_FLAT);
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                if (mla_string_is_empty(cmd.text.content))
                    break;
                
                mla_global_ui_surface_windows_opengl_font_cache_item* fontItem = 
                    __mla_global_ui_surface_windows_opengl_font_cache_getOrCreateFont(
                        window_surface->renderCache, window_surface->hdc, cmd.text.font_type);
                
                if (fontItem && fontItem->resources && fontItem->resources->displayListBase != 0) {
                    // Set text color
                    __gl_set_color(cmd.text.fill);
                    
                    // Save current matrix
                    glPushMatrix();
                    
                    // Position text - Direct2D style (top-left origin)
                    glTranslatef((GLfloat)cmd.text.x, (GLfloat)cmd.text.y, 0.0f);
                    
                    // Scale for proper font size - wglUseFontOutlines creates very small glyphs
                    // Need to scale up significantly (empirically determined multiplier)
                    GLfloat scale = (GLfloat)cmd.text.font_type.size * 1.5f;
                    glScalef(scale, -scale, 1.0f);  // Negative Y to flip text right-side up
                    
                    // Convert text to UTF-16 for Windows
                    mla_string_utf16_buffer_t contentWide = mla_string_to_utf16_buffer(cmd.text.content);
                    
                    // Set the display list base
                    glListBase(fontItem->resources->displayListBase);
                    
                    mla_size_t len = wcslen((const WCHAR*)contentWide.data);
                    
                    // Convert to single-byte for display list rendering
                    char* asciiText = (char*)mla_malloc(len + 1);
                    for (mla_size_t i = 0; i < len; i++) {
                        wchar_t wc = ((const WCHAR*)contentWide.data)[i];
                        asciiText[i] = (wc < 256) ? (char)wc : '?';
                    }
                    asciiText[len] = '\0';
                    
                    // Draw the text using outline font display lists
                    glCallLists((GLsizei)len, GL_UNSIGNED_BYTE, asciiText);
                    
                    // Restore matrix
                    glPopMatrix();
                    
                    mla_free(asciiText);
                    mla_string_utf16_buffer_destroy(contentWide);
                }
                
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINEAR_GRADIENT: {
                const auto &gradient = cmd.linear_gradient;
                
                window_surface->renderCache.gradientState.has_linear_gradient = true;
                window_surface->renderCache.gradientState.has_radial_gradient = false;
                window_surface->renderCache.gradientState.linear_x1 = gradient.x1;
                window_surface->renderCache.gradientState.linear_y1 = gradient.y1;
                window_surface->renderCache.gradientState.linear_x2 = gradient.x2;
                window_surface->renderCache.gradientState.linear_y2 = gradient.y2;
                
                // Default colors (will be updated by STOP commands)
                window_surface->renderCache.gradientState.linear_start_color = {0, 0, 0, 255};
                window_surface->renderCache.gradientState.linear_end_color = {255, 255, 255, 255};
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RADIAL_GRADIENT: {
                const auto &gradient = cmd.radial_gradient;
                
                window_surface->renderCache.gradientState.has_linear_gradient = false;
                window_surface->renderCache.gradientState.has_radial_gradient = true;
                window_surface->renderCache.gradientState.radial_cx = gradient.cx;
                window_surface->renderCache.gradientState.radial_cy = gradient.cy;
                window_surface->renderCache.gradientState.radial_r = gradient.r;
                
                // Default colors (will be updated by STOP commands)
                window_surface->renderCache.gradientState.radial_center_color = {255, 255, 255, 255};
                window_surface->renderCache.gradientState.radial_edge_color = {0, 0, 0, 255};
                break;
            }
            
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_STOP: {
                const auto &stop = cmd.stop;
                
                // Update gradient colors based on offset
                if (window_surface->renderCache.gradientState.has_linear_gradient) {
                    if (stop.offset <= 0.0) {
                        window_surface->renderCache.gradientState.linear_start_color = stop.stop_color;
                    } else if (stop.offset >= 1.0) {
                        window_surface->renderCache.gradientState.linear_end_color = stop.stop_color;
                    } else {
                        // For intermediate stops, we'll use the last one before offset 1.0 as end
                        if (stop.offset > 0.5) {
                            window_surface->renderCache.gradientState.linear_end_color = stop.stop_color;
                        } else {
                            window_surface->renderCache.gradientState.linear_start_color = stop.stop_color;
                        }
                    }
                } else if (window_surface->renderCache.gradientState.has_radial_gradient) {
                    if (stop.offset <= 0.0) {
                        window_surface->renderCache.gradientState.radial_center_color = stop.stop_color;
                    } else if (stop.offset >= 1.0) {
                        window_surface->renderCache.gradientState.radial_edge_color = stop.stop_color;
                    } else {
                        if (stop.offset > 0.5) {
                            window_surface->renderCache.gradientState.radial_edge_color = stop.stop_color;
                        } else {
                            window_surface->renderCache.gradientState.radial_center_color = stop.stop_color;
                        }
                    }
                }
                break;
            }
            
            default:
                break;
        }
    }
    
#ifdef mla_debug_build
    DWORD current_time = GetTickCount();
    if (window_surface->DEBUG_last_fps_time == 0) {
        window_surface->DEBUG_last_fps_time = current_time;
    }
    
    window_surface->DEBUG_frames_accumulated++;
    
    if (current_time - window_surface->DEBUG_last_fps_time >= 1000) {
        window_surface->DEBUG_current_fps = window_surface->DEBUG_frames_accumulated;
        window_surface->DEBUG_frames_accumulated = 0;
        window_surface->DEBUG_last_fps_time = current_time;
    }
    
    // Draw FPS counter using OpenGL
    mla_ui_surface_font_type_t debugFontType = { mla_string_const("Arial"), 15.0, false, false };
    mla_global_ui_surface_windows_opengl_font_cache_item* debugFont = 
        __mla_global_ui_surface_windows_opengl_font_cache_getOrCreateFont(
            window_surface->renderCache, window_surface->hdc, debugFontType);
    
    if (debugFont && debugFont->resources && debugFont->resources->displayListBase != 0) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red color
        
        glPushMatrix();
        glTranslatef((GLfloat)(windowWidth - 75), (GLfloat)(windowHeight - 25), 0.0f);
        glScalef(22.5f, -22.5f, 1.0f); // 15 * 1.5 = 22.5
        
        char buffer[64];
        wsprintfA(buffer, "FPS: %d", window_surface->DEBUG_current_fps);
        
        glListBase(debugFont->resources->displayListBase);
        glCallLists((GLsizei)strlen(buffer), GL_UNSIGNED_BYTE, buffer);
        
        glPopMatrix();
    }
#endif
    
    // Swap buffers
    SwapBuffers(window_surface->hdc);
    
    return true;
}

mla_buffer_cleanup_mode __windows_surface_opengl_buffer_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(data);
    
    if (window_surface != nullptr) {
        
        // Cleanup OpenGL resources
        window_surface->renderCache = __mla_global_ui_surface_windows_opengl_cache_empty();
        
        if (window_surface->hglrc) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(window_surface->hglrc);
            window_surface->hglrc = nullptr;
        }
        
        if (window_surface->hdc) {
            ReleaseDC(window_surface->hwnd, window_surface->hdc);
            window_surface->hdc = nullptr;
        }
        
        if (IsWindow(window_surface->hwnd)) {
            DestroyWindow(window_surface->hwnd);
        }
    }
    
    return CLEAN_UP_NEEDED;
}

mla_bool_t __windows_create_opengl_surface(mla_ui_surface_t &outSurface) {
    
    mla_windows_window_surface_opengl_t *window_surface = static_cast<mla_windows_window_surface_opengl_t *>(mla_malloc(sizeof(mla_windows_window_surface_opengl_t)));
    
    if (window_surface == nullptr) {
        return false;
    }
    
    mla_memset(window_surface, 0, sizeof(mla_windows_window_surface_opengl_t));
    window_surface->is_initialized = false;
    window_surface->default_size = {0, 0};
    
#ifdef mla_debug_build
    window_surface->DEBUG_last_fps_time = 0;
    window_surface->DEBUG_frames_accumulated = 0;
    window_surface->DEBUG_current_fps = 0;
#endif
    
    outSurface.resource = window_surface;
    outSurface.resourceOwner = mla_buffer_reference(window_surface, true, __windows_surface_opengl_buffer_cleanup);
    outSurface.get_size = __windows_surface_opengl_get_size;
    outSurface.set_size = __windows_surface_opengl_set_size;
    outSurface.render_draw_commands = __windows_surface_opengl_render_draw_commands;
    outSurface.calc_text_size = __windows_surface_opengl_calc_text_size;
    outSurface.get_input_states = __windows_surface_opengl_input_states;
    
    return true;
}

mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    __windows_create_opengl_surface
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Startup and Shutdown functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialize OpenGL (call once at startup)
void __windows_opengl_init() {
    SetProcessDPIAware();
}

// Cleanup OpenGL (call at shutdown)
void __windows_opengl_shutdown() {
    // OpenGL contexts are cleaned up per-surface
}

struct MlaOpenGLAutoInit {
    MlaOpenGLAutoInit() { __windows_opengl_init(); }
    ~MlaOpenGLAutoInit() { __windows_opengl_shutdown(); }
};

// Single global instance
static MlaOpenGLAutoInit g_mlaOpenGLAutoInit;


#endif // COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_OPENGL_H
