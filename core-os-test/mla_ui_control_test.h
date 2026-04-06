//
// Created for testing the UI controls package.
//

#ifndef COREOS_MLA_UI_CONTROL_TEST_H
#define COREOS_MLA_UI_CONTROL_TEST_H

#include "../core-os/ui/controls/mla_ui_control.h"
#include "../core-os/ui/controls/mla_ui_button.h"
#include "../core-os/ui/controls/mla_ui_label.h"
#include "../core-os/ui/controls/mla_ui_text_edit.h"
#include "../core-os/ui/controls/mla_ui_window.h"
#include "../core-os/ui/controls/mla_ui_loading_indicator.h"
#include "../core-os/ui/surfaces/mla_ui_surface_draw.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static mla_ui_control_context_t make_test_context(mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

static mla_ui_control_context_t make_test_context_with_cursor(mla_double_t cursorX, mla_double_t cursorY, mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    inputStates.cursorPosition.x = cursorX;
    inputStates.cursorPosition.y = cursorY;
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

// ---------------------------------------------------------------------------
// mla_ui_control tests
// ---------------------------------------------------------------------------

inline void UiControlCreateTest() {
    mla_ui_control_t control = mla_ui_control();
    // mla_ui_control() generates a runtime ID – it must not be empty
    assert_false(mla_string_is_empty(control.id), "mla_ui_control() should produce a non-empty ID");
    assert_true(control.renderToDrawCommands == nullptr, "Default render callback should be null");
}

inline void UiControlEmptyTest() {
    mla_ui_control_t control = mla_ui_control_empty();
    assert_true(mla_string_is_empty(control.id), "mla_ui_control_empty() should have an empty ID");
    assert_true(control.renderToDrawCommands == nullptr, "Default render callback should be null");
}

inline void UiControlSetGetBoolValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_bool(control, mla_string_const("active"), true),
                "Setting bool value should succeed");
    assert_true(mla_ui_control_get_value_as_bool(control, mla_string_const("active"), false),
                "Getting bool value should return true");

    assert_true(mla_ui_control_set_value_as_bool(control, mla_string_const("active"), false),
                "Overwriting bool value should succeed");
    assert_false(mla_ui_control_get_value_as_bool(control, mla_string_const("active"), true),
                 "Overwritten bool value should return false");
}

inline void UiControlDefaultBoolValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_false(mla_ui_control_get_value_as_bool(control, mla_string_const("missing"), false),
                 "Missing bool should return false default");
    assert_true(mla_ui_control_get_value_as_bool(control, mla_string_const("missing"), true),
                "Missing bool should return true default");
}

inline void UiControlSetGetInt32ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_int32(control, mla_string_const("count"), -42),
                "Setting int32 value should succeed");
    assert_equal(mla_ui_control_get_value_as_int32(control, mla_string_const("count"), 0),
                 (mla_int32_t)-42, "Getting int32 value should return -42");
}

inline void UiControlSetGetUint32ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_uint32(control, mla_string_const("size"), 1024u),
                "Setting uint32 value should succeed");
    assert_equal(mla_ui_control_get_value_as_uint32(control, mla_string_const("size"), 0u),
                 (mla_uint32_t)1024, "Getting uint32 value should return 1024");
}

inline void UiControlSetGetInt64ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_int64(control, mla_string_const("ts"), (mla_int64_t)-9000000000LL),
                "Setting int64 value should succeed");
    assert_equal(mla_ui_control_get_value_as_int64(control, mla_string_const("ts"), 0),
                 (mla_int64_t)-9000000000LL, "Getting int64 value should match");
}

inline void UiControlSetGetUint64ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_uint64(control, mla_string_const("ts"), (mla_uint64_t)9000000000ULL),
                "Setting uint64 value should succeed");
    assert_equal(mla_ui_control_get_value_as_uint64(control, mla_string_const("ts"), 0u),
                 (mla_uint64_t)9000000000ULL, "Getting uint64 value should match");
}

inline void UiControlSetGetDoubleValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_double(control, mla_string_const("ratio"), 3.14),
                "Setting double value should succeed");
    mla_double_t val = mla_ui_control_get_value_as_double(control, mla_string_const("ratio"), 0.0);
    assert_true(val > 3.139 && val < 3.141, "Getting double value should be approximately 3.14");
}

