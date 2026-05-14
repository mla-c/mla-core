//
// Created by chris on 2/5/2026.
//

#ifndef MLA_UI_DISPLAY_SURFACE_H
#define MLA_UI_DISPLAY_SURFACE_H

#include "../surfaces/mla_ui_surface.h"


mla_ui_surface_t mla_ui_display_surface_create();

// Low level access to UI surface creation
////////////////////////////////////////////////////////////////////////////

struct mla_ui_display_surface_low_level_access_t {
    mla_bool_t (*create_surface)(mla_ui_surface_t& outSurface);
};

#endif