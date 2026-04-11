//
// Created for testing the mla_ui_button control.
//

#ifndef COREOS_MLA_UI_BUTTON_TEST_H
#define COREOS_MLA_UI_BUTTON_TEST_H

#include "../core-os/ui/controls/mla_ui_button.h"
#include "../core-os/ui/surfaces/mla_ui_surface_draw.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static mla_ui_control_context_t ui_button_make_test_context(mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

inline void UiButtonCreateTest() {
    mla_ui_control_t button = mla_ui_button();
    assert_false(mla_string_is_empty(button.id), "Button should have a non-empty ID");
    assert_true(button.renderToDrawCommands != nullptr, "Button should have a render callback");
    assert_true(button.processClickEvent != nullptr, "Button should have a click event callback");
}

inline void UiButtonSetGetTextTest() {
    mla_ui_control_t button = mla_ui_button();
    assert_true(mla_string_is_empty(mla_ui_button_get_text(button)), "Button text should be empty initially");

    assert_true(mla_ui_button_set_text(button, mla_string_const("Click Me")),
                "Setting button text should succeed");
    mla_string_t text = mla_ui_button_get_text(button);
    assert_true(mla_string_equals(text, mla_string_const("Click Me")), "Button text should be 'Click Me'");
}

inline void UiButtonSetGetStyleTest() {
    mla_ui_control_t button = mla_ui_button();
    assert_equal(mla_ui_button_get_style(button), MLA_UI_BUTTON_STYLE_PRIMARY,
                 "Default button style should be PRIMARY");

    assert_true(mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_SECONDARY),
                "Setting button style should succeed");
    assert_equal(mla_ui_button_get_style(button), MLA_UI_BUTTON_STYLE_SECONDARY,
                 "Button style should be SECONDARY after set");

    assert_true(mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_TERTIARY),
                "Setting button style to TERTIARY should succeed");
    assert_equal(mla_ui_button_get_style(button), MLA_UI_BUTTON_STYLE_TERTIARY,
                 "Button style should be TERTIARY after set");

    assert_true(mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_LINK),
                "Setting button style to LINK should succeed");
    assert_equal(mla_ui_button_get_style(button), MLA_UI_BUTTON_STYLE_LINK,
                 "Button style should be LINK after set");
}

inline void UiButtonSetGetDisabledTest() {
    mla_ui_control_t button = mla_ui_button();
    assert_false(mla_ui_button_get_disable(button), "Button should be enabled by default");

    assert_true(mla_ui_button_set_disable(button, true), "Disabling button should succeed");
    assert_true(mla_ui_button_get_disable(button), "Button should be disabled after set");

    assert_true(mla_ui_button_set_disable(button, false), "Re-enabling button should succeed");
    assert_false(mla_ui_button_get_disable(button), "Button should be enabled after re-enable");
}

static mla_bool_t g_button_clicked = false;

static void button_click_handler(mla_ui_control_t &control, const mla_ui_surface_input_event_click_t &clickEvent,
                                 mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls,
                                 mla_user_data_t& userData) {
    (void)control; (void)clickEvent; (void)uiControls; (void)userData;
    g_button_clicked = true;
}

inline void UiButtonSetGetClickEventTest() {
    mla_ui_control_t button = mla_ui_button();
    assert_true(mla_ui_button_get_click_event(button) == nullptr, "Click event should be null initially");

    assert_true(mla_ui_button_set_click_event(button, button_click_handler),
                "Setting click event should succeed");
    assert_true(mla_ui_button_get_click_event(button) != nullptr, "Click event should not be null after set");
    assert_true(mla_ui_button_get_click_event(button) == button_click_handler,
                "Click event should be the handler that was set");
}

inline void UiButtonRenderPrimaryProducesDrawCommandsTest() {
    mla_ui_control_t button = mla_ui_button();
    button.layout = {0.0, 0.0, 120.0, 32.0};
    assert_true(mla_ui_button_set_text(button, mla_string_const("Submit")),
                "Setting button text should succeed");
    assert_true(mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_PRIMARY),
                "Setting style should succeed");

    mla_ui_control_context_t ctx = ui_button_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(button.renderToDrawCommands(ctx, button, drawCommands, inputAreas),
                "Rendering primary button should succeed");
    // Primary button with text: at least a rect (bg) and a text command
    assert_true(mla_array_list_size(drawCommands) >= 2,
                "Primary button with text should produce at least 2 draw commands (rect + text)");
    // Input area should be registered for non-disabled button
    assert_equal(mla_array_list_size(inputAreas), (mla_size_t)1,
                 "Enabled button should register 1 input area");
}

inline void UiButtonRenderDisabledProducesNoInputAreaTest() {
    mla_ui_control_t button = mla_ui_button();
    button.layout = {0.0, 0.0, 120.0, 32.0};
    assert_true(mla_ui_button_set_text(button, mla_string_const("Disabled")),
                "Setting button text should succeed");
    assert_true(mla_ui_button_set_disable(button, true), "Disabling button should succeed");

    mla_ui_control_context_t ctx = ui_button_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(button.renderToDrawCommands(ctx, button, drawCommands, inputAreas),
                "Rendering disabled button should succeed");
    // Disabled button should NOT register an input area
    assert_equal(mla_array_list_size(inputAreas), (mla_size_t)0,
                 "Disabled button should not register any input areas");
}