inline void UiControlSetGetStringValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_string(control, mla_string_const("label"), mla_string_const("Hello")),
                "Setting string value should succeed");
    mla_string_t val = mla_ui_control_get_value_as_string(control, mla_string_const("label"), mla_string_empty());
    assert_true(mla_string_equals(val, mla_string_const("Hello")), "Getting string value should return 'Hello'");
}

inline void UiControlDefaultStringValueTest() {
    mla_ui_control_t control = mla_ui_control();
    mla_string_t val = mla_ui_control_get_value_as_string(control, mla_string_const("missing"), mla_string_const("default"));
    assert_true(mla_string_equals(val, mla_string_const("default")), "Missing string should return default");
}

inline void UiControlSetGetPointerValueTest() {
    mla_ui_control_t control = mla_ui_control();
    mla_int32_t data = 12345;
    assert_true(mla_ui_control_set_value_as_pointer(control, mla_string_const("ptr"), &data),
                "Setting pointer value should succeed");
    mla_pointer_t ptr = mla_ui_control_get_value_as_pointer(control, mla_string_const("ptr"), nullptr);
    assert_not_null(ptr, "Getting pointer value should not be null");
    assert_true(ptr != nullptr && *static_cast<mla_int32_t*>(ptr) == 12345,
                "Pointer value should point to 12345");
}

inline void UiControlSetGetUint8ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_uint8(control, mla_string_const("byte"), (mla_uint8_t)255),
                "Setting uint8 value should succeed");
    assert_equal(mla_ui_control_get_value_as_uint8(control, mla_string_const("byte"), 0),
                 (mla_uint8_t)255, "Getting uint8 value should return 255");
}

inline void UiControlSetGetInt8ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_int8(control, mla_string_const("sbyte"), (mla_int8_t)-100),
                "Setting int8 value should succeed");
    assert_equal(mla_ui_control_get_value_as_int8(control, mla_string_const("sbyte"), 0),
                 (mla_int8_t)-100, "Getting int8 value should return -100");
}

inline void UiControlSetGetUint16ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_uint16(control, mla_string_const("u16"), (mla_uint16_t)50000),
                "Setting uint16 value should succeed");
    assert_equal(mla_ui_control_get_value_as_uint16(control, mla_string_const("u16"), 0),
                 (mla_uint16_t)50000, "Getting uint16 value should return 50000");
}

inline void UiControlSetGetInt16ValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_int16(control, mla_string_const("s16"), (mla_int16_t)-1000),
                "Setting int16 value should succeed");
    assert_equal(mla_ui_control_get_value_as_int16(control, mla_string_const("s16"), 0),
                 (mla_int16_t)-1000, "Getting int16 value should return -1000");
}

inline void UiControlSetGetFloatValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_float(control, mla_string_const("fval"), 2.5f),
                "Setting float value should succeed");
    mla_float_t val = mla_ui_control_get_value_as_float(control, mla_string_const("fval"), 0.0f);
    assert_true(val > 2.49f && val < 2.51f, "Getting float value should be approximately 2.5");
}

inline void UiControlOverwriteValueTest() {
    mla_ui_control_t control = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_int32(control, mla_string_const("x"), 10),
                "First set should succeed");
    assert_equal(mla_ui_control_get_value_as_int32(control, mla_string_const("x"), 0),
                 (mla_int32_t)10, "First value should be 10");

    assert_true(mla_ui_control_set_value_as_int32(control, mla_string_const("x"), 20),
                "Overwrite should succeed");
    assert_equal(mla_ui_control_get_value_as_int32(control, mla_string_const("x"), 0),
                 (mla_int32_t)20, "Overwritten value should be 20");
}

inline void UiControlAddChildTest() {
    mla_ui_control_t parent = mla_ui_control();
    mla_ui_control_t child = mla_ui_control();

    assert_equal(mla_array_list_size(parent.children), (mla_size_t)0, "Parent should have no children initially");
    assert_true(mla_ui_control_add_child(parent, child), "Adding child should succeed");
    assert_equal(mla_array_list_size(parent.children), (mla_size_t)1, "Parent should have one child after add");
}

