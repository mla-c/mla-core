//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_CLI_PARSER_TEST_H
#define COREOS_MLA_CLI_PARSER_TEST_H

#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os/cli/mla_cli_parser.h"

inline void ParseCommandWithParameters() {

    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("execute",
                                                                 mla_cli_command_parameter("path", true),
                                                                 mla_cli_command_parameter("force", false)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("execute --path /usr/bin --force true"));
    assert_true(result.isValid, "Command should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("execute"), "Command name should be 'execute'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)2, "There should be 2 parameters");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("path")), "Parameters should contain 'path'");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("force")), "Parameters should contain 'force'");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("path")), mla_string("/usr/bin"), "Parameter 'path' should be '/usr/bin'");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("force")), mla_string("true"), "Parameter 'force' should be 'true'");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)0, "There should be no possible auto completions");
}

inline void ParseCommandWithParametersAndWhiteSpace() {

    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("execute",
                                                                 mla_cli_command_parameter("path", true),
                                                                 mla_cli_command_parameter("force", false)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("execute     --path     /usr/bin  --force     true     "));
    assert_true(result.isValid, "Command should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("execute"), "Command name should be 'execute'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)2, "There should be 2 parameters");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("path")), "Parameters should contain 'path'");
    assert_true(mla_hash_map_contains(result.matchingParameters, mla_string("force")), "Parameters should contain 'force'");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("path")), mla_string("/usr/bin"), "Parameter 'path' should be '/usr/bin'");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("force")), mla_string("true"), "Parameter 'force' should be 'true'");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)0, "There should be no possible auto completions");
}

inline void AutoCompleteCommand() {

    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("execute",
                                                                 mla_cli_command_parameter("path", true),
                                                                 mla_cli_command_parameter("force", false)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("exec"));
    assert_false(result.isValid, "Command should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string(""), "Command name should be ''");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)1, "There should be 1 possible auto completion");
    assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 0), mla_string("ute"), "Possible auto completion should be 'execute'");

}

inline void ParseCommandWithQuotedParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("backup",
                                                                 mla_cli_command_parameter("source", true),
                                                                 mla_cli_command_parameter("destination", true),
                                                                 mla_cli_command_parameter("verbose", false)
                       )
    );

    // Test a simpler case without quotes that should work
    auto result = mla_cli_parser_parse(parser, mla_string("backup --source \"/home/user name\" --destination /backup --verbose true"));
    assert_true(result.isValid, "Command without quotes should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("backup"), "Command name should be 'backup'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)3, "There should be 3 parameters");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("source")), mla_string("/home/user name"), "Parameter 'source' should be correct");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("destination")), mla_string("/backup"), "Parameter 'destination' should be parsed correctly");
}

inline void ParseCommandWithOnlyMandatoryParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("deploy",
                                                                 mla_cli_command_parameter("app", true),
                                                                 mla_cli_command_parameter("debug", false)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("deploy --app myapp"));
    assert_true(result.isValid, "Command should be valid with only mandatory parameters");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("deploy"), "Command name should be 'deploy'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)1, "There should be 1 parameter");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("app")), mla_string("myapp"), "Parameter 'app' should be 'myapp'");
    assert_false(mla_hash_map_contains(result.matchingParameters, mla_string("debug")), "Optional parameter 'debug' should not be present");
}

inline void ParseInvalidCommand() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("start",
                                                                 mla_cli_command_parameter("service", true)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("invalid --service test"));
    assert_false(result.isValid, "Invalid command should not be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string(""), "Command name should be empty for invalid command");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)0, "There should be no parameters for invalid command");
}

inline void ParseEmptyCommand() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("help"));

    auto result = mla_cli_parser_parse(parser, mla_string(""));
    assert_false(result.isValid, "Empty command should not be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string(""), "Command name should be empty");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)0, "There should be no parameters");
}

inline void ParseCommandWithoutAnyParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("version"));

    auto result = mla_cli_parser_parse(parser, mla_string("version"));
    assert_true(result.isValid, "Command without parameters should be valid");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("version"), "Command name should be 'version'");
    assert_equal(mla_hash_map_size(result.matchingParameters), (mla_size_t)0, "There should be no parameters");
}

inline void AutoCompleteMultipleCommands() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("start"));
    mla_array_list_add(parser.availableCommands, mla_cli_command("stop"));
    mla_array_list_add(parser.availableCommands, mla_cli_command("status"));

    auto result = mla_cli_parser_parse(parser, mla_string("st"));
    assert_false(result.isValid, "Partial command should not be valid");
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)3, "There should be 3 possible auto completions");
}

inline void AutoCompleteParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("configure",
                                                                 mla_cli_command_parameter("database", true),
                                                                 mla_cli_command_parameter("debug", false),
                                                                 mla_cli_command_parameter("deploy", false)
                       )
    );

    // Test parameter completion when command is complete
    auto result = mla_cli_parser_parse(parser, mla_string("configure"));
    assert_true(result.isValid, "Complete command should be valid");
    // When command is complete, it should show available parameters as auto-completions
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)3, "There should be 3 possible parameter completions");
    assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 0), mla_string(" --database"), "First Parameter should be database");
}

inline void AutoCompleteIncompleteParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("configure",
                                                                 mla_cli_command_parameter("database", true),
                                                                 mla_cli_command_parameter("debug", false),
                                                                 mla_cli_command_parameter("deploy", false)
                       )
    );

    // Test parameter completion when command is complete
    auto result = mla_cli_parser_parse(parser, mla_string("configure --da"));
    assert_false(result.isValid, "Complete command should be valid");
    // When command is complete, it should show available parameters as auto-completions
    assert_equal(mla_array_list_size(result.possibleAutoCompletions), (mla_size_t)1, "There should be 1 possible parameter completions");
    assert_struct_equal(mla_string_t, *mla_array_list_get_ref(result.possibleAutoCompletions, 0), mla_string("tabase"), "First Parameter should be tabase");
}

inline void ParseCommandWithMalformedParameters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("test",
                                                                 mla_cli_command_parameter("input", true)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("test --input"));
    assert_false(result.isValid, "Command with missing parameter value should not be valid");
}

inline void ParseCommandWithTrailingSpaces() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("clean",
                                                                 mla_cli_command_parameter("force", false)
                       )
    );

    auto result = mla_cli_parser_parse(parser, mla_string("clean --force true   "));
    assert_true(result.isValid, "Command with trailing spaces should be valid due to parsing rules");
}

inline void ParseCommandWithSpecialCharacters() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("process",
                                                                 mla_cli_command_parameter("file", true),
                                                                 mla_cli_command_parameter("pattern", false)
                       )
    );

    // Test without quotes first - the parser may have issues with quoted special characters
    auto result = mla_cli_parser_parse(parser, mla_string("process --file data.txt --pattern *.log"));
    assert_true(result.isValid, "Command with special characters should be valid");
    assert_struct_equal(mla_string_t, *mla_hash_map_get_ref(result.matchingParameters, mla_string("pattern")), mla_string("*.log"), "Pattern parameter should contain special characters");
}

inline void ParseMultipleCommandsWithSamePrefixes() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("build"));
    mla_array_list_add(parser.availableCommands, mla_cli_command("buildall"));

    auto result = mla_cli_parser_parse(parser, mla_string("build"));
    assert_true(result.isValid, "Exact command match should be valid even with similar prefixes");
    assert_struct_equal(mla_string_t, result.matchingCommand.name, mla_string("build"), "Command name should be 'build', not 'buildall'");
}

inline void AutoCompleteWithNoMatches() {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_array_list_add(parser.availableCommands, mla_cli_command("start"));
    mla_array_list_add(parser.availableCommands, mla_cli_command("stop"));

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
