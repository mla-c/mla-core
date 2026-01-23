//
// Created by chris on 1/19/2026.
//

#ifndef COREOS_MLA_UI_STYLE_H
#define COREOS_MLA_UI_STYLE_H

#include "../../mla_data_types.h"

// Generic font defaults
#define MLA_UI_FONT_FAMILY_DEFAULT        "Segoe UI"

/* Base text */
#define MLA_UI_FONT_SIZE_DEFAULT          12  /* Body, labels, inputs */
#define MLA_UI_FONT_SIZE_SECONDARY        11  /* Helper, hint, metadata */
#define MLA_UI_FONT_SIZE_CAPTION          10  /* Very compact captions */

/* Interactive elements */
#define MLA_UI_FONT_SIZE_BUTTON           12  /* Buttons */
#define MLA_UI_FONT_SIZE_LINK             12  /* Links */
#define MLA_UI_FONT_SIZE_TAB              12  /* Tabs, navigation */

/* Headings */
#define MLA_UI_FONT_SIZE_SECTION          14  /* Section headers */
#define MLA_UI_FONT_SIZE_PAGE_TITLE       18  /* Page titles */
#define MLA_UI_FONT_SIZE_PAGE_TITLE_LG    22  /* Large titles */

/* Status / feedback text */
#define MLA_UI_FONT_SIZE_STATUS           11  /* Error, warning, info, success */

/* Optional micro text (use carefully) */
#define MLA_UI_FONT_SIZE_MICRO             9  /* Legal, counters */


enum mla_ui_text_kind_t: mla_uint8_t {
    MLA_UI_TEXT_KIND_PRIMARY, // Accent color
    MLA_UI_TEXT_KIND_SECONDARY, // Subtle color
    MLA_UI_TEXT_KIND_DISABLED, // Disabled color
    MLA_UI_TEXT_KIND_LINK, // Link color
    MLA_UI_TEXT_KIND_LINK_DISABLED, // Link disabled color
    MLA_UI_TEXT_KIND_ERROR, // Error color
    MLA_UI_TEXT_KIND_WARNING, // Warning color
    MLA_UI_TEXT_KIND_SUCCESS, // Success color
    MLA_UI_TEXT_KIND_INFO, // Info color,
    MLA_UI_TEXT_KIND_CUSTOM // Custom color (not defined in style),
};

#define MLA_UI_COLOR_TEXT_PRIMARY         {0, 0, 0, 255}
#define MLA_UI_COLOR_TEXT_SECONDARY       {110, 110, 110, 255}
#define MLA_UI_COLOR_TEXT_DISABLED        {158, 158, 158, 255}
#define MLA_UI_COLOR_TEXT_LINK            {39, 174, 96, 255}
#define MLA_UI_COLOR_TEXT_LINK_HOVER      {30, 143, 79, 255}
#define MLA_UI_COLOR_TEXT_LINK_DISABLED   {158, 158, 158, 255}
#define MLA_UI_COLOR_TEXT_ERROR           {209, 52, 56, 255}
#define MLA_UI_COLOR_TEXT_WARNING         {255, 170, 68, 255}
#define MLA_UI_COLOR_TEXT_SUCCESS         {39, 174, 96, 255}
#define MLA_UI_COLOR_TEXT_INFO            {0, 120, 212, 255}

#endif