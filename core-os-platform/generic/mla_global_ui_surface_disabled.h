//
// Created by christian on 3/3/26.
//

#ifndef MLA_C_MLA_GLOBAL_UI_SURFACE_DISABLED_H
#define MLA_C_MLA_GLOBAL_UI_SURFACE_DISABLED_H

#include "../../core-os/ui/display/mla_ui_display_surface.h"

mla_bool_t __disabled_create_surface(mla_ui_surface_t &outSurface) {
    (void)outSurface;
    return false;
}

mla_ui_display_surface_low_level_access_t g_ui_display_surface_low_level_access = {
    __disabled_create_surface
};

#endif