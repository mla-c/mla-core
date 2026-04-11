//
// Created for testing the mla_ui_window control.
//

#ifndef COREOS_MLA_UI_WINDOW_TEST_H
#define COREOS_MLA_UI_WINDOW_TEST_H

#include "../core-os/ui/controls/mla_ui_window.h"
#include "../core-os-test-support/mla_test_executor.h"

// ---------------------------------------------------------------------------
// Tests
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

static void window_close_handler(mla_ui_control_t &window) {
    (void)window;
    g_window_closed = true;
}

inline void UiWindowSetGetCloseCallbackTest() {
    mla_ui_control_t window = mla_ui_window();
    assert_true(mla_ui_window_get_close(window) == nullptr, "Close callback should be null initially");

    assert_true(mla_ui_window_set_close(window, window_close_handler),
                "Setting close callback should succeed");
    assert_true(mla_ui_window_get_close(window) != nullptr, "Close callback should not be null after set");
    assert_true(mla_ui_window_get_close(window) == window_close_handler,
                "Close callback should match the handler that was set");
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

inline void RegisterUIWindowTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("UiWindowCreate", test_category, UiWindowCreateTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiWindowSetGetTitle", test_category, UiWindowSetGetTitleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiWindowSetGetResizable", test_category, UiWindowSetGetResizableTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("UiWindowSetGetCloseCallback", test_category, UiWindowSetGetCloseCallbackTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif // COREOS_MLA_UI_WINDOW_TEST_H
