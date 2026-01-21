//
// Created by chris on 1/19/2026.
//

#ifndef COREOS_MLA_UI_STYLE_H
#define COREOS_MLA_UI_STYLE_H

#include "../../mla_data_types.h"

// Generic SVG filter region defaults
#define MLA_UI_SVG_SHADOW_FILTER_X        "-20%"
#define MLA_UI_SVG_SHADOW_FILTER_Y        "-20%"
#define MLA_UI_SVG_SHADOW_FILTER_W        "140%"
#define MLA_UI_SVG_SHADOW_FILTER_H        "140%"
#define MLA_UI_SVG_SHADOW_FILTER_CIF      "sRGB"

// Generic font defaults
#define MLA_UI_FONT_FAMILY_DEFAULT        "Segoe UI, system-ui, -apple-system"
#define MLA_UI_FONT_SIZE_DEFAULT          14
#define MLA_UI_FONT_WEIGHT_SEMIBOLD       600


// Text colors (Fluent-like neutrals and link)
// Notes:
// - Primary: near-black with high contrast on white
// - Secondary: slightly lighter neutral for subtle text
// - Disabled: muted neutral
// - Link: Fluent-style blue
#define MLA_UI_COLOR_TEXT_PRIMARY         "rgba(0,0,0,0.90)"
#define MLA_UI_COLOR_TEXT_SECONDARY       "rgba(0,0,0,0.70)"
#define MLA_UI_COLOR_TEXT_DISABLED        "rgba(0,0,0,0.36)"
#define MLA_UI_COLOR_TEXT_LINK            "#115EA3"


enum mla_ui_text_kind_t: mla_uint8_t {
    MLA_UI_TEXT_KIND_PRIMARY, // Accent color
    MLA_UI_TEXT_KIND_SECONDARY, // Subtle color
    MLA_UI_TEXT_KIND_DISABLED, // Disabled color
    MLA_UI_TEXT_KIND_LINK, // Link color
};

#endif