//
// Created for testing the mla_ui_control base control.
//

#ifndef COREOS_MLA_UI_CONTROL_BASE_TEST_H
#define COREOS_MLA_UI_CONTROL_BASE_TEST_H

#include "../core-os/ui/controls/mla_ui_control.h"
#include "../core-os/ui/surfaces/mla_ui_surface_draw.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static mla_ui_control_context_t ui_make_test_context(mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

static mla_ui_control_context_t ui_make_test_context_with_cursor(mla_double_t cursorX, mla_double_t cursorY, mla_double_t width = 800.0, mla_double_t height = 600.0) {
    mla_ui_surface_input_states_t inputStates = mla_ui_surface_input_states_empty();
    inputStates.cursorPosition.x = cursorX;
    inputStates.cursorPosition.y = cursorY;
    return mla_ui_control_context(width, height, inputStates, nullptr, 0);
}

// ---------------------------------------------------------------------------
// Tests
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
    mla_ui_control_context_t ctx = ui_make_test_context_with_cursor(50.0, 30.0);
    assert_true(mla_ui_control_is_hovered(ctx, control), "Control should be hovered when cursor is inside");
}

inline void UiControlIsHoveredFalseTest() {
    // Control at (10, 10) with size 100x50, cursor outside
    mla_ui_control_t control = mla_ui_control();
    control.layout = {10.0, 10.0, 100.0, 50.0};
    mla_ui_control_context_t ctx = ui_make_test_context_with_cursor(200.0, 200.0);
    assert_false(mla_ui_control_is_hovered(ctx, control), "Control should not be hovered when cursor is outside");
}

inline void UiControlIsHoveredNegativeCursorTest() {
    // Cursor at negative position
    mla_ui_control_t control = mla_ui_control();
    control.layout = {0.0, 0.0, 100.0, 100.0};
    mla_ui_control_context_t ctx = ui_make_test_context_with_cursor(-1.0, -1.0);
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

    mla_ui_control_context_t ctx = ui_make_test_context();
    auto drawCommands = mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    auto inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    assert_false(mla_ui_control_render_to_draw_commands(ctx, control, drawCommands, inputAreas),
                 "Rendering control with no callback should return false");
    assert_equal(mla_array_list_size(drawCommands), (mla_size_t)0,
                 "No draw commands should be produced for a control with no render callback");
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

inline void RegisterUIControlBaseTests(mla_test_executor_t &p_TestExecutor) {
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
}

#endif // COREOS_MLA_UI_CONTROL_BASE_TEST_H
