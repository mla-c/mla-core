//
// Created by christian on 9/13/25.
//

#ifndef MLA_MLA_CLI_APP_TEST_H
#define MLA_MLA_CLI_APP_TEST_H

#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os/cli/mla_cli_app.h"
#include "../core-os/system/mla_stream.h"

// Test execution flags for command testing
static mla_bool_t test_command_executed = false;
static mla_string_t test_command_output = mla_string_empty();

// Test command execution function
inline mla_bool_t test_command_execute(const mla_cli_command_t& command, const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>& parameters, const mla_cli_command_execute_outstream_t& out) {
    (void)command;
    (void)parameters;
    test_command_executed = true;
    test_command_output = mla_string("Command executed successfully");
    out.write(out.userdata, test_command_output);
    return true;
}

inline void SimpleNavigationTest() {

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t subModule = mla_cli_module(mla_string_const("SubModule1"));
    mla_cli_module_t subSubModule = mla_cli_module(mla_string_const("SubSubModule1"));
    mla_array_list_add(subModule.subModules, subSubModule);

    mla_array_list_add(root.subModules, subModule);

    mla_cli_module_t subModule2 = mla_cli_module(mla_string_const("SubModule2"));
    mla_array_list_add(root.subModules, subModule2);

    mla_stream_output_t output = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, output);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should start with root module active");
    }

    // Process no input
    mla_stream_input_t input = mla_stream_noop_input();
    mla_cli_app_update_and_process_input(app, input, output);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should start with root module active");
    }

    mla_string_t buffer = mla_string("SubModule1\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    mla_cli_app_update_and_process_input(app, input, output);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("SubModule1"), "Active module should be SubModule1");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to SubModule1");
    }

    buffer = mla_string("SubSubModule1\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    mla_cli_app_update_and_process_input(app, input, output);

    if (mla_array_list_size(app.activeModules) == 3) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("SubSubModule1"), "Active module should be SubSubModule1");
    } else {
        // If the size is not 3, fail the test immediately
        assert_fail("App should have navigated to SubSubModule1");
    }

    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("SubModule1"), "Active module should be SubModule1");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated back to SubModule1");
    }

    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should have navigated back to Root");
    }

    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should remain at Root when exiting from root");
    }

}

inline void InvalidNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t subModule = mla_cli_module(mla_string_const("ValidModule"));
    mla_array_list_add(root.subModules, subModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test invalid module navigation
    mla_string_t buffer = mla_string("InvalidModule\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in root module on invalid navigation");

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should still be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should start with root module active");
    }

    // Test empty input
    buffer = mla_string("\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in root module on empty input");

    // Test whitespace only input
    buffer = mla_string("   \n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in root module on whitespace input");
}

inline void CommandExecutionTest() {
    test_command_executed = false;
    test_command_output = mla_string_empty();

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

    // Create a command with parameters
    mla_cli_command_t testCommand = mla_cli_command(mla_string_const("testcmd"));
    mla_cli_command_add_parameter(testCommand, mla_string_const("param1"), true);
    mla_cli_command_add_parameter(testCommand, mla_string_const("param2"), false);
    testCommand.execute = test_command_execute;

    mla_array_list_add(root.availableCommands, testCommand);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test command execution
    mla_string_t buffer = mla_string("testcmd --param1 value1\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    assert_equal(test_command_executed, true, "Command should have been executed");
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should remain in same module after command execution");
}

inline void MultipleModuleNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t module1 = mla_cli_module(mla_string_const("Module1"));
    mla_cli_module_t module2 = mla_cli_module(mla_string_const("Module2"));
    mla_cli_module_t module3 = mla_cli_module(mla_string_const("Module3"));

    mla_array_list_add(root.subModules, module1);
    mla_array_list_add(root.subModules, module2);
    mla_array_list_add(root.subModules, module3);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Navigate to Module2
    mla_string_t buffer = mla_string("Module2\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Module2"), "Active module should be Module2");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to Module2");
    }



    // Navigate back to root and then to Module1
    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Module1\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Module1"), "Active module should be Module1");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to Module1");
    }

}

inline void DeepNavigationTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t level1 = mla_cli_module(mla_string_const("Level1"));
    mla_cli_module_t level2 = mla_cli_module(mla_string_const("Level2"));
    mla_cli_module_t level3 = mla_cli_module(mla_string_const("Level3"));
    mla_cli_module_t level4 = mla_cli_module(mla_string_const("Level4"));

    mla_array_list_add(level3.subModules, level4);
    mla_array_list_add(level2.subModules, level3);
    mla_array_list_add(level1.subModules, level2);
    mla_array_list_add(root.subModules, level1);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Navigate deep into the hierarchy
    mla_string_t buffer = mla_string("Level1\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Level2\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Level3\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    buffer = mla_string("Level4\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);


    if (mla_array_list_size(app.activeModules) == 5) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Top module should be Root");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Level1"), "Second module should be Level1");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("Level2"), "Third module should be Level2");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 3)->moduleName, mla_string("Level3"), "Fourth module should be Level3");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 4)->moduleName, mla_string("Level4"), "Deepest module should be Level4");
    } else {
        // If the size is not 5, fail the test immediately
        assert_fail("App should be at depth 5");
    }

    // Navigate back up multiple levels
    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    buffer = mla_string("exit\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 3) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Top module should be Root");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("Level1"), "Second module should be Level1");
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("Level2"), "Active module should be Level2");
    } else {
        // If the size is not 3, fail the test immediately
        assert_fail("App should be back at Level2");
    }

}

inline void EmptyModuleTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t emptyModule = mla_cli_module(mla_string_const("EmptyModule"));

    // EmptyModule has no submodules or commands
    mla_array_list_add(root.subModules, emptyModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Navigate to empty module
    mla_string_t buffer = mla_string("EmptyModule\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("EmptyModule"), "Active module should be EmptyModule");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should have navigated to EmptyModule");
    }

    // Try to navigate to non-existent submodule
    buffer = mla_string("NonExistent\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("EmptyModule"), "Active module should still be EmptyModule");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should remain in EmptyModule on invalid navigation");
    }

}

inline void CaseSensitivityTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t testModule = mla_cli_module(mla_string_const("TestModule"));

    mla_array_list_add(root.subModules, testModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test case sensitivity - should not navigate with wrong case
    mla_string_t buffer = mla_string("testmodule\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 1) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should still be Root");
    } else {
        // If the size is not 1, fail the test immediately
        assert_fail("App should not navigate with wrong case");
    }

    // Test correct case - should navigate
    buffer = mla_string("TestModule\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);

    if (mla_array_list_size(app.activeModules) == 2) {
        assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("TestModule"), "Active module should be TestModule");
    } else {
        // If the size is not 2, fail the test immediately
        assert_fail("App should navigate with correct case");
    }

}

inline void SpecialCharacterInputTest() {
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));
    mla_cli_module_t normalModule = mla_cli_module(mla_string_const("NormalModule"));

    mla_array_list_add(root.subModules, normalModule);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Test special characters and symbols
    mla_string_t buffer = mla_string("@#$%^&*()\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should ignore special character input");

    // Test numbers
    buffer = mla_string("12345\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should ignore numeric input");

    // Test mixed alphanumeric with special chars
    buffer = mla_string("Module-123!\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should ignore mixed special character input");
}

inline void MultipleCommandsTest() {
    test_command_executed = false;

    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

    // Create multiple commands
    mla_cli_command_t cmd1 = mla_cli_command(mla_string_const("command1"));
    cmd1.execute = test_command_execute;

    mla_cli_command_t cmd2 = mla_cli_command(mla_string_const("command2"));
    cmd2.execute = test_command_execute;

    mla_cli_command_t cmd3 = mla_cli_command(mla_string_const("help"));
    cmd3.execute = test_command_execute;

    mla_array_list_add(root.availableCommands, cmd1);
    mla_array_list_add(root.availableCommands, cmd2);
    mla_array_list_add(root.availableCommands, cmd3);

    mla_stream_output_t noopOutput = mla_stream_noop_output();
    mla_cli_app_t app = mla_cli_app_init(root, noopOutput);

    // Execute different commands
    mla_string_t buffer = mla_string("command1\n");
    mla_stream_input_t input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_executed, true, "First command should execute");

    test_command_executed = false;
    buffer = mla_string("help\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_executed, true, "Help command should execute");

    test_command_executed = false;
    buffer = mla_string("nonexistent\n");
    input = mla_stream_input_from_buffer((mla_byte_t*)mla_string_data(buffer), mla_string_length(buffer));
    noopOutput = mla_stream_noop_output();
    mla_cli_app_update_and_process_input(app, input, noopOutput);
    assert_equal(test_command_executed, false, "Non-existent command should not execute");
}

inline void RegisterCliAppTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleNavigation", test_category, SimpleNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InvalidNavigation", test_category, InvalidNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CommandExecution", test_category, CommandExecutionTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultipleModuleNavigation", test_category, MultipleModuleNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("DeepNavigation", test_category, DeepNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EmptyModule", test_category, EmptyModuleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CaseSensitivity", test_category, CaseSensitivityTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SpecialCharacterInput", test_category, SpecialCharacterInputTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultipleCommands", test_category, MultipleCommandsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
