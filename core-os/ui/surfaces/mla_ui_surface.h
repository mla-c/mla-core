//
// Created by chris on 1/21/2026.
//

#ifndef COREOS_MLA_UI_SURFACE_H
#define COREOS_MLA_UI_SURFACE_H

#include "../../mla_data_types.h"
#include "../../system/mla_string.h"
#include "../../system/mla_reference.h"
#include "mla_ui_surface_draw.h"


struct mla_ui_surface_size_t {
    mla_uint32_t width;
    mla_uint32_t height;
};

struct mla_ui_surface_t {
    mla_pointer_t resource; // Pointer to the surface resource
    mla_buffer_reference_t resourceOwner; // Resource owner for the surface

    mla_ui_surface_size_t (*get_size)(const mla_ui_surface_t& surface);
    mla_bool_t (*set_size)(mla_ui_surface_t& surface, mla_ui_surface_size_t size);

    mla_bool_t (*render_svg)(mla_ui_surface_t& surface, mla_string_t svgContent);

    mla_bool_t (*render_draw_commands)(mla_ui_surface_t& surface, const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands);
};

mla_ui_surface_t mla_ui_surface_invalid();
mla_ui_surface_t mla_ui_surface_create();
mla_bool_t mla_ui_surface_is_valid(const mla_ui_surface_t& surface);

mla_ui_surface_size_t mla_ui_surface_get_size(const mla_ui_surface_t& surface);
mla_bool_t mla_ui_surface_set_size(mla_ui_surface_t& surface, mla_ui_surface_size_t size);
mla_bool_t mla_ui_surface_render_svg(mla_ui_surface_t& surface, mla_string_t svgContent);
mla_bool_t mla_ui_surface_render_draw_commands(mla_ui_surface_t& surface, const mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands);


// Low level access to UI surface creation
////////////////////////////////////////////////////////////////////////////

struct mla_ui_surface_low_level_access_t {
    mla_bool_t (*create_surface)(mla_ui_surface_t& outSurface);
};

mla_global mla_ui_surface_low_level_access_t g_ui_surface_low_level_access;


#endif