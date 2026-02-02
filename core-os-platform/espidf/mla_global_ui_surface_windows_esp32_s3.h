//
// Created by chris on 1/30/2026.
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_ESP32_S3_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_ESP32_S3_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"
#include <math.h>
#include <stdlib.h>

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_heap_caps.h>
#include <driver/gpio.h>
#include <esp_attr.h>

// Basic 5x7 font - simplified ASCII
static const unsigned char font5x7[] = {
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

struct mla_esp32_software_renderer_t {
    uint16_t* framebuffer;
    int width;
    int height;
};

static uint16_t mla_esp32_software_renderer_color565(const mla_ui_surface_draw_command_color_t &c) {
    return ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);
}

static void mla_esp32_software_renderer_draw_pixel(mla_esp32_software_renderer_t* r, int x, int y, uint16_t color) {
    if (x < 0 || x >= r->width || y < 0 || y >= r->height) return;
    r->framebuffer[y * r->width + x] = color;
}

static void mla_esp32_software_renderer_fill_rect(mla_esp32_software_renderer_t* r, int x, int y, int w, int h, uint16_t color) {
    if (x >= r->width || y >= r->height) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > r->width) w = r->width - x;
    if (y + h > r->height) h = r->height - y;
    if (w <= 0 || h <= 0) return;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            r->framebuffer[(y + i) * r->width + (x + j)] = color;
        }
    }
}

static void mla_esp32_software_renderer_draw_rect(mla_esp32_software_renderer_t* r, int x, int y, int w, int h, uint16_t color, int stroke_width) {
    // Simplified stroke
    for(int i=0; i<stroke_width; i++) {
            // Top
            mla_esp32_software_renderer_fill_rect(r, x, y+i, w, 1, color);
            // Bottom
            mla_esp32_software_renderer_fill_rect(r, x, y+h-1-i, w, 1, color);
            // Left
            mla_esp32_software_renderer_fill_rect(r, x+i, y, 1, h, color);
            // Right
            mla_esp32_software_renderer_fill_rect(r, x+w-1-i, y, 1, h, color);
    }
}

static void mla_esp32_software_renderer_draw_line(mla_esp32_software_renderer_t* r, int x0, int y0, int x1, int y1, uint16_t color, int stroke_width) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        if(stroke_width == 1)
            mla_esp32_software_renderer_draw_pixel(r, x0, y0, color);
        else
            mla_esp32_software_renderer_fill_rect(r, x0 - stroke_width/2, y0 - stroke_width/2, stroke_width, stroke_width, color);

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void mla_esp32_software_renderer_draw_circle(mla_esp32_software_renderer_t* r, int cx, int cy, int rad, uint16_t color, bool fill) {
    int x = -rad, y = 0, err = 2 - 2 * rad;
    do {
        if (fill) {
            mla_esp32_software_renderer_draw_line(r, cx + x, cy - y, cx - x, cy - y, color, 1);
            mla_esp32_software_renderer_draw_line(r, cx + x, cy + y, cx - x, cy + y, color, 1);
        } else {
            mla_esp32_software_renderer_draw_pixel(r, cx - x, cy + y, color);
            mla_esp32_software_renderer_draw_pixel(r, cx - y, cy - x, color);
            mla_esp32_software_renderer_draw_pixel(r, cx + x, cy - y, color);
            mla_esp32_software_renderer_draw_pixel(r, cx + y, cy + x, color);
        }
        rad = err;
        if (rad <= y) err += ++y * 2 + 1;
        if (rad > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

static void mla_esp32_software_renderer_draw_ellipse(mla_esp32_software_renderer_t* r, int cx, int cy, int rx, int ry, uint16_t color)
{
    // Simple ellipse approximation or bounding box for now if complex
    // This is a minimal implementation placeholder
    mla_esp32_software_renderer_draw_circle(r, cx, cy, (rx+ry)/2, color, false);
}

static void mla_esp32_software_renderer_draw_char(mla_esp32_software_renderer_t* r, int x, int y, char c, uint16_t color, int size) {
    if (c < 32 || c > 127) return;
    const unsigned char* bitmap = font5x7 + (c - 32) * 5;
    for (int i = 0; i < 5; i++) {
        unsigned char line = bitmap[i];
        for (int j = 0; j < 8; j++) {
            if (line & 0x01) {
                if (size == 1)
                    mla_esp32_software_renderer_draw_pixel(r, x + i, y + j, color);
                else
                    mla_esp32_software_renderer_fill_rect(r, x + i*size, y + j*size, size, size, color);
            }
            line >>= 1;
        }
    }
}

static void mla_esp32_software_renderer_draw_string(mla_esp32_software_renderer_t* r, int x, int y, const char* str, uint16_t color, int size) {
    while (*str) {
        mla_esp32_software_renderer_draw_char(r, x, y, *str, color, size);
        x += 6 * size;
        str++;
    }
}

static void mla_esp32_software_renderer_clear(mla_esp32_software_renderer_t* r, uint16_t color) {
    for (int i = 0; i < r->width * r->height; i++) {
        r->framebuffer[i] = color;
    }
}

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (16 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_PIN_NUM_BK_LIGHT       38
#define EXAMPLE_PIN_NUM_HSYNC          39
#define EXAMPLE_PIN_NUM_VSYNC          41
#define EXAMPLE_PIN_NUM_DE             40
#define EXAMPLE_PIN_NUM_PCLK           42
#define EXAMPLE_PIN_NUM_DATA0          45 // R0
#define EXAMPLE_PIN_NUM_DATA1          48 // R1
#define EXAMPLE_PIN_NUM_DATA2          47 // R2
#define EXAMPLE_PIN_NUM_DATA3          21 // R3
#define EXAMPLE_PIN_NUM_DATA4          14 // R4
#define EXAMPLE_PIN_NUM_DATA5          5  // G0
#define EXAMPLE_PIN_NUM_DATA6          6  // G1
#define EXAMPLE_PIN_NUM_DATA7          7  // G2
#define EXAMPLE_PIN_NUM_DATA8          15 // G3
#define EXAMPLE_PIN_NUM_DATA9          16 // G4
#define EXAMPLE_PIN_NUM_DATA10         4  // G5
#define EXAMPLE_PIN_NUM_DATA11         8  // B0
#define EXAMPLE_PIN_NUM_DATA12         3  // B1
#define EXAMPLE_PIN_NUM_DATA13         46 // B2
#define EXAMPLE_PIN_NUM_DATA14         9  // B3
#define EXAMPLE_PIN_NUM_DATA15         1  // B4
#define EXAMPLE_PIN_NUM_DISP_EN        -1

#define LCD_H_RES                      480
#define LCD_V_RES                      480

struct mla_esp32_surface_t {
    esp_lcd_panel_handle_t panel_handle;
    uint16_t *frame_buffer;
    mla_esp32_software_renderer_t renderer;
    mla_ui_surface_size_t size;
};

// Cleanup function
inline mla_buffer_cleanup_mode __esp32_surface_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;
    mla_esp32_surface_t *surface = (mla_esp32_surface_t *) data;
    if (surface) {
        // panel handle cleanup if needed, usually we don't deinit display in embedded
    }
    return CLEAN_UP_NEEDED;
}

inline mla_ui_surface_size_t __esp32_surface_get_size(const mla_ui_surface_t &surface) {
     mla_esp32_surface_t *esp_surface = (mla_esp32_surface_t *) surface.resource;
     if(!esp_surface) return {0,0};
     return esp_surface->size;
}

inline mla_bool_t __esp32_surface_set_size(const mla_ui_surface_t &surface, mla_ui_surface_size_t size) {
    (void) surface; (void) size;
    return false; // Cannot resize physical display
}

inline mla_ui_surface_draw_size_t __esp32_surface_calc_text_size(const mla_ui_surface_t &surface, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {
     (void) surface; (void) font_type;
     // Basic 5x7 font calculation
     mla_ui_surface_draw_size_t size;
     size.height = 7;
     // We treat font size as scale factor in our simple renderer
     if(font_type.size > 0) size.height = 7.0 * (font_type.size / 10.0); // Rough scaling
     if(size.height < 7) size.height = 7;

     // Access string data for length
     mla_size_t len = mla_string_length(text);

     size.width = (double)(len * 6 * (size.height / 7.0));
     return size;
}

inline mla_ui_surface_input_states_t __esp32_surface_get_input_states(const mla_ui_surface_t &surface) {
    (void) surface;
    // TODO: Implement GT911 touch driver reading
    return mla_ui_surface_input_states_empty();
}

inline mla_bool_t __esp32_surface_render_draw_commands(const mla_ui_surface_t& surface, const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>& eventsSinceLastFame) {
    (void) eventsSinceLastFame;
    mla_esp32_surface_t *esp_surface = (mla_esp32_surface_t *) surface.resource;
    if (!esp_surface) return false;

    mla_esp32_software_renderer_t *r = &esp_surface->renderer;

    // Clear background if needed? Or we assume commands cover it.
    // Let's iterate commands
    for (mla_size_t i = 0; i < mla_array_list_size(drawCommands); i++) {
        const mla_ui_surface_draw_command_t &cmd = mla_array_list_get_unsafe(drawCommands, i);

        switch (cmd.kind) {
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT: {
                uint16_t color = mla_esp32_software_renderer_color565(cmd.rect.color);
                // Fill
                mla_esp32_software_renderer_fill_rect(r, (int)cmd.rect.x, (int)cmd.rect.y, (int)cmd.rect.width, (int)cmd.rect.height, color);
                // Stroke
                if(cmd.rect.stroke_width > 0) {
                     uint16_t strokeColor = mla_esp32_software_renderer_color565(cmd.rect.stroke);
                     mla_esp32_software_renderer_draw_rect(r, (int)cmd.rect.x, (int)cmd.rect.y, (int)cmd.rect.width, (int)cmd.rect.height, strokeColor, (int)cmd.rect.stroke_width);
                }
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE: {
                uint16_t color = mla_esp32_software_renderer_color565(cmd.circle.fill);
                // Fill
                mla_esp32_software_renderer_draw_circle(r, (int)cmd.circle.cx, (int)cmd.circle.cy, (int)cmd.circle.r, color, true);
                // Stroke
                 if(cmd.circle.stroke_width > 0) {
                     uint16_t strokeColor = mla_esp32_software_renderer_color565(cmd.circle.stroke);
                     mla_esp32_software_renderer_draw_circle(r, (int)cmd.circle.cx, (int)cmd.circle.cy, (int)cmd.circle.r, strokeColor, false);
                }
                break;
            }
             case MLA_UI_SURFACE_DRAW_COMMAND_KIND_ELLIPSE: {
                uint16_t color = mla_esp32_software_renderer_color565(cmd.ellipse.fill);
                mla_esp32_software_renderer_draw_ellipse(r, (int)cmd.ellipse.cx, (int)cmd.ellipse.cy, (int)cmd.ellipse.rx, (int)cmd.ellipse.ry, color);
                break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE: {
                 uint16_t color = mla_esp32_software_renderer_color565(cmd.line.stroke);
                 mla_esp32_software_renderer_draw_line(r, (int)cmd.line.x1, (int)cmd.line.y1, (int)cmd.line.x2, (int)cmd.line.y2, color, (int)(cmd.line.stroke_width > 0 ? cmd.line.stroke_width : 1));
                 break;
            }
            case MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT: {
                 uint16_t color = mla_esp32_software_renderer_color565(cmd.text.fill);

                 const char* str_data = mla_string_data(cmd.text.content);

                 if(str_data == nullptr) break;

                 // Rough font size handling
                 int scale = 1;
                 if(cmd.text.font_type.size > 12) scale = 2;
                 if(cmd.text.font_type.size > 24) scale = 3;

                 mla_esp32_software_renderer_draw_string(r, (int)cmd.text.x, (int)cmd.text.y, str_data, color, scale);
                 break;
            }
            default:
                break;
        }
    }

    // Explicitly cache maintenance if needed, but esp_lcd usually handles it via bounce buffer or direct
    // Since we write to PSRAM mapped frame buffer, we should ensure data is written to RAM
    // esp_cache_msync((void*)esp_surface->frame_buffer, LCD_H_RES * LCD_V_RES * 2, ESP_CACHE_MSYNC_FLAG_DIR_C2M);

    return true;
}

static bool s_is_initialized = false;
static mla_esp32_surface_t *s_global_surface = nullptr;

inline mla_bool_t __esp32_create_surface(mla_ui_surface_t &outSurface) {
    if (s_is_initialized && s_global_surface) {
         // Return existing
         outSurface.resource = s_global_surface;
         outSurface.resourceOwner = mla_buffer_reference(s_global_surface, false, __esp32_surface_cleanup);
         outSurface.get_size = __esp32_surface_get_size;
         outSurface.set_size = __esp32_surface_set_size;
         outSurface.render_draw_commands = __esp32_surface_render_draw_commands;
         outSurface.calc_text_size = __esp32_surface_calc_text_size;
         outSurface.get_input_states = __esp32_surface_get_input_states;
         return true;
    }

    // Configure GPIOs
    gpio_config_t bk_gpio_config = {
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&bk_gpio_config);

    esp_lcd_rgb_panel_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));

    panel_config.data_width = 16;
    panel_config.psram_trans_align = 64;
    panel_config.num_fbs = 1;
    panel_config.clk_src = LCD_CLK_SRC_DEFAULT;
    panel_config.disp_gpio_num = EXAMPLE_PIN_NUM_DISP_EN;
    panel_config.pclk_gpio_num = EXAMPLE_PIN_NUM_PCLK;
    panel_config.vsync_gpio_num = EXAMPLE_PIN_NUM_VSYNC;
    panel_config.hsync_gpio_num = EXAMPLE_PIN_NUM_HSYNC;
    panel_config.de_gpio_num = EXAMPLE_PIN_NUM_DE;
    panel_config.data_gpio_nums[0] = EXAMPLE_PIN_NUM_DATA0;
    panel_config.data_gpio_nums[1] = EXAMPLE_PIN_NUM_DATA1;
    panel_config.data_gpio_nums[2] = EXAMPLE_PIN_NUM_DATA2;
    panel_config.data_gpio_nums[3] = EXAMPLE_PIN_NUM_DATA3;
    panel_config.data_gpio_nums[4] = EXAMPLE_PIN_NUM_DATA4;
    panel_config.data_gpio_nums[5] = EXAMPLE_PIN_NUM_DATA5;
    panel_config.data_gpio_nums[6] = EXAMPLE_PIN_NUM_DATA6;
    panel_config.data_gpio_nums[7] = EXAMPLE_PIN_NUM_DATA7;
    panel_config.data_gpio_nums[8] = EXAMPLE_PIN_NUM_DATA8;
    panel_config.data_gpio_nums[9] = EXAMPLE_PIN_NUM_DATA9;
    panel_config.data_gpio_nums[10] = EXAMPLE_PIN_NUM_DATA10;
    panel_config.data_gpio_nums[11] = EXAMPLE_PIN_NUM_DATA11;
    panel_config.data_gpio_nums[12] = EXAMPLE_PIN_NUM_DATA12;
    panel_config.data_gpio_nums[13] = EXAMPLE_PIN_NUM_DATA13;
    panel_config.data_gpio_nums[14] = EXAMPLE_PIN_NUM_DATA14;
    panel_config.data_gpio_nums[15] = EXAMPLE_PIN_NUM_DATA15;

    panel_config.timings.pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ;
    panel_config.timings.h_res = LCD_H_RES;
    panel_config.timings.v_res = LCD_V_RES;
    panel_config.timings.hsync_back_porch = 50;
    panel_config.timings.hsync_front_porch = 10;
    panel_config.timings.hsync_pulse_width = 8;
    panel_config.timings.vsync_back_porch = 20;
    panel_config.timings.vsync_front_porch = 10;
    panel_config.timings.vsync_pulse_width = 8;
    panel_config.timings.flags.pclk_active_neg = true;
    panel_config.flags.fb_in_psram = 1;

    esp_lcd_panel_handle_t panel_handle = NULL;

    // Check available SPIRAM
    size_t free_spiram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    printf("Free SPIRAM: %zu, Largest Block: %zu. Required approx: %u\n", free_spiram, largest_block, LCD_H_RES * LCD_V_RES * 2);

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Get framebuffer
    uint16_t *frame_buffer = NULL;
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 1, (void **)&frame_buffer));

    // Turn on backlight
    gpio_set_level((gpio_num_t)EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    mla_esp32_surface_t *esp_surface = (mla_esp32_surface_t *) mla_malloc(sizeof(mla_esp32_surface_t));
    esp_surface->panel_handle = panel_handle;
    esp_surface->frame_buffer = frame_buffer;
    esp_surface->size = {LCD_H_RES, LCD_V_RES};

    // Init software renderer
    esp_surface->renderer = {frame_buffer, LCD_H_RES, LCD_V_RES};

    // Initial clear
    mla_esp32_software_renderer_clear(&esp_surface->renderer, 0x0000);

    outSurface.resource = esp_surface;
    outSurface.resourceOwner = mla_buffer_reference(esp_surface, false, __esp32_surface_cleanup);
    outSurface.get_size = __esp32_surface_get_size;
    outSurface.set_size = __esp32_surface_set_size;
    outSurface.render_draw_commands = __esp32_surface_render_draw_commands;
    outSurface.calc_text_size = __esp32_surface_calc_text_size;
    outSurface.get_input_states = __esp32_surface_get_input_states;

    s_global_surface = esp_surface;
    s_is_initialized = true;

    return true;
}

mla_ui_surface_low_level_access_t g_ui_surface_low_level_access = {
    __esp32_create_surface
};

#endif


