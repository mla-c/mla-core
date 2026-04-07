//
// Created for testing the mla_ui_loading_indicator control.
//

#ifndef COREOS_MLA_UI_LOADING_INDICATOR_TEST_H
#define COREOS_MLA_UI_LOADING_INDICATOR_TEST_H

#include "../core-os/ui/controls/mla_ui_loading_indicator.h"
#include "../core-os/ui/surfaces/mla_ui_surface_draw.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static mla_ui_control_context_t ui_loading_make_test_context(mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

inline void UiLoadingIndicatorCreateTest() {
    mla_ui_control_t indicator = mla_ui_loading_indicator();
    assert_false(mla_string_is_empty(indicator.id), "Loading indicator should have a non-empty ID");
    assert_true(indicator.renderToDrawCommands != nullptr, "Loading indicator should have a render callback");
}

inline void UiLoadingIndicatorRenderProducesDrawCommandsTest() {
    mla_ui_control_t indicator = mla_ui_loading_indicator();
    indicator.layout = {0.0, 0.0, 24.0, 24.0};

    mla_ui_control_context_t ctx = ui_loading_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(indicator.renderToDrawCommands(ctx, indicator, drawCommands, inputAreas),
                "Rendering loading indicator should succeed");
    // Loading indicator produces a background circle and an active arc segment
    assert_true(mla_array_list_size(drawCommands) >= 2,
                "Loading indicator should produce at least 2 draw commands");
    if (mla_array_list_size(drawCommands) == 0) return; // Guard against allocation-failure mode
    assert_equal(mla_array_list_get_unsafe(drawCommands, 0).kind,
                 MLA_UI_SURFACE_DRAW_COMMAND_KIND_CIRCLE,
                 "First command should be the background circle");
}

inline void UiLoadingIndicatorZeroSizeNoRenderTest() {
    mla_ui_control_t indicator = mla_ui_loading_indicator();
    // Zero size – the indicator should handle it gracefully and produce no draw commands
    indicator.layout = {0.0, 0.0, 0.0, 0.0};

    mla_ui_control_context_t ctx = ui_loading_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(indicator.renderToDrawCommands(ctx, indicator, drawCommands, inputAreas),
                "Rendering zero-size loading indicator should succeed without crash");
    assert_equal(mla_array_list_size(drawCommands), (mla_size_t)0,
                 "Zero-size loading indicator should produce no draw commands");
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

inline void RegisterUILoadingIndicatorTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("UiLoadingIndicatorCreate", test_category, UiLoadingIndicatorCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLoadingIndicatorRenderProducesDrawCommands", test_category, UiLoadingIndicatorRenderProducesDrawCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLoadingIndicatorZeroSizeNoRender", test_category, UiLoadingIndicatorZeroSizeNoRenderTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // COREOS_MLA_UI_LOADING_INDICATOR_TEST_H
