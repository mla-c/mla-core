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

    mla_bytes_t bytes = mla_ui_bitmap_surface_get_bytes_borrowed(surface);
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

    mla_bytes_t bytes = mla_ui_bitmap_surface_get_bytes_borrowed(surface);
    assert_equal(mla_bytes_length(bytes), 80u * 80u * 4u, "Resized byte length mismatch");
    mla_buffer_reference_destroy(surface.resourceOwner);
}

inline void TestBitmapSurfaceRendering() {
    mla_ui_surface_t surface = mla_ui_bitmap_surface_create(20, 20);

    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t> commands = mla_array_list<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();

    // 1. Semi-transparent red rectangle
    mla_ui_surface_draw_command_t rectCmd = mla_ui_surface_draw_command_empty();
    rectCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_RECT;
    rectCmd.rect.x = 0;
    rectCmd.rect.y = 0;
    rectCmd.rect.width = 10;
    rectCmd.rect.height = 10;
    rectCmd.rect.color = {255, 0, 0, 128}; // 50% alpha red
    mla_array_list_add(commands, rectCmd);

    // 2. Thick line (green, 3px)
    mla_ui_surface_draw_command_t lineCmd = mla_ui_surface_draw_command_empty();
    lineCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE;
    lineCmd.line.x1 = 0;
    lineCmd.line.y1 = 15;
    lineCmd.line.x2 = 20;
    lineCmd.line.y2 = 15;
    lineCmd.line.stroke = {0, 255, 0, 255};
    lineCmd.line.stroke_width = 3;
    mla_array_list_add(commands, lineCmd);

    // 3. Stroked circle (blue)
    mla_ui_surface_draw_command_t circleCmd = mla_ui_surface_draw_command_empty();
    circleCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE;
    circleCmd.circle.cx = 15;
    circleCmd.circle.cy = 5;
    circleCmd.circle.r = 4;
    circleCmd.circle.fill = {0, 0, 255, 255};
    circleCmd.circle.stroke = {255, 255, 255, 255};
    circleCmd.circle.stroke_width = 2;
    mla_array_list_add(commands, circleCmd);

    // 4. Text
    mla_ui_surface_draw_command_t textCmd = mla_ui_surface_draw_command_empty();
    textCmd.kind = MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT;
    textCmd.text.x = 2;
    textCmd.text.y = 2;
    textCmd.text.content = mla_string_const("A");
    textCmd.text.fill = {255, 255, 255, 255};
    textCmd.text.font_type.size = 7;
    mla_array_list_add(commands, textCmd);

    mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t> events = mla_array_list<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>();
    mla_ui_surface_render_draw_commands(surface, commands, events, 16);

    mla_bytes_t bytes = mla_ui_bitmap_surface_get_bytes_borrowed(surface);
    const mla_byte_t* data = mla_bytes_get_data_readonly(bytes);

    // Verify alpha blending (Red rect, 50% alpha over black should be 127 or 128)
    assert_true(data[0] >= 127 && data[0] <= 128, "Red channel blending mismatch");
    assert_equal(data[3], (mla_byte_t)128, "Alpha channel blending mismatch");

    // Verify thick line (Green line at y=15, 3px width means y=14, 15, 16)
    assert_equal(data[(15 * 20 + 10) * 4 + 1], (mla_byte_t)255, "Line center green mismatch");
    assert_equal(data[(14 * 20 + 10) * 4 + 1], (mla_byte_t)255, "Line top edge green mismatch");
    assert_equal(data[(16 * 20 + 10) * 4 + 1], (mla_byte_t)255, "Line bottom edge green mismatch");

    // Verify circle ring (Outer stroke white, inner fill blue)
    // Center at (15, 5), radius 4, stroke 2. Inner radius = 4 - 2 = 2.
    // Center pixel should be blue
    assert_equal(data[(5 * 20 + 15) * 4 + 2], (mla_byte_t)255, "Circle center blue mismatch");
    // Edge at (15, 5-4) = (15, 1) should be white stroke
    assert_equal(data[(1 * 20 + 15) * 4 + 0], (mla_byte_t)255, "Circle edge white mismatch");

    mla_buffer_reference_destroy(surface.resourceOwner);
    mla_array_list_destroy(commands);
    mla_array_list_destroy(events);
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