inline void UiControlFindByIdTest() {
    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> controls =
        mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>();

    mla_ui_control_t ctrl1 = mla_ui_control();
    mla_ui_control_t ctrl2 = mla_ui_control();
    mla_string_t id1 = ctrl1.id;
    mla_string_t id2 = ctrl2.id;

    assert_true(mla_array_list_add(controls, ctrl1), "Adding control 1 should succeed");
    assert_true(mla_array_list_add(controls, ctrl2), "Adding control 2 should succeed");

    mla_ui_control_t* found = nullptr;
    assert_true(mla_ui_control_find_by_id(controls, id1, found), "Should find control 1");
    assert_not_null(found, "Found control 1 should not be null");
    assert_true(found != nullptr && mla_string_equals(found->id, id1), "Found control should match id1");

    found = nullptr;
    assert_true(mla_ui_control_find_by_id(controls, id2, found), "Should find control 2");
    assert_not_null(found, "Found control 2 should not be null");
    assert_true(found != nullptr && mla_string_equals(found->id, id2), "Found control should match id2");
}

inline void UiControlFindByIdNotFoundTest() {
    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> controls =
        mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>();

    mla_ui_control_t ctrl = mla_ui_control();
    assert_true(mla_array_list_add(controls, ctrl), "Adding control should succeed");

    mla_ui_control_t* found = nullptr;
    assert_false(mla_ui_control_find_by_id(controls, mla_string_const("nonexistent-id"), found),
                 "Should not find a control with a nonexistent ID");
}

inline void UiControlIsHoveredTrueTest() {
    // Control at (10, 10) with size 100x50, cursor inside
    mla_ui_control_t control = mla_ui_control();
    control.layout = {10.0, 10.0, 100.0, 50.0};
    mla_ui_control_context_t ctx = make_test_context_with_cursor(50.0, 30.0);
    assert_true(mla_ui_control_is_hovered(ctx, control), "Control should be hovered when cursor is inside");
}

inline void UiControlIsHoveredFalseTest() {
    // Control at (10, 10) with size 100x50, cursor outside
    mla_ui_control_t control = mla_ui_control();
    control.layout = {10.0, 10.0, 100.0, 50.0};
    mla_ui_control_context_t ctx = make_test_context_with_cursor(200.0, 200.0);
    assert_false(mla_ui_control_is_hovered(ctx, control), "Control should not be hovered when cursor is outside");
}

inline void UiControlIsHoveredNegativeCursorTest() {
    // Cursor at negative position
    mla_ui_control_t control = mla_ui_control();
    control.layout = {0.0, 0.0, 100.0, 100.0};
    mla_ui_control_context_t ctx = make_test_context_with_cursor(-1.0, -1.0);
    assert_false(mla_ui_control_is_hovered(ctx, control), "Control should not be hovered with negative cursor");
}

inline void UiControlResetValuesTest() {
    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> controls =
        mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>();

    mla_ui_control_t ctrl = mla_ui_control();
    assert_true(mla_ui_control_set_value_as_bool(ctrl, mla_string_const("hasFocus"), true),
                "Setting hasFocus should succeed");
    assert_true(mla_ui_control_get_value_as_bool(ctrl, mla_string_const("hasFocus"), false),
                "hasFocus should be true before reset");

    assert_true(mla_array_list_add(controls, ctrl), "Adding control should succeed");

    mla_ui_control_reset_values(controls, mla_string_const("hasFocus"));

    mla_ui_control_t* found = nullptr;
    assert_true(mla_ui_control_find_by_id(controls, ctrl.id, found), "Should find control after reset");
    assert_not_null(found, "Found control should not be null");
    assert_true(found == nullptr || !mla_ui_control_get_value_as_bool(*found, mla_string_const("hasFocus"), false),
                "hasFocus should be false after reset");
}

inline void UiControlRenderToDrawCommandsTest() {
    // A control with no render callback returns false
    mla_ui_control_t control = mla_ui_control();
    control.layout = {0.0, 0.0, 100.0, 50.0};

    mla_ui_control_context_t ctx = make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_false(mla_ui_control_render_to_draw_commands(ctx, control, drawCommands, inputAreas),
                 "Rendering control with no callback should return false");
    assert_equal(mla_array_list_size(drawCommands), (mla_size_t)0,
                 "No draw commands should be produced for a control with no render callback");
}

// ---------------------------------------------------------------------------
// mla_ui_button tests
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

static void test_button_click_handler(mla_ui_control_t &control, const mla_ui_surface_input_event_click_t &clickEvent,
                                         mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls,
                                         mla_user_data_t& userData) {
    (void)control; (void)clickEvent; (void)uiControls; (void)userData;
    g_button_clicked = true;
}

inline void UiButtonSetGetClickEventTest() {
    mla_ui_control_t button = mla_ui_button();
    assert_true(mla_ui_button_get_click_event(button) == nullptr, "Click event should be null initially");

    assert_true(mla_ui_button_set_click_event(button, test_button_click_handler),
                "Setting click event should succeed");
    assert_true(mla_ui_button_get_click_event(button) != nullptr, "Click event should not be null after set");
    assert_true(mla_ui_button_get_click_event(button) == test_button_click_handler,
                "Click event should be the handler that was set");
}

inline void UiButtonRenderPrimaryProducesDrawCommandsTest() {
    mla_ui_control_t button = mla_ui_button();
    button.layout = {0.0, 0.0, 120.0, 32.0};
    assert_true(mla_ui_button_set_text(button, mla_string_const("Submit")),
                "Setting button text should succeed");
    assert_true(mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_PRIMARY),
                "Setting style should succeed");

    mla_ui_control_context_t ctx = make_test_context();
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

    mla_ui_control_context_t ctx = make_test_context();
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

    mla_ui_control_context_t ctx = make_test_context();
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
    assert_true(mla_ui_button_set_click_event(button, test_button_click_handler),
                "Setting click event should succeed");

    // Render to register input area
    mla_ui_control_context_t ctx = make_test_context();
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
    assert_true(mla_ui_button_set_click_event(button, test_button_click_handler),
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
    assert_true(mla_ui_button_set_click_event(button, test_button_click_handler),
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
// mla_ui_label tests
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

    mla_ui_control_context_t ctx = make_test_context();
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

    mla_ui_control_context_t ctx = make_test_context();
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

    mla_ui_control_context_t ctx = make_test_context();
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
// mla_ui_text_edit tests
// ---------------------------------------------------------------------------

inline void UiTextEditCreateTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_false(mla_string_is_empty(textEdit.id), "TextEdit should have a non-empty ID");
    assert_true(textEdit.renderToDrawCommands != nullptr, "TextEdit should have a render callback");
}

inline void UiTextEditSetGetTextTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_true(mla_string_is_empty(mla_ui_text_edit_get_text(textEdit)),
                "TextEdit text should be empty initially");

    assert_true(mla_ui_text_edit_set_text(textEdit, mla_string_const("user@example.com")),
                "Setting TextEdit text should succeed");
    mla_string_t text = mla_ui_text_edit_get_text(textEdit);
    assert_true(mla_string_equals(text, mla_string_const("user@example.com")),
                "TextEdit text should match the set value");
}

inline void UiTextEditSetGetStyleTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_equal(mla_ui_text_edit_get_style(textEdit), MLA_UI_TEXT_EDIT_STYLE_STANDARD,
                 "Default TextEdit style should be STANDARD");

    assert_true(mla_ui_text_edit_set_style(textEdit, MLA_UI_TEXT_EDIT_STYLE_PASSWORD),
                "Setting TextEdit style to PASSWORD should succeed");
    assert_equal(mla_ui_text_edit_get_style(textEdit), MLA_UI_TEXT_EDIT_STYLE_PASSWORD,
                 "TextEdit style should be PASSWORD after set");

    assert_true(mla_ui_text_edit_set_style(textEdit, MLA_UI_TEXT_EDIT_STYLE_STANDARD),
                "Restoring TextEdit style to STANDARD should succeed");
    assert_equal(mla_ui_text_edit_get_style(textEdit), MLA_UI_TEXT_EDIT_STYLE_STANDARD,
                 "TextEdit style should be STANDARD after restore");
}

inline void UiTextEditSetGetDisabledTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_false(mla_ui_text_edit_get_disable(textEdit), "TextEdit should be enabled by default");

    assert_true(mla_ui_text_edit_set_disable(textEdit, true),
                "Disabling TextEdit should succeed");
    assert_true(mla_ui_text_edit_get_disable(textEdit), "TextEdit should be disabled after set");

    assert_true(mla_ui_text_edit_set_disable(textEdit, false),
                "Re-enabling TextEdit should succeed");
    assert_false(mla_ui_text_edit_get_disable(textEdit), "TextEdit should be enabled after re-enable");
}

inline void UiTextEditSetGetCursorPositionTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_true(mla_ui_text_edit_set_text(textEdit, mla_string_const("hello")),
                "Setting text should succeed");

    assert_true(mla_ui_text_edit_set_cursor_position(textEdit, 3),
                "Setting cursor position should succeed");
    assert_equal(mla_ui_text_edit_get_cursor_position(textEdit), (mla_size_t)3,
                 "Cursor position should be 3 after set");

    assert_true(mla_ui_text_edit_set_cursor_position(textEdit, 0),
                "Setting cursor position to 0 should succeed");
    assert_equal(mla_ui_text_edit_get_cursor_position(textEdit), (mla_size_t)0,
                 "Cursor position should be 0 after set");
}

inline void UiTextEditSetGetSelectedTextTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_true(mla_string_is_empty(mla_ui_text_edit_get_selected_text(textEdit)),
                "Selected text should be empty initially");

    // Must set text first; selection must be a substring of the current text
    assert_true(mla_ui_text_edit_set_text(textEdit, mla_string_const("hello")),
                "Setting text should succeed");
    assert_true(mla_ui_text_edit_set_selected_text(textEdit, mla_string_const("ell")),
                "Setting selected text should succeed");
    mla_string_t selected = mla_ui_text_edit_get_selected_text(textEdit);
    assert_true(mla_string_equals(selected, mla_string_const("ell")),
                "Selected text should match the set value");
}

static mla_bool_t g_text_changed_fired = false;

static void test_text_changed_handler(mla_ui_control_t &control,
                                         mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls,
                                         mla_user_data_t& userData) {
    (void)control; (void)uiControls; (void)userData;
    g_text_changed_fired = true;
}

inline void UiTextEditSetGetTextChangedEventTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_true(mla_ui_text_edit_get_text_changed_event(textEdit) == nullptr,
                "Text changed event should be null initially");

    assert_true(mla_ui_text_edit_set_text_changed_event(textEdit, test_text_changed_handler),
                "Setting text changed event should succeed");
    assert_true(mla_ui_text_edit_get_text_changed_event(textEdit) != nullptr,
                    "Text changed event should not be null after set");
    assert_true(mla_ui_text_edit_get_text_changed_event(textEdit) == test_text_changed_handler,
                "Text changed event should match the handler that was set");
}

// ---------------------------------------------------------------------------
// mla_ui_window tests
// ---------------------------------------------------------------------------

inline void UiWindowCreateTest() {
    mla_ui_control_t window = mla_ui_window();
    assert_false(mla_string_is_empty(window.id), "Window should have a non-empty ID");
}

inline void UiWindowSetGetTitleTest() {
    mla_ui_control_t window = mla_ui_window();
    assert_true(mla_string_is_empty(mla_ui_window_get_title(window)),
                "Window title should be empty initially");

    assert_true(mla_ui_window_set_title(window, mla_string_const("Settings")),
                "Setting window title should succeed");
    mla_string_t title = mla_ui_window_get_title(window);
    assert_true(mla_string_equals(title, mla_string_const("Settings")),
                "Window title should be 'Settings' after set");
}

inline void UiWindowSetGetResizableTest() {
    mla_ui_control_t window = mla_ui_window();
    assert_true(mla_ui_window_get_resizable(window), "Window should be resizable by default");

    assert_true(mla_ui_window_set_resizable(window, false),
                "Setting window not resizable should succeed");
    assert_false(mla_ui_window_get_resizable(window), "Window should not be resizable after set");

    assert_true(mla_ui_window_set_resizable(window, true),
                "Restoring window resizable should succeed");
    assert_true(mla_ui_window_get_resizable(window), "Window should be resizable again");
}

static mla_bool_t g_window_closed = false;

static void test_window_close_handler(mla_ui_control_t &window) {
    (void)window;
    g_window_closed = true;
}

inline void UiWindowSetGetCloseCallbackTest() {
    mla_ui_control_t window = mla_ui_window();
    assert_true(mla_ui_window_get_close(window) == nullptr, "Close callback should be null initially");

    assert_true(mla_ui_window_set_close(window, test_window_close_handler),
                "Setting close callback should succeed");
    assert_true(mla_ui_window_get_close(window) != nullptr, "Close callback should not be null after set");
    assert_true(mla_ui_window_get_close(window) == test_window_close_handler,
                "Close callback should match the handler that was set");
}

