//
// Created by chris on 2/5/2026.
//

#include "mla_ui_display_surface.h"

mla_global mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access;

mla_ui_surface_t mla_ui_display_surface_create() {

    if (g_ui_display_surface_low_level_access.create_surface == nullptr) {
        return mla_ui_surface_invalid();
    }

    mla_ui_surface_t surface = mla_ui_surface_invalid();

    if (!g_ui_display_surface_low_level_access.create_surface(surface)) {
        return mla_ui_surface_invalid();
    }

    return surface;
}