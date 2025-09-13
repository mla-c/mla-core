//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_CLI_PARSER_TEST_H
#define COREOS_MLA_CLI_PARSER_TEST_H

#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"
#include "../core-os/cli/mla_cli_parser.h"

inline void ParseCommandWithoutParameters() {

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

void RegisterCliParserTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("CommandWithoutParameters", test_category, ParseCommandWithoutParameters);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoCompleteCommand", test_category, AutoCompleteCommand);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