// ---------------------------------------------------------------------------
// mla_ui_loading_indicator tests
// ---------------------------------------------------------------------------

inline void UiLoadingIndicatorCreateTest() {
    mla_ui_control_t indicator = mla_ui_loading_indicator();
    assert_false(mla_string_is_empty(indicator.id), "Loading indicator should have a non-empty ID");
    assert_true(indicator.renderToDrawCommands != nullptr, "Loading indicator should have a render callback");
}

inline void UiLoadingIndicatorRenderProducesDrawCommandsTest() {
    mla_ui_control_t indicator = mla_ui_loading_indicator();
    indicator.layout = {0.0, 0.0, 24.0, 24.0};

    mla_ui_control_context_t ctx = make_test_context();
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

    mla_ui_control_context_t ctx = make_test_context();
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

inline void RegisterUIControlTests(mla_test_executor_t &p_TestExecutor) {
    // mla_ui_control
    mla_test_t test = mla_test("UiControlCreate", test_category, UiControlCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlEmpty", test_category, UiControlEmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetBoolValue", test_category, UiControlSetGetBoolValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlDefaultBoolValue", test_category, UiControlDefaultBoolValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetInt32Value", test_category, UiControlSetGetInt32ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetUint32Value", test_category, UiControlSetGetUint32ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetInt64Value", test_category, UiControlSetGetInt64ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetUint64Value", test_category, UiControlSetGetUint64ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetDoubleValue", test_category, UiControlSetGetDoubleValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetStringValue", test_category, UiControlSetGetStringValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlDefaultStringValue", test_category, UiControlDefaultStringValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetPointerValue", test_category, UiControlSetGetPointerValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetUint8Value", test_category, UiControlSetGetUint8ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetInt8Value", test_category, UiControlSetGetInt8ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetUint16Value", test_category, UiControlSetGetUint16ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetInt16Value", test_category, UiControlSetGetInt16ValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlSetGetFloatValue", test_category, UiControlSetGetFloatValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlOverwriteValue", test_category, UiControlOverwriteValueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlAddChild", test_category, UiControlAddChildTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlFindById", test_category, UiControlFindByIdTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlFindByIdNotFound", test_category, UiControlFindByIdNotFoundTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlIsHoveredTrue", test_category, UiControlIsHoveredTrueTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlIsHoveredFalse", test_category, UiControlIsHoveredFalseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlIsHoveredNegativeCursor", test_category, UiControlIsHoveredNegativeCursorTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlResetValues", test_category, UiControlResetValuesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiControlRenderToDrawCommands", test_category, UiControlRenderToDrawCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    // mla_ui_button
    test = mla_test("UiButtonCreate", test_category, UiButtonCreateTest);
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

    // mla_ui_label
    test = mla_test("UiLabelCreate", test_category, UiLabelCreateTest);
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

    // mla_ui_text_edit
    test = mla_test("UiTextEditCreate", test_category, UiTextEditCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiTextEditSetGetText", test_category, UiTextEditSetGetTextTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiTextEditSetGetStyle", test_category, UiTextEditSetGetStyleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiTextEditSetGetDisabled", test_category, UiTextEditSetGetDisabledTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiTextEditSetGetCursorPosition", test_category, UiTextEditSetGetCursorPositionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiTextEditSetGetSelectedText", test_category, UiTextEditSetGetSelectedTextTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiTextEditSetGetTextChangedEvent", test_category, UiTextEditSetGetTextChangedEventTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    // mla_ui_window
    test = mla_test("UiWindowCreate", test_category, UiWindowCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiWindowSetGetTitle", test_category, UiWindowSetGetTitleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiWindowSetGetResizable", test_category, UiWindowSetGetResizableTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiWindowSetGetCloseCallback", test_category, UiWindowSetGetCloseCallbackTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    // mla_ui_loading_indicator
    test = mla_test("UiLoadingIndicatorCreate", test_category, UiLoadingIndicatorCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLoadingIndicatorRenderProducesDrawCommands", test_category, UiLoadingIndicatorRenderProducesDrawCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiLoadingIndicatorZeroSizeNoRender", test_category, UiLoadingIndicatorZeroSizeNoRenderTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // COREOS_MLA_UI_CONTROL_TEST_H
