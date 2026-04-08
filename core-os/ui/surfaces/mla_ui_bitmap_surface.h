//
// Created by Jules on 2/22/2026.
//

#ifndef COREOS_MLA_UI_BITMAP_SURFACE_H
#define COREOS_MLA_UI_BITMAP_SURFACE_H

#include "mla_ui_surface.h"
#include "../../system/mla_bytes.h"

/**
 * Creates a new UI surface that renders into a 32-bit RGBA bitmap.
 * @param p_Width Initial width of the bitmap.
 * @param p_Height Initial height of the bitmap.
 * @return A new mla_ui_surface_t instance.
 */
mla_ui_surface_t mla_ui_bitmap_surface_create(mla_uint32_t p_Width, mla_uint32_t p_Height);

/**
 * Retrieves the raw pixel data from a bitmap surface as a borrowed reference.
 * Note: The returned buffer remains valid only as long as the surface is not resized or destroyed.
 * @param p_Surface The surface to retrieve data from.
 * @return mla_bytes_t containing a borrowed view of the 32-bit RGBA pixel data.
 */
mla_bytes_t mla_ui_bitmap_surface_get_bytes_borrowed(const mla_ui_surface_t& p_Surface);

#endif //COREOS_MLA_UI_BITMAP_SURFACE_H
