//
// Created by christian on 9/13/25.
//

#include "mla_cli_parser.h"

#include "../system/mla_string_concat.h"

mla_cli_parser_t mla_cli_parser() {
    return {
        mla_array_list<mla_cli_command_t, mla_cli_command_initializer>()
    };
}

mla_cli_parser_result mla_cli_parser_parse(const mla_cli_parser_t &parser, const mla_string_t &command) {
    mla_cli_parser_result result = {
        false,
        mla_cli_command_initializer::init(),
        mla_hash_map_empty<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer,
            mla_string_initializer>(),
        mla_array_list_empty<mla_string_t, mla_string_initializer>()
    };

    // We will parse the command char by char with following rules
    // 1. The command starts with the command name followed by a space
    // 2. After the command name, we can have parameters in the form of --parameterName parameterValue

    // Parameter values can be sounded by quotes to allow spaces in the value

    // if not all parts of the command are successfully aligned to a command is not valid

    mla_size_t matchedPositon = 0;

    // We are looking for the command name
    for (mla_size_t j = 0; j < mla_array_list_size(parser.availableCommands); j++) {
        const mla_cli_command_t &cmd = mla_array_list_get_unsafe(parser.availableCommands, j);

        if (mla_string_starts_with(command, cmd.name)) {
            // Check for Perfect Match
            if (command.length == cmd.name.length || command.data[cmd.name.length] == ' ') {
                result.matchingCommand = cmd;
                matchedPositon = cmd.name.length;
                break;
            }
        }
    }

    if (result.matchingCommand.name.length == 0) {

        // Autocomplete possible commands
        for (mla_size_t j = 0; j < mla_array_list_size(parser.availableCommands); j++) {
            const mla_cli_command_t &cmd = mla_array_list_get_unsafe(parser.availableCommands, j);
            if (mla_string_starts_with(cmd.name, command)) {
                mla_array_list_add(result.possibleAutoCompletions,
                                   mla_string_substr(cmd.name, command.length));
            }
        }

        // No matching command found
        return result;
    }

    // Skip whitespace between command and parameters
    while (matchedPositon + 1 < command.length && command.data[matchedPositon] == ' ' && command.data[matchedPositon + 1] == ' ') {
        matchedPositon++;
    }


    // Match the parameters
    while (matchedPositon + 3 < command.length) {
        // Check if we are at the beginning of a parameter
        if (command.data[matchedPositon] != ' ' || command.data[matchedPositon + 1] != '-' || command.data[
                matchedPositon + 2] != '-') {
            break;
        }

        // Find the end of the parameter name
        mla_size_t paramNameStart = matchedPositon + 3;
        mla_size_t paramNameEnd = paramNameStart;
        while (paramNameEnd < command.length && command.data[paramNameEnd] != ' ') {
            paramNameEnd++;
        }

        if (paramNameEnd == paramNameStart) {
            // No parameter name found
            break;
        }

        if (command.data[paramNameEnd] != ' ') {
            // Not finished
            break;
        }

        matchedPositon = paramNameEnd;
        mla_string_t paramName = mla_string_substr(command, paramNameStart, paramNameEnd - paramNameStart);

        // Skip whitespace between parameter name and value
        while (matchedPositon + 1 < command.length && command.data[matchedPositon] == ' ' && command.data[matchedPositon + 1] == ' ') {
            matchedPositon++;
        }

        // Extract the parameter value
        mla_size_t paramValueStart = matchedPositon;

        // Check if the parameter is quoted
        mla_bool_t isQuoted = false;

        if (paramValueStart < command.length && command.data[paramValueStart] == ' ') {
            paramValueStart++;

            if (paramValueStart < command.length && command.data[paramValueStart] == '\"') {
                isQuoted = true;
                paramValueStart++;
            }
        }

        if (paramValueStart >= command.length) {
            // No parameter value found
            break;
        }

        // Find the end of the parameter value
        mla_size_t paramValueEnd = paramValueStart;
        if (isQuoted) {
            while (paramValueEnd < command.length && command.data[paramValueEnd] != '\"') {
                paramValueEnd++;
            }
            if (paramValueEnd < command.length && command.data[paramValueEnd] != '\"') {
                // Not finished
                break;
            }
        } else {
            while (paramValueEnd < command.length && command.data[paramValueEnd] != ' ') {
                paramValueEnd++;
            }
        }

        // Extract the parameter value
        if (paramValueEnd == paramValueStart) {
            // No parameter value found
            break;
        }

        mla_string_t paramValue = mla_string_empty();

        if (isQuoted) {
            paramValue = mla_string_substr(command, paramValueStart, paramValueEnd - paramValueStart);
            paramValueEnd++; // Skip the ending quote
        } else {
            if (paramNameEnd == command.length - 1) {
                paramValue = mla_string_substr(command, paramValueStart, paramValueEnd);
            } else {
                paramValue = mla_string_substr(command, paramValueStart, paramValueEnd - paramValueStart);
            }
        }

        mla_hash_map_push(result.matchingParameters, paramName, paramValue);
        matchedPositon = paramValueEnd;

        // Skip whitespace between parameters
        while (matchedPositon + 1 < command.length && command.data[matchedPositon] == ' ' && command.data[matchedPositon + 1] == ' ') {
            matchedPositon++;
        }
    }

    // Skip ending whitespace
    while (matchedPositon + 1 < command.length && command.data[matchedPositon] == ' ' && command.data[matchedPositon + 1] == ' ') {
        matchedPositon++;
    }

    // All Data Parsed or last character is a space
    if (matchedPositon == command.length || (matchedPositon == command.length - 1 && command.data[matchedPositon] == ' ')) {
        result.isValid = true;
    }

    // Autocomplete possible parameters

    // Check if we are at the beginning of a parameter
    if (matchedPositon + 3 < command.length && command.data[matchedPositon] == ' ' && command.data[matchedPositon + 1]
        == '-' && command.data[matchedPositon + 2] == '-') {

        // We are in the middle of a parameter name
        mla_size_t paramNameStart = matchedPositon + 3;
        mla_size_t paramNameEnd = paramNameStart;
        while (paramNameEnd < command.length && command.data[paramNameEnd] != ' ') {
            paramNameEnd++;
        }
        mla_string_t paramName = mla_string_substr(command, paramNameStart, paramNameEnd - paramNameStart);
        for (mla_size_t i = 0; i < mla_array_list_size(result.matchingCommand.parameters); ++i) {
            mla_cli_command_parameter_t *command_parameter = mla_array_list_get_ref(result.matchingCommand.parameters, i);

            if (mla_hash_map_contains(result.matchingParameters, command_parameter->parameterName)) {
                continue; // Command already used
            }

            if (mla_string_starts_with(command_parameter->parameterName, paramName)) {
                mla_array_list_add(result.possibleAutoCompletions,
                                   mla_string_substr(command_parameter->parameterName, paramName.length));
            }
        }
    } else {
        // List all the Parameters which are not already used

        for (mla_size_t i = 0; i < mla_array_list_size(result.matchingCommand.parameters); ++i) {
            mla_cli_command_parameter_t *command_parameter = mla_array_list_get_ref(result.matchingCommand.parameters, i);

            if (mla_hash_map_contains(result.matchingParameters, command_parameter->parameterName)) {
                continue; // Command already used
            }

            mla_array_list_add(result.possibleAutoCompletions, mla_string_concat(" --", command_parameter->parameterName));
        }
    }

    return result;
}
