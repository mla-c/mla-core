//
// Created by Jules on 2/22/2026.
//

#ifndef COREOS_MLA_UI_BITMAP_SURFACE_TEST_H
#define COREOS_MLA_UI_BITMAP_SURFACE_TEST_H

#include "../../core-os/ui/surfaces/mla_ui_bitmap_surface.h"
#include "../../core-os-test-support/Test/mla_test.h"

#undef test_category
#define test_category "UI"

inline void TestBitmapSurfaceCreation() {
    mla_ui_surface_t surface = mla_ui_bitmap_surface_create(100, 100);
    assert_true(mla_ui_surface_is_valid(surface), "Surface should be valid");

    mla_ui_surface_size_t size = mla_ui_surface_get_size(surface);
    assert_equal(size.width, 100u, "Width should be 100");
    assert_equal(size.height, 100u, "Height should be 100");

    mla_bytes_t bytes = mla_ui_bitmap_surface_get_bytes(surface);
    assert_equal(mla_bytes_length(bytes), 100u * 100u * 4u, "Byte length mismatch");

    // Initial bitmap should be all zeros (transparent black)
    const mla_byte_t* data = mla_bytes_get_data_readonly(bytes);
    for (mla_size_t i = 0; i < 100 * 100 * 4; ++i) {
        if (data[i] != 0) {
            assert_true(false, "Bitmap not cleared");
        }
    }
    mla_buffer_reference_destroy(surface.resourceOwner);
}

inline void TestBitmapSurfaceResizing() {
    mla_ui_surface_t surface = mla_ui_bitmap_surface_create(50, 50);
    mla_ui_surface_set_size(surface, {80, 80});

    mla_ui_surface_size_t size = mla_ui_surface_get_size(surface);
    assert_equal(size.width, 80u, "Resized width mismatch");
    assert_equal(size.height, 80u, "Resized height mismatch");

    mla_bytes_t bytes = mla_ui_bitmap_surface_get_bytes(surface);
    assert_equal(mla_bytes_length(bytes), 80u * 80u * 4u, "Resized byte length mismatch");
    mla_buffer_reference_destroy(surface.resourceOwner);
}

inline void TestBitmapSurfaceRendering() {
    mla_ui_surface_t surface = mla_ui_bitmap_surface_create(10, 10);

    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t> commands = mla_array_list<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();

    // Draw a red rectangle covering the whole surface
    mla_ui_surface_draw_command_t rectCmd = mla_ui_surface_draw_command_empty();
    rectCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    rectCmd.rect.x = 0;
    rectCmd.rect.y = 0;
    rectCmd.rect.width = 10;
    rectCmd.rect.height = 10;
    rectCmd.rect.color = {255, 0, 0, 255};
    mla_array_list_add(commands, rectCmd);

    mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t> events = mla_array_list<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>();

    mla_ui_surface_render_draw_commands(surface, commands, events, 16);

    mla_bytes_t bytes = mla_ui_bitmap_surface_get_bytes(surface);
    const mla_byte_t* data = mla_bytes_get_data_readonly(bytes);

    // Check a few pixels
    assert_equal(data[0], (mla_byte_t)255, "Red channel mismatch");
    assert_equal(data[1], (mla_byte_t)0,   "Green channel mismatch");
    assert_equal(data[2], (mla_byte_t)0,   "Blue channel mismatch");
    assert_equal(data[3], (mla_byte_t)255, "Alpha channel mismatch");

    assert_equal(data[(5 * 10 + 5) * 4 + 0], (mla_byte_t)255, "Center pixel red channel mismatch");
    mla_buffer_reference_destroy(surface.resourceOwner);
}

inline void RegisterBitmapSurfaceTests(mla_test_executor_t& executor) {
    mla_test_t t1 = mla_test("Bitmap Surface Creation", test_category, TestBitmapSurfaceCreation);
    mla_test_executor_register_test(executor, t1);
    mla_test_t t2 = mla_test("Bitmap Surface Resizing", test_category, TestBitmapSurfaceResizing);
    mla_test_executor_register_test(executor, t2);
    mla_test_t t3 = mla_test("Bitmap Surface Rendering", test_category, TestBitmapSurfaceRendering);
    mla_test_executor_register_test(executor, t3);
}

#endif //COREOS_MLA_UI_BITMAP_SURFACE_TEST_H
