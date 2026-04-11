//
// Created for testing the mla_ui_label control.
//

#ifndef COREOS_MLA_UI_LABEL_TEST_H
#define COREOS_MLA_UI_LABEL_TEST_H

#include "../core-os/ui/controls/mla_ui_label.h"
#include "../core-os/ui/surfaces/mla_ui_surface_draw.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static mla_ui_control_context_t ui_label_make_test_context(mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

inline void UiLabelCreateTest() {
    mla_ui_control_t label = mla_ui_label();
    assert_false(mla_string_is_empty(label.id), "Label should have a non-empty ID");
    assert_true(label.renderToDrawCommands != nullptr, "Label should have a render callback");
}

inline void UiLabelSetGetTextTest() {
    mla_ui_control_t label = mla_ui_label();
    assert_true(mla_string_is_empty(mla_ui_label_get_text(label)), "Label text should be empty initially");

    assert_true(mla_ui_label_set_text(label, mla_string_const("Hello World")),
                "Setting label text should succeed");
    mla_string_t text = mla_ui_label_get_text(label);
    assert_true(mla_string_equals(text, mla_string_const("Hello World")), "Label text should be 'Hello World'");
}

inline void UiLabelSetGetFontSizeTest() {
    mla_ui_control_t label = mla_ui_label();
    assert_equal(mla_ui_label_get_font_size(label), (mla_uint16_t)MLA_UI_FONT_SIZE_DEFAULT,
                 "Default label font size should be MLA_UI_FONT_SIZE_DEFAULT");

    assert_true(mla_ui_label_set_font_size(label, (mla_uint16_t)18),
                "Setting font size should succeed");
    assert_equal(mla_ui_label_get_font_size(label), (mla_uint16_t)18,
                 "Label font size should be 18 after set");
}

inline void UiLabelSetGetTextKindTest() {
    mla_ui_control_t label = mla_ui_label();
    assert_equal(mla_ui_label_get_text_kind(label), MLA_UI_TEXT_KIND_PRIMARY,
                 "Default text kind should be PRIMARY");

    assert_true(mla_ui_label_set_text_kind(label, MLA_UI_TEXT_KIND_ERROR),
                "Setting text kind should succeed");
    assert_equal(mla_ui_label_get_text_kind(label), MLA_UI_TEXT_KIND_ERROR,
                 "Text kind should be ERROR after set");

    assert_true(mla_ui_label_set_text_kind(label, MLA_UI_TEXT_KIND_SUCCESS),
                "Setting text kind to SUCCESS should succeed");
    assert_equal(mla_ui_label_get_text_kind(label), MLA_UI_TEXT_KIND_SUCCESS,
                 "Text kind should be SUCCESS after set");
}

inline void UiLabelSetGetCustomColorTest() {
    mla_ui_control_t label = mla_ui_label();
    assert_true(mla_string_is_empty(mla_ui_label_get_custom_color(label)),
                "Custom color should be empty initially");

    assert_true(mla_ui_label_set_custom_color(label, mla_string_const("#FF5733")),
                "Setting custom color should succeed");
    mla_string_t color = mla_ui_label_get_custom_color(label);
    assert_true(mla_string_equals(color, mla_string_const("#FF5733")),
                "Custom color should be '#FF5733' after set");
}

inline void UiLabelRenderProducesTextCommandTest() {
    mla_ui_control_t label = mla_ui_label();
    label.layout = {0.0, 0.0, 200.0, 30.0};
    assert_true(mla_ui_label_set_text(label, mla_string_const("Status")),
                "Setting label text should succeed");

    mla_ui_control_context_t ctx = ui_label_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(label.renderToDrawCommands(ctx, label, drawCommands, inputAreas),
                "Rendering label should succeed");
    assert_true(mla_array_list_size(drawCommands) >= 1,
                "Label with text should produce at least 1 draw command");
    if (mla_array_list_size(drawCommands) == 0) return; // Guard against allocation-failure mode
    assert_equal(mla_array_list_get_unsafe(drawCommands, 0).kind,
                 MLA_UI_SURFACE_DRAW_COMMAND_KIND_TEXT,
                 "First draw command should be a text command");
}

inline void UiLabelEmptyTextNoRenderTest() {
    mla_ui_control_t label = mla_ui_label();
    label.layout = {0.0, 0.0, 200.0, 30.0};
    // No text set

    mla_ui_control_context_t ctx = ui_label_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(label.renderToDrawCommands(ctx, label, drawCommands, inputAreas),
                "Rendering empty label should succeed without error");
    assert_equal(mla_array_list_size(drawCommands), (mla_size_t)0,
                 "Empty label should produce no draw commands");
}

inline void UiLabelLinkKindProducesUnderlineTest() {
    mla_ui_control_t label = mla_ui_label();
    label.layout = {0.0, 0.0, 200.0, 30.0};
    assert_true(mla_ui_label_set_text(label, mla_string_const("Click here")),
                "Setting label text should succeed");
    assert_true(mla_ui_label_set_text_kind(label, MLA_UI_TEXT_KIND_LINK),
                "Setting text kind to LINK should succeed");

    mla_ui_control_context_t ctx = ui_label_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(label.renderToDrawCommands(ctx, label, drawCommands, inputAreas),
                "Rendering link label should succeed");
    // Link labels produce a text command + an underline line command
    assert_true(mla_array_list_size(drawCommands) >= 2,
                "Link label should produce at least 2 draw commands (text + underline)");
    if (mla_array_list_size(drawCommands) < 2) return; // Guard against allocation-failure mode
    assert_equal(mla_array_list_get_unsafe(drawCommands, 1).kind,
                 MLA_UI_SURFACE_DRAW_COMMAND_KIND_LINE,
                 "Second draw command should be a line (underline)");
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

inline void RegisterUILabelTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("UiLabelCreate", test_category, UiLabelCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelSetGetText", test_category, UiLabelSetGetTextTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelSetGetFontSize", test_category, UiLabelSetGetFontSizeTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelSetGetTextKind", test_category, UiLabelSetGetTextKindTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelSetGetCustomColor", test_category, UiLabelSetGetCustomColorTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelRenderProducesTextCommand", test_category, UiLabelRenderProducesTextCommandTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelEmptyTextNoRender", test_category, UiLabelEmptyTextNoRenderTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLabelLinkKindProducesUnderline", test_category, UiLabelLinkKindProducesUnderlineTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // COREOS_MLA_UI_LABEL_TEST_H
