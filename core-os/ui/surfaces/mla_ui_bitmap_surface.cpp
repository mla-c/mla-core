//
// Created by Jules on 2/22/2026.
//

#include "mla_ui_bitmap_surface.h"
#include "../../system/mla_buffer.h"
#include "../../system/mla_number.h"

struct mla_ui_bitmap_surface_resource_t {
    mla_uint32_t width;
    mla_uint32_t height;
    mla_byte_t* pixels;
};

static mla_buffer_cleanup_mode __mla_ui_bitmap_surface_cleanup(mla_pointer_t p_Data, const mla_dynamic_data_t& p_UserData) {
    (void)p_UserData;
    mla_ui_bitmap_surface_resource_t* resource = static_cast<mla_ui_bitmap_surface_resource_t*>(p_Data);
    if (resource != nullptr) {
        if (resource->pixels != nullptr) {
            mla_free(resource->pixels);
        }
        mla_free(resource);
    }
    return CLEAN_UP_NEEDED;
}

// Basic 5x7 font - simplified ASCII
static const mla_byte_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // (backslash)
    0x00, 0x41, 0x41, 0x7F, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x0C, 0x52, 0x52, 0x52, 0x3E, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x7F, 0x10, 0x28, 0x44, 0x00, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08  // ->
};

static mla_ui_surface_size_t __mla_ui_bitmap_surface_get_size(const mla_ui_surface_t& p_Surface) {
    mla_ui_bitmap_surface_resource_t* resource = static_cast<mla_ui_bitmap_surface_resource_t*>(p_Surface.resource);
    if (resource == nullptr) return {0, 0};
    return {resource->width, resource->height};
}

static mla_bool_t __mla_ui_bitmap_surface_set_size(const mla_ui_surface_t& p_Surface, mla_ui_surface_size_t p_Size) {
    mla_ui_bitmap_surface_resource_t* resource = static_cast<mla_ui_bitmap_surface_resource_t*>(p_Surface.resource);
    if (resource == nullptr) return false;

    if (resource->width == p_Size.width && resource->height == p_Size.height) return true;

    mla_size_t newSize = p_Size.width * p_Size.height * 4;
    mla_byte_t* newPixels = static_cast<mla_byte_t*>(mla_malloc(newSize));
    if (newPixels == nullptr) return false;

    mla_memset(newPixels, 0, newSize);

    if (resource->pixels != nullptr) {
        mla_free(resource->pixels);
    }

    resource->pixels = newPixels;
    resource->width = p_Size.width;
    resource->height = p_Size.height;

    return true;
}

static mla_ui_surface_draw_size_t __mla_ui_bitmap_surface_calc_text_size(const mla_ui_surface_t &p_Surface, const mla_ui_surface_font_type_t &p_Font_type, const mla_string_t &p_Text) {
    (void)p_Surface;
    mla_size_t len = mla_string_length(p_Text);
    mla_double_t scale = p_Font_type.size / 7.0;
    if (scale < 1.0) scale = 1.0;

    return { static_cast<mla_double_t>(len * 6 * scale), 7.0 * scale };
}

static mla_ui_surface_input_states_t __mla_ui_bitmap_surface_get_input_states(const mla_ui_surface_t &p_Surface) {
    (void)p_Surface;
    return mla_ui_surface_input_states_empty();
}

static void __mla_ui_bitmap_surface_draw_pixel(mla_ui_bitmap_surface_resource_t* r, mla_int32_t x, mla_int32_t y, mla_ui_surface_draw_command_color_t color) {
    if (x < 0 || x >= (mla_int32_t)r->width || y < 0 || y >= (mla_int32_t)r->height) return;
    if (color.a == 0) return;

    mla_byte_t* p = &r->pixels[(y * r->width + x) * 4];

    if (color.a == 255) {
        p[0] = color.r;
        p[1] = color.g;
        p[2] = color.b;
        p[3] = color.a;
    } else {
        mla_uint32_t alpha = color.a;
        mla_uint32_t inv_alpha = 255 - alpha;
        p[0] = (mla_byte_t)((color.r * alpha + p[0] * inv_alpha) / 255);
        p[1] = (mla_byte_t)((color.g * alpha + p[1] * inv_alpha) / 255);
        p[2] = (mla_byte_t)((color.b * alpha + p[2] * inv_alpha) / 255);
        p[3] = (mla_byte_t)(alpha + (p[3] * inv_alpha) / 255);
    }
}

static void __mla_ui_bitmap_surface_fill_rect(mla_ui_bitmap_surface_resource_t* r, mla_int32_t x, mla_int32_t y, mla_int32_t w, mla_int32_t h, mla_ui_surface_draw_command_color_t color) {
    for (mla_int32_t i = 0; i < h; ++i) {
        for (mla_int32_t j = 0; j < w; ++j) {
            __mla_ui_bitmap_surface_draw_pixel(r, x + j, y + i, color);
        }
    }
}

