//
// Created by christian on 8/8/25.
//

#ifndef MLA_LOGGER_CONSOLE_TEST_H
#define MLA_LOGGER_CONSOLE_TEST_H

#include "../framework/core/log/mla_logger_console.h"
#include "../framework/core/log/mla_logging.h"
#include "../framework/core-test-support/mla_test_executor.h"

void LoggerConsoleTest() {

    assert_false(mla_log_to_console_is_active(), "Console logger should not be active");
    // Activate the console logger
    assert_true(mla_log_to_console_activate(), "Console logger should be activated");
    assert_true(mla_log_to_console_is_active(), "Console logger should be active");

    assert_true(mla_log_to_console_set_level(MLA_LOG_LEVEL_VERBOSE), "Console logger level should be set to VERBOSE");
    assert_equal(mla_log_to_console_get_level(), MLA_LOG_LEVEL_VERBOSE, "Console logger level should be VERBOSE");

    // Log some messages at different levels
    mla_log_msg(MLA_LOG_LEVEL_VERBOSE, "This is a verbose message");
    mla_verbose("This is a verbose message");
    mla_log_msg(MLA_LOG_LEVEL_DEBUG, "This is a debug message");
    mla_debug("This is a debug message");
    mla_log_msg(MLA_LOG_LEVEL_INFO, "This is an info message");
    mla_info("This is an info message");
    mla_log_msg(MLA_LOG_LEVEL_WARNING, "This is a warning message");
    mla_warning("This is a warning message");
    mla_log_msg(MLA_LOG_LEVEL_ERROR, "This is an error message");
    mla_error("This is an error message");

    // Deactivate the console logger
    assert_true(mla_log_to_console_deactivate(), "Console logger should be deactivated");
    assert_false(mla_log_to_console_is_active(), "Console logger should not be active");
}


void RegisterLoggerConsoleTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("LoggerConsole", test_category, LoggerConsoleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}


#endif //MLA_LOGGER_CONSOLE_TEST_H