inline void UiButtonRenderSecondaryTest() {
    mla_ui_control_t button = mla_ui_button();
    button.layout = {0.0, 0.0, 120.0, 32.0};
    assert_true(mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_SECONDARY),
                "Setting secondary style should succeed");
    assert_true(mla_ui_button_set_text(button, mla_string_const("Cancel")),
                "Setting button text should succeed");

    mla_ui_control_context_t ctx = ui_button_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_true(button.renderToDrawCommands(ctx, button, drawCommands, inputAreas),
                "Rendering secondary button should succeed");
    assert_true(mla_array_list_size(drawCommands) >= 2,
                "Secondary button with text should produce at least 2 draw commands");
}

inline void UiButtonClickEventFiredWhenEnabledTest() {
    g_button_clicked = false;

    mla_ui_control_t button = mla_ui_button();
    button.layout = {0.0, 0.0, 120.0, 32.0};
    assert_true(mla_ui_button_set_click_event(button, button_click_handler),
                "Setting click event should succeed");

    // Render to register input area
    mla_ui_control_context_t ctx = ui_button_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();
    assert_true(button.renderToDrawCommands(ctx, button, drawCommands, inputAreas),
                "Render should succeed");
    assert_equal(mla_array_list_size(inputAreas), (mla_size_t)1, "Should have 1 input area");
    if (mla_array_list_size(inputAreas) == 0) return; // Guard against null dereference in allocation-failure mode

    // Simulate left-click event
    mla_ui_surface_input_event_click_t clickEvent = {};
    clickEvent.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT;
    clickEvent.position.x = 60.0;
    clickEvent.position.y = 16.0;

    mla_ui_control_input_area_t& area = mla_array_list_get_unsafe(inputAreas, 0);
    auto uiControls = mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>();
    mla_user_data_t userData = mla_user_data_empty();

    assert_true(button.processClickEvent(button, clickEvent, area, uiControls, userData),
                "processClickEvent should return true for left click on enabled button");
    assert_true(g_button_clicked, "Click event handler should have been fired");
}

inline void UiButtonClickEventNotFiredWhenDisabledTest() {
    g_button_clicked = false;

    mla_ui_control_t button = mla_ui_button();
    button.layout = {0.0, 0.0, 120.0, 32.0};
    assert_true(mla_ui_button_set_click_event(button, button_click_handler),
                "Setting click event should succeed");
    assert_true(mla_ui_button_set_disable(button, true), "Disabling button should succeed");

    mla_ui_surface_input_event_click_t clickEvent = {};
    clickEvent.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_LEFT;

    mla_ui_control_input_area_t dummyArea = mla_ui_control_input_area_empty();
    auto uiControls = mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>();
    mla_user_data_t userData = mla_user_data_empty();

    assert_false(button.processClickEvent(button, clickEvent, dummyArea, uiControls, userData),
                 "processClickEvent should return false for disabled button");
    assert_false(g_button_clicked, "Click event handler should NOT have been fired on disabled button");
}

inline void UiButtonRightClickEventNotFiredTest() {
    g_button_clicked = false;

    mla_ui_control_t button = mla_ui_button();
    assert_true(mla_ui_button_set_click_event(button, button_click_handler),
                "Setting click event should succeed");

    mla_ui_surface_input_event_click_t clickEvent = {};
    clickEvent.button = MLA_UI_SURFACE_INPUT_EVENT_CLICK_BUTTON_RIGHT;

    mla_ui_control_input_area_t dummyArea = mla_ui_control_input_area_empty();
    auto uiControls = mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>();
    mla_user_data_t userData = mla_user_data_empty();

    assert_false(button.processClickEvent(button, clickEvent, dummyArea, uiControls, userData),
                 "processClickEvent should return false for right-click");
    assert_false(g_button_clicked, "Click event handler should NOT be fired on right-click");
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

inline void RegisterUIButtonTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("UiButtonCreate", test_category, UiButtonCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonSetGetText", test_category, UiButtonSetGetTextTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonSetGetStyle", test_category, UiButtonSetGetStyleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonSetGetDisabled", test_category, UiButtonSetGetDisabledTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonSetGetClickEvent", test_category, UiButtonSetGetClickEventTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonRenderPrimaryProducesDrawCommands", test_category, UiButtonRenderPrimaryProducesDrawCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonRenderDisabledProducesNoInputArea", test_category, UiButtonRenderDisabledProducesNoInputAreaTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonRenderSecondary", test_category, UiButtonRenderSecondaryTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonClickEventFiredWhenEnabled", test_category, UiButtonClickEventFiredWhenEnabledTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonClickEventNotFiredWhenDisabled", test_category, UiButtonClickEventNotFiredWhenDisabledTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiButtonRightClickEventNotFired", test_category, UiButtonRightClickEventNotFiredTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // COREOS_MLA_UI_BUTTON_TEST_H