static void __mla_ui_bitmap_surface_draw_line(mla_ui_bitmap_surface_resource_t* r, mla_int32_t x0, mla_int32_t y0, mla_int32_t x1, mla_int32_t y1, mla_ui_surface_draw_command_color_t color, mla_int32_t stroke_width) {
    mla_int32_t dx = (x1 - x0 > 0) ? (x1 - x0) : (x0 - x1);
    mla_int32_t sx = x0 < x1 ? 1 : -1;
    mla_int32_t dy = (y1 - y0 > 0) ? -(y1 - y0) : -(y0 - y1);
    mla_int32_t sy = y0 < y1 ? 1 : -1;
    mla_int32_t err = dx + dy, e2;

    for (;;) {
        if (stroke_width <= 1) {
            __mla_ui_bitmap_surface_draw_pixel(r, x0, y0, color);
        } else {
            __mla_ui_bitmap_surface_fill_rect(r, x0 - stroke_width / 2, y0 - stroke_width / 2, stroke_width, stroke_width, color);
        }

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void __mla_ui_bitmap_surface_draw_circle(mla_ui_bitmap_surface_resource_t* r, mla_int32_t cx, mla_int32_t cy, mla_int32_t rad, mla_ui_surface_draw_command_color_t color, mla_bool_t fill) {
    mla_int32_t x = rad, y = 0;
    mla_int32_t radiusError = 1 - x;

    while (x >= y) {
        if (fill) {
            __mla_ui_bitmap_surface_draw_line(r, cx - x, cy + y, cx + x, cy + y, color, 1);
            if (y > 0) __mla_ui_bitmap_surface_draw_line(r, cx - x, cy - y, cx + x, cy - y, color, 1);
            if (x != y) {
                __mla_ui_bitmap_surface_draw_line(r, cx - y, cy + x, cx + y, cy + x, color, 1);
                if (x > 0) __mla_ui_bitmap_surface_draw_line(r, cx - y, cy - x, cx + y, cy - x, color, 1);
            }
        } else {
            __mla_ui_bitmap_surface_draw_pixel(r, cx + x, cy + y, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx - x, cy + y, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx + x, cy - y, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx - x, cy - y, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx + y, cy + x, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx - y, cy + x, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx + y, cy - x, color);
            __mla_ui_bitmap_surface_draw_pixel(r, cx - y, cy - x, color);
        }
        y++;
        if (radiusError < 0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

static void __mla_ui_bitmap_surface_draw_char(mla_ui_bitmap_surface_resource_t* r, mla_int32_t x, mla_int32_t y, mla_char_t c, mla_ui_surface_draw_command_color_t color, mla_int32_t scale) {
    if (c < 32 || c > 127) return;
    const mla_byte_t* bitmap = font5x7 + (c - 32) * 5;
    for (mla_int32_t i = 0; i < 5; i++) {
        mla_byte_t line = bitmap[i];
        for (mla_int32_t j = 0; j < 8; j++) {
            if (line & 0x01) {
                if (scale <= 1)
                    __mla_ui_bitmap_surface_draw_pixel(r, x + i, y + j, color);
                else
                    __mla_ui_bitmap_surface_fill_rect(r, x + i * scale, y + j * scale, scale, scale, color);
            }
            line >>= 1;
        }
    }
}

static void __mla_ui_bitmap_surface_draw_string(mla_ui_bitmap_surface_resource_t* r, mla_int32_t x, mla_int32_t y, const mla_char_t* str, mla_ui_surface_draw_command_color_t color, mla_int32_t scale) {
    while (*str) {
        __mla_ui_bitmap_surface_draw_char(r, x, y, *str, color, scale);
        x += 6 * scale;
        str++;
    }
}

static mla_bool_t __mla_ui_bitmap_surface_render_draw_commands(const mla_ui_surface_t& p_Surface, const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& p_DrawCommands, mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>& p_EventsSinceLastFame, mla_uint64_t p_TimeSinceLastFrameMs) {
    (void)p_EventsSinceLastFame;
    (void)p_TimeSinceLastFrameMs;
    mla_ui_bitmap_surface_resource_t* resource = static_cast<mla_ui_bitmap_surface_resource_t*>(p_Surface.resource);
    if (resource == nullptr) return false;

    for (mla_size_t i = 0; i < mla_array_list_size(p_DrawCommands); i++) {
        const mla_ui_surface_draw_command_t& cmd = mla_array_list_get_unsafe(p_DrawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                __mla_ui_bitmap_surface_fill_rect(resource, (mla_int32_t)cmd.rect.x, (mla_int32_t)cmd.rect.y, (mla_int32_t)cmd.rect.width, (mla_int32_t)cmd.rect.height, cmd.rect.color);
                if (cmd.rect.stroke_width > 0) {
                    // Simple outline
                    mla_int32_t x = (mla_int32_t)cmd.rect.x;
                    mla_int32_t y = (mla_int32_t)cmd.rect.y;
                    mla_int32_t w = (mla_int32_t)cmd.rect.width;
                    mla_int32_t h = (mla_int32_t)cmd.rect.height;
                    __mla_ui_bitmap_surface_draw_line(resource, x, y, x + w - 1, y, cmd.rect.stroke, (mla_int32_t)cmd.rect.stroke_width);
                    __mla_ui_bitmap_surface_draw_line(resource, x + w - 1, y, x + w - 1, y + h - 1, cmd.rect.stroke, (mla_int32_t)cmd.rect.stroke_width);
                    __mla_ui_bitmap_surface_draw_line(resource, x + w - 1, y + h - 1, x, y + h - 1, cmd.rect.stroke, (mla_int32_t)cmd.rect.stroke_width);
                    __mla_ui_bitmap_surface_draw_line(resource, x, y + h - 1, x, y, cmd.rect.stroke, (mla_int32_t)cmd.rect.stroke_width);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                __mla_ui_bitmap_surface_draw_circle(resource, (mla_int32_t)cmd.circle.cx, (mla_int32_t)cmd.circle.cy, (mla_int32_t)cmd.circle.r, cmd.circle.fill, true);
                if (cmd.circle.stroke_width > 0) {
                    __mla_ui_bitmap_surface_draw_circle(resource, (mla_int32_t)cmd.circle.cx, (mla_int32_t)cmd.circle.cy, (mla_int32_t)cmd.circle.r, cmd.circle.stroke, false);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                __mla_ui_bitmap_surface_draw_line(resource, (mla_int32_t)cmd.line.x1, (mla_int32_t)cmd.line.y1, (mla_int32_t)cmd.line.x2, (mla_int32_t)cmd.line.y2, cmd.line.stroke, (mla_int32_t)cmd.line.stroke_width);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                const mla_char_t* str = mla_string_data(cmd.text.content);
                if (str != nullptr) {
                    mla_int32_t scale = (mla_int32_t)(cmd.text.font_type.size / 7.0);
                    if (scale < 1) scale = 1;
                    __mla_ui_bitmap_surface_draw_string(resource, (mla_int32_t)cmd.text.x, (mla_int32_t)cmd.text.y, str, cmd.text.fill, scale);
                }
                break;
            }
            default:
                break;
        }
    }

    return true;
}

mla_ui_surface_t mla_ui_bitmap_surface_create(mla_uint32_t p_Width, mla_uint32_t p_Height) {
    mla_ui_bitmap_surface_resource_t* resource = static_cast<mla_ui_bitmap_surface_resource_t*>(mla_malloc(sizeof(mla_ui_bitmap_surface_resource_t)));
    if (resource == nullptr) return mla_ui_surface_invalid();

    resource->width = p_Width;
    resource->height = p_Height;
    mla_size_t size = p_Width * p_Height * 4;
    resource->pixels = static_cast<mla_byte_t*>(mla_malloc(size));

    if (resource->pixels == nullptr) {
        mla_free(resource);
        return mla_ui_surface_invalid();
    }

    mla_memset(resource->pixels, 0, size);

    mla_ui_surface_t surface = mla_ui_surface_invalid();
    surface.resource = resource;
    surface.resourceOwner = mla_buffer_reference_create(resource, false, __mla_ui_bitmap_surface_cleanup, mla_dynamic_data_empty());
    surface.get_size = __mla_ui_bitmap_surface_get_size;
    surface.set_size = __mla_ui_bitmap_surface_set_size;
    surface.calc_text_size = __mla_ui_bitmap_surface_calc_text_size;
    surface.get_input_states = __mla_ui_bitmap_surface_get_input_states;
    surface.render_draw_commands = __mla_ui_bitmap_surface_render_draw_commands;

    return surface;
}

mla_bytes_t mla_ui_bitmap_surface_get_bytes(const mla_ui_surface_t& p_Surface) {
    mla_ui_bitmap_surface_resource_t* resource = static_cast<mla_ui_bitmap_surface_resource_t*>(p_Surface.resource);
    if (resource == nullptr || resource->pixels == nullptr) return mla_bytes_empty();

    mla_size_t size = resource->width * resource->height * 4;
    return mla_bytes_from_external_buffer(resource->pixels, size);
}
