//
// Created by chris on 1/30/2026.
//

#ifndef COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_ESP32_S3_H
#define COREOS_MLA_GLOBAL_UI_SURFACE_WINDOWS_ESP32_S3_H

#include "../../core-os/ui/surfaces/mla_ui_surface.h"

mla_bool_t __esp32_create_surface(mla_ui_surface_t &outSurface) {
    // ESP32-S3 platform does not support windowed surfaces
    (void) outSurface;
    return false;
}

mla_ui_surface_low_level_access_t g_ui_surface_low_level_access = {
    __esp32_create_surface
};

#endif