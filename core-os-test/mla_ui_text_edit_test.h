//
// Created for testing the mla_ui_text_edit control.
//

#ifndef COREOS_MLA_UI_TEXT_EDIT_TEST_H
#define COREOS_MLA_UI_TEXT_EDIT_TEST_H

#include "../core-os/ui/controls/mla_ui_text_edit.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Tests
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

static void text_edit_text_changed_handler(mla_ui_control_t &control,
                                           mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls,
                                           mla_user_data_t& userData) {
    (void)control; (void)uiControls; (void)userData;
    g_text_changed_fired = true;
}

inline void UiTextEditSetGetTextChangedEventTest() {
    mla_ui_control_t textEdit = mla_ui_text_edit();
    assert_true(mla_ui_text_edit_get_text_changed_event(textEdit) == nullptr,
                "Text changed event should be null initially");

    assert_true(mla_ui_text_edit_set_text_changed_event(textEdit, text_edit_text_changed_handler),
                "Setting text changed event should succeed");
    assert_true(mla_ui_text_edit_get_text_changed_event(textEdit) != nullptr,
                "Text changed event should not be null after set");
    assert_true(mla_ui_text_edit_get_text_changed_event(textEdit) == text_edit_text_changed_handler,
                "Text changed event should match the handler that was set");
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

inline void RegisterUITextEditTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("UiTextEditCreate", test_category, UiTextEditCreateTest);
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
}

#endif // COREOS_MLA_UI_TEXT_EDIT_TEST_H
