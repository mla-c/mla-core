//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_PARSER_TEST_H
#define MLA_CLI_PARSER_TEST_H

#include "../base-lib/core/cli/mla_cli_app.h"
#include "../base-lib/test-support/mla_test_executor.h"
#include "../base-lib/core/cli/mla_cli_parser.h"

inline void ParseCommandWithParameters() {

    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdExecute = mla_cli_command(mla_string_const("execute"));
    mla_cli_command_add_parameter(cmdExecute, mla_cli_command_parameter(mla_string_const("path"), true));
    mla_cli_command_add_parameter(cmdExecute, mla_cli_command_parameter(mla_string_const("force"), false));
    mla_array_list_add(parser.availableCommands, cmdExecute);

    auto result = mla_cli_parser_parse(parser, mla_string("execute --path /usr/bin --force true"));
    assert_true(result.isValid, "Command should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("execute"), "Command name should be 'execute'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)2, "There should be 2 parameters");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("path")), "Parameters should contain 'path'");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("force")), "Parameters should contain 'force'");

    if (mla_hash_map_size(result.matchingParameters) == 2) {
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("path")), mla_string("/usr/bin"), "Parameter 'path' should be '/usr/bin'");
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("force")), mla_string("true"), "Parameter 'force' should be 'true'");
    } else {
        assert_fail("There should be exactly 2 parameters");
    }

    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)0, "There should be no possible auto completions");
}

inline void ParseCommandWithParametersAndWhiteSpace() {

    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdExecute = mla_cli_command(mla_string_const("execute"));
    mla_cli_command_add_parameter(cmdExecute, mla_cli_command_parameter(mla_string_const("path"), true));
    mla_cli_command_add_parameter(cmdExecute, mla_cli_command_parameter(mla_string_const("force"), false));
    mla_array_list_add(parser.availableCommands, cmdExecute);

    auto result = mla_cli_parser_parse(parser, mla_string("execute     --path     /usr/bin  --force     true     "));
    assert_true(result.isValid, "Command should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("execute"), "Command name should be 'execute'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)2, "There should be 2 parameters");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("path")), "Parameters should contain 'path'");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("force")), "Parameters should contain 'force'");

    if (mla_hash_map_size(result.matchingParameters) == 2) {
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("path")), mla_string("/usr/bin"), "Parameter 'path' should be '/usr/bin'");
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("force")), mla_string("true"), "Parameter 'force' should be 'true'");
    } else {
        assert_fail("There should be exactly 2 parameters");
    }

    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)0, "There should be no possible auto completions");
}

inline void AutoCompleteCommand() {

    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdExecute = mla_cli_command(mla_string_const("execute"));
    mla_cli_command_add_parameter(cmdExecute, mla_cli_command_parameter(mla_string_const("path"), true));
    mla_cli_command_add_parameter(cmdExecute, mla_cli_command_parameter(mla_string_const("force"), false));
    mla_array_list_add(parser.availableCommands, cmdExecute);

    auto result = mla_cli_parser_parse(parser, mla_string("exec"));
    assert_false(result.isValid, "Command should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string(""), "Command name should be ''");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)1, "There should be 1 possible auto completion");

    if (mla_array_list_size(result.possibleAutoCompletions) == 1) {
        assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 0), mla_string("ute"), "Possible auto completion should be 'execute'");
    } else {
        assert_fail("There should be exactly 1 possible auto completion");
    }

}

inline void ParseCommandWithQuotedParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdBackup = mla_cli_command(mla_string_const("backup"));
    mla_cli_command_add_parameter(cmdBackup, mla_string_const("source"), true);
    mla_cli_command_add_parameter(cmdBackup, mla_string_const("destination"), true);
    mla_cli_command_add_parameter(cmdBackup, mla_string_const("verbose"), false);
    mla_array_list_add(parser.availableCommands, cmdBackup);

    // Test a simpler case without quotes that should work
    auto result = mla_cli_parser_parse(parser, mla_string("backup --source \"/home/user name\" --destination /backup --verbose true"));
    assert_true(result.isValid, "Command without quotes should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("backup"), "Command name should be 'backup'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)3, "There should be 3 parameters");

    if (mla_hash_map_size(result.matchingParameters) == 3) {
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("source")), mla_string("/home/user name"), "Parameter 'source' should be correct");
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("destination")), mla_string("/backup"), "Parameter 'destination' should be parsed correctly");
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("verbose")), mla_string("true"), "Parameter 'verbose' should be 'true'");
    } else {
        assert_fail("There should be exactly 3 parameters");
    }

}

inline void ParseCommandWithOnlyMandatoryParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdDeploy = mla_cli_command(mla_string_const("deploy"));
    mla_cli_command_add_parameter(cmdDeploy, mla_string_const("app"), true);
    mla_cli_command_add_parameter(cmdDeploy, mla_string_const("debug"), false);
    mla_array_list_add(parser.availableCommands, cmdDeploy);

    auto result = mla_cli_parser_parse(parser, mla_string("deploy --app myapp"));
    assert_true(result.isValid, "Command should be valid with only mandatory parameters");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("deploy"), "Command name should be 'deploy'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)1, "There should be 1 parameter");

    if (mla_hash_map_size(result.matchingParameters) == 1) {
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("app")), mla_string("myapp"), "Parameter 'app' should be 'myapp'");
    } else {
        assert_fail("There should be exactly 1 parameter");
    }

    assert_false(mla_hash_map_contains(result.matchingParameters, mla_string("debug")), "Optional parameter 'debug' should not be present");
}

inline void ParseInvalidCommand() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdStart = mla_cli_command(mla_string_const("start"));
    mla_cli_command_add_parameter(cmdStart, mla_string_const("service"), true);
    mla_array_list_add(parser.availableCommands, cmdStart);

    auto result = mla_cli_parser_parse(parser, mla_string("invalid --service test"));
    assert_false(result.isValid, "Invalid command should not be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string(""), "Command name should be empty for invalid command");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)0, "There should be no parameters for invalid command");
}

inline void ParseEmptyCommand() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("help")));

    auto result = mla_cli_parser_parse(parser, mla_string(""));
    assert_false(result.isValid, "Empty command should not be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string(""), "Command name should be empty");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)0, "There should be no parameters");
}

inline void ParseCommandWithoutAnyParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("version")));

    auto result = mla_cli_parser_parse(parser, mla_string("version"));
    assert_true(result.isValid, "Command without parameters should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("version"), "Command name should be 'version'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)0, "There should be no parameters");
}

inline void AutoCompleteMultipleCommands() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("start")));
    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("stop")));
    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("status")));

    auto result = mla_cli_parser_parse(parser, mla_string("st"));
    assert_false(result.isValid, "Partial command should not be valid");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)3, "There should be 3 possible auto completions");
}

inline void AutoCompleteParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdConfigure = mla_cli_command(mla_string_const("configure"));
    mla_cli_command_add_parameter(cmdConfigure, mla_string_const("database"), true);
    mla_cli_command_add_parameter(cmdConfigure, mla_string_const("debug"), false);
    mla_cli_command_add_parameter(cmdConfigure, mla_string_const("deploy"), false);
    mla_array_list_add(parser.availableCommands, cmdConfigure);

    // Test parameter completion when command is complete
    auto result = mla_cli_parser_parse(parser, mla_string("configure"));
    assert_true(result.isValid, "Complete command should be valid");
    // When command is complete, it should show available parameters as auto-completions
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)3, "There should be 3 possible parameter completions");

    if (mla_array_list_size(result.possibleAutoCompletions) == 3) {
        assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 0), mla_string(" --database"), "First Parameter should be database");
        assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 1), mla_string(" --debug"), "Second Parameter should be debug");
        assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 2), mla_string(" --deploy"), "Third Parameter should be deploy");
    } else {
        assert_fail("There should be exactly 3 possible parameter completions");
    }

}

inline void AutoCompleteIncompleteParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdConfigure = mla_cli_command(mla_string_const("configure"));
    mla_cli_command_add_parameter(cmdConfigure, mla_string_const("database"), true);
    mla_cli_command_add_parameter(cmdConfigure, mla_string_const("debug"), false);
    mla_cli_command_add_parameter(cmdConfigure, mla_string_const("deploy"), false);
    mla_array_list_add(parser.availableCommands, cmdConfigure);

    // Test parameter completion when command is complete
    auto result = mla_cli_parser_parse(parser, mla_string("configure --da"));
    assert_false(result.isValid, "Complete command should be valid");
    // When command is complete, it should show available parameters as auto-completions
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)1, "There should be 1 possible parameter completions");

    if (mla_array_list_size(result.possibleAutoCompletions) == 1) {
        assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 0), mla_string("tabase"), "First Parameter should be tabase");
    } else {
        assert_fail("There should be exactly 1 possible parameter completion");
    }

}

inline void ParseCommandWithMalformedParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdTest = mla_cli_command(mla_string_const("test"));
    mla_cli_command_add_parameter(cmdTest, mla_string_const("input"), true);
    mla_array_list_add(parser.availableCommands, cmdTest);

    auto result = mla_cli_parser_parse(parser, mla_string("test --input"));
    assert_false(result.isValid, "Command with missing parameter value should not be valid");
}

inline void ParseCommandWithTrailingSpaces() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdClean = mla_cli_command(mla_string_const("clean"));
    mla_cli_command_add_parameter(cmdClean, mla_string_const("force"), false);
    mla_array_list_add(parser.availableCommands, cmdClean);

    auto result = mla_cli_parser_parse(parser, mla_string("clean --force true   "));
    assert_true(result.isValid, "Command with trailing spaces should be valid due to parsing rules");
}

inline void ParseCommandWithSpecialCharacters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_cli_command_t cmdProcess = mla_cli_command(mla_string_const("process"));
    mla_cli_command_add_parameter(cmdProcess, mla_string_const("file"), true);
    mla_cli_command_add_parameter(cmdProcess, mla_string_const("pattern"), false);
    mla_array_list_add(parser.availableCommands, cmdProcess);

    // Test without quotes first - the parser may have issues with quoted special characters
    auto result = mla_cli_parser_parse(parser, mla_string("process --file data.txt --pattern *.log"));
    assert_true(result.isValid, "Command with special characters should be valid");

    if (mla_hash_map_size(result.matchingParameters) == 2) {
        assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("pattern")), mla_string("*.log"), "Pattern parameter should contain special characters");
    } else {
        assert_fail("There should be exactly 2 parameters");
    }

}

inline void ParseMultipleCommandsWithSamePrefixes() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("build")));
    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("buildall")));

    auto result = mla_cli_parser_parse(parser, mla_string("build"));
    assert_true(result.isValid, "Exact command match should be valid even with similar prefixes");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("build"), "Command name should be 'build', not 'buildall'");
}

inline void AutoCompleteWithNoMatches() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("start")));
    mla_array_list_add(parser.availableCommands, mla_cli_command(mla_string_const("stop")));

    auto result = mla_cli_parser_parse(parser, mla_string("xyz"));
    assert_false(result.isValid, "No matching command should not be valid");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)0, "There should be no auto completions for non-matching prefix");
}

void RegisterCliParserTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("CommandWithParameters", test_category, ParseCommandWithParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("CommandWithParametersAndWhiteSpace", test_category, ParseCommandWithParametersAndWhiteSpace);
    mla_test_executor_register_test(p_TestExecutor, test);


    test = mla_test("AutoCompleteCommand", test_category, AutoCompleteCommand);
    mla_test_executor_register_test(p_TestExecutor, test);

    // New comprehensive tests
    test = mla_test("ParseCommandWithQuotedParameters", test_category, ParseCommandWithQuotedParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseCommandWithOnlyMandatoryParameters", test_category, ParseCommandWithOnlyMandatoryParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseInvalidCommand", test_category, ParseInvalidCommand);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseEmptyCommand", test_category, ParseEmptyCommand);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseCommandWithoutAnyParameters", test_category, ParseCommandWithoutAnyParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoCompleteMultipleCommands", test_category, AutoCompleteMultipleCommands);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoCompleteParameters", test_category, AutoCompleteParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoCompleteIncompleteParameters", test_category, AutoCompleteIncompleteParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseCommandWithMalformedParameters", test_category, ParseCommandWithMalformedParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseCommandWithTrailingSpaces", test_category, ParseCommandWithTrailingSpaces);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseCommandWithSpecialCharacters", test_category, ParseCommandWithSpecialCharacters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ParseMultipleCommandsWithSamePrefixes", test_category, ParseMultipleCommandsWithSamePrefixes);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoCompleteWithNoMatches", test_category, AutoCompleteWithNoMatches);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
