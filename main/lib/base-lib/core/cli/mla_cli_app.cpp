//
// Created by christian on 9/13/25.
//

#include "mla_cli_app.h"
#include "mla_cli_parser.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

mla_user_data_id_init(mla_stream_output_user_data_name)
mla_user_data_id_init(mla_cli_app_user_data_name)
mla_user_data_id_init(mla_cli_submodule_user_data_name)

void mla_private_cli_write_string(mla_stream_output_t &outputStream, const mla_string_t& str) {
    outputStream.write(outputStream, 0, mla_string_length(str), mla_r_cast<const mla_byte_t*>(mla_string_data(str)));
}

void mla_private_cli_command_execute_outstream_to_stream_bridge(const mla_user_data_t& userdata, const mla_string_t &data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    mla_private_cli_write_string(*output, data);
}

void mla_private_cli_command_execute_outstream_verbose_to_stream_bridge(const mla_user_data_t& userdata, const mla_string_t &data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    mla_private_cli_write_string(*output, data);
}

void mla_private_cli_command_execute_outstream_verbose_to_stream_disabled(const mla_user_data_t& userdata, const mla_string_t &data) {
    (void)userdata;
    (void)data;
}

void mla_private_cli_command_execute_outstream_c_string_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, mla_strlen(data), mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data) {
    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, mla_strlen(data), mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_disabled(const mla_user_data_t& userdata, const mla_char_t* data) {
    (void)userdata;
    (void)data;
}

void mla_private_cli_command_execute_outstream_buffer_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {

    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, length, mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_bridge(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {

    mla_stream_output_t *output = mla_user_data_get_pointer_data<mla_stream_output_t>(userdata, mla_stream_output_user_data_name);

    if (output == nullptr) {
        return;
    }

    output->write(*output, 0, length, mla_r_cast<const mla_byte_t*>(data));
}

void mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_disabled(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length) {
    (void)userdata;
    (void)data;
    (void)length;
}

void mla_private_cli_write_module_prompt(mla_cli_app_t &app, mla_stream_output_t &outputStream) {
    mla_size_t size = mla_array_list_size(app.activeModules);

    if (size > 1) {
        // Dont print the root module
        for (mla_size_t i = 1; i < size; ++i) {
            if (i != 1) {
                outputStream.write(outputStream, 0, 1, mla_r_cast<const mla_byte_t*>(">"));
            }

            mla_string_t currentModuleName = mla_array_list_get_ref(app.activeModules, i)->moduleName;
            mla_private_cli_write_string(outputStream, currentModuleName);
        }
    }

    outputStream.write(outputStream, 0, 1, mla_r_cast<const mla_byte_t*>(">"));
}

void mla_private_cli_activate_module(mla_cli_app_t &app, mla_cli_module_t &module) {
    mla_array_list_add(app.activeModules, module);
}

mla_bool_t mla_private_cli_cmd_exit_execute(const mla_cli_command_t &command,
                                const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer,
                                    mla_string_initializer> &parameters,
                                const mla_cli_command_execute_outstream_t &out) {
    (void) parameters;
    mla_cli_app_t *app = mla_user_data_get_pointer_data<mla_cli_app_t>(command.user_data, mla_cli_app_user_data_name);

    if (app == nullptr) {
        return false;
    }

    mla_size_t size = mla_array_list_size(app->activeModules);

    if (size > 1) {
        out.write(out.userdata, mla_string_concat("Close module '",  mla_array_list_get_ref(app->activeModules, size -1)->moduleName, "'\n"));
        mla_array_list_remove(app->activeModules, size - 1);

    }

    return true;
}

mla_string_t mla_private_cli_format_command(const mla_cli_command_t &command) {

    // Start with command usage
    mla_string_t result = mla_string_concat("  ", command.name);

    // Add parameters to usage line
    for (mla_size_t i = 0; i < mla_array_list_size(command.parameters); ++i) {
        mla_cli_command_parameter_t *param = mla_array_list_get_ref(command.parameters, i);
        if (param->mandatory) {
            result = mla_string_concat(result, " --", param->parameterName, " <value>");
        } else {
            result = mla_string_concat(result, " [--", param->parameterName, " <value>]");
        }
    }

    // Add command description if available
    if (mla_string_length(command.description) > 0) {
        result = mla_string_concat(result, "\n    ", command.description);
    }

    // Add parameter descriptions
    for (mla_size_t i = 0; i < mla_array_list_size(command.parameters); ++i) {
        mla_cli_command_parameter_t *param = mla_array_list_get_ref(command.parameters, i);

        if (mla_string_length(param->description) > 0) {
            mla_string_t paramDesc = mla_string_concat("\n      --", param->parameterName, ": ",
                                                      param->description);
            if (param->mandatory) {
                paramDesc = mla_string_concat(paramDesc, " (required)");
            } else {
                paramDesc = mla_string_concat(paramDesc, " (optional)");
            }

            result = mla_string_concat(result, paramDesc);
        }
    }

    return result;
}

mla_bool_t mla_private_cli_cmd_help_execute(const mla_cli_command_t &command,
                                const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer,
                                    mla_string_initializer> &parameters,
                                const mla_cli_command_execute_outstream_t &out) {
    (void) parameters;
    mla_cli_app_t *app = mla_user_data_get_pointer_data<mla_cli_app_t>(command.user_data, mla_cli_app_user_data_name);

    if (app == nullptr) {
        return false;
    }

    mla_size_t size = mla_array_list_size(app->activeModules);

    if (size == 0) {
        return true;
    }

    const mla_cli_module_t *activeModule = mla_array_list_get_ref(app->activeModules, size - 1);

    if (activeModule == nullptr) {
        return true;
    }

    // List all commands
    out.write(out.userdata, mla_string_concat("Help for module '", activeModule->moduleName, "'\n\n"));

    mla_size_t commandCount = mla_array_list_size(activeModule->availableCommands);

    if (commandCount > 0) {
        out.write(out.userdata, mla_string_const("Available commands:\n"));

        for (mla_size_t i = 0; i < commandCount; ++i) {
            mla_cli_command_t *commandOfModule = mla_array_list_get_ref(activeModule->availableCommands, i);

            // Remove the help command from the list
            if (mla_string_equals(command.name, commandOfModule->name)) {
                continue;
            }

            mla_string_t commandFormated = mla_private_cli_format_command(*commandOfModule);
            out.write(out.userdata, mla_string_concat(commandFormated, "\n"));
        }
    } else {
        out.write(out.userdata, mla_string_const("No commands available in this module.\n"));
    }

    mla_size_t subModuleCount = mla_array_list_size(activeModule->subModules);

    if (subModuleCount > 0) {
        out.write(out.userdata, mla_string_const("\nAvailable Modules:\n"));

        for (mla_size_t i = 0; i < subModuleCount; ++i) {
            mla_cli_module_t *subModule = mla_array_list_get_ref(activeModule->subModules, i);
            out.write(out.userdata, mla_string_concat("  ", subModule->moduleName, "\n"));

            if (mla_string_length(subModule->description) > 0) {
                out.write(out.userdata, mla_string_concat("    ", subModule->description, "\n"));
            }
        }
    }

    return true;
}

mla_bool_t mla_private_cli_cmd_open_sub_module_execute(const mla_cli_command_t &command,
                                           const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer,
                                               mla_string_initializer> &parameters,
                                           const mla_cli_command_execute_outstream_t &out) {
    (void) parameters;

    mla_cli_app_t *app = mla_user_data_get_pointer_data<mla_cli_app_t>(command.user_data, mla_cli_app_user_data_name);

    if (app == nullptr) {
        return false;
    }

    mla_cli_module_t *subModule = mla_user_data_get_pointer_data<mla_cli_module_t>(command.user_data, mla_cli_submodule_user_data_name);

    if (subModule == nullptr) {
        return false;
    }

    out.write(out.userdata, mla_string_concat("Open module '", subModule->moduleName, "'\n"));
    mla_private_cli_activate_module(*app, *subModule);
    return true;
}

mla_cli_parser_t mla_private_cli_setup_parser(mla_cli_app_t &app) {
    mla_cli_parser_t parser = mla_cli_parser();

    mla_size_t moduleCount = mla_array_list_size(app.activeModules);

    mla_pointer_t app_ptr = mla_platform_pointer_to_managed_pointer(&app);

    // Add all commands from the last active module
    if (moduleCount > 0) {
        mla_cli_module_t *currentModule = mla_array_list_get_ref(app.activeModules, moduleCount - 1);

        for (mla_size_t i = 0; i < mla_array_list_size(currentModule->availableCommands); ++i) {
            mla_cli_command_t *cmd = mla_array_list_get_ref(currentModule->availableCommands, i);
            mla_array_list_add(parser.availableCommands, *cmd);
        }

        // Add all commands to enter sub modules
        for (mla_size_t i = 0; i < mla_array_list_size(currentModule->subModules); ++i) {
            mla_cli_module_t *subModule = mla_array_list_get_ref(currentModule->subModules, i);
            mla_cli_command_t cmdEnterModule = mla_cli_command(subModule->moduleName, mla_private_cli_cmd_open_sub_module_execute);
            mla_pointer_t subModule_ptr = mla_platform_pointer_to_managed_pointer(subModule);

            mla_user_data_set_pointer(cmdEnterModule.user_data, mla_cli_app_user_data_name, app_ptr);
            mla_user_data_set_pointer(cmdEnterModule.user_data, mla_cli_submodule_user_data_name, subModule_ptr);

            mla_array_list_add(parser.availableCommands, cmdEnterModule);
        }
    }


    // Back Command if we are not in the root module
    if (moduleCount > 1) {
        mla_cli_command_t cmdExit = mla_cli_command(mla_string_const("exit"), mla_private_cli_cmd_exit_execute);

        mla_user_data_set_pointer(cmdExit.user_data, mla_cli_app_user_data_name, app_ptr);
        mla_array_list_add(parser.availableCommands, cmdExit);
    }

    // Help command
    mla_cli_command_t cmdHelp = mla_cli_command(mla_string_const("help"), mla_private_cli_cmd_help_execute);
    mla_user_data_set_pointer(cmdHelp.user_data, mla_cli_app_user_data_name, app_ptr);
    mla_array_list_add(parser.availableCommands, cmdHelp);

    return parser;
}

mla_bool_t mla_private_cli_process_parser_result(const mla_string_t& inputCommand, const mla_cli_parser_result &parser_result, mla_stream_output_t &outputStream) {

    if (parser_result.isValid && mla_string_length(parser_result.matchingCommand.name) != 0) {
        // Validate mandatory parameters
        mla_bool_t missingMandatoryParameter = false;

        for (mla_size_t i = 0; i < mla_array_list_size(parser_result.matchingCommand.parameters); ++i) {
            mla_cli_command_parameter_t *param = mla_array_list_get_ref(parser_result.matchingCommand.parameters, i);

            if (param->mandatory && !mla_hash_map_contains(parser_result.matchingParameters, param->parameterName)) {
                mla_private_cli_write_string(outputStream,
                                       mla_string_concat(mla_string("Parameter '"), param->parameterName,
                                                         mla_string("' is mandatory but not provided\n")));
                missingMandatoryParameter = true;
            }
        }

        if (missingMandatoryParameter) {
            // not all mandatory parameters are provided
            return false;
        }

        // Execute the command
        if (parser_result.matchingCommand.execute != nullptr) {

            mla_user_data_t user_data = mla_user_data_empty();
            mla_pointer_t outputStream_ptr = mla_platform_pointer_to_managed_pointer(&outputStream);
            mla_user_data_set_pointer(user_data, mla_stream_output_user_data_name, outputStream_ptr);

            mla_cli_command_execute_outstream_t stringOutstream = {
                user_data,
                mla_private_cli_command_execute_outstream_to_stream_bridge,
                mla_private_cli_command_execute_outstream_buffer_to_stream_bridge,
                mla_private_cli_command_execute_outstream_c_string_to_stream_bridge,

                mla_private_cli_command_execute_outstream_verbose_to_stream_disabled,
                mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_disabled,
                mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_disabled
            };

            if (mla_cli_command_parameter_verbose_output_active(parser_result.matchingCommand, parser_result.matchingParameters)) {
                stringOutstream.writeVerbose = mla_private_cli_command_execute_outstream_verbose_to_stream_bridge;
                stringOutstream.writeVerboseBuffer = mla_private_cli_command_execute_outstream_verbose_buffer_to_stream_bridge;
                stringOutstream.writeVerboseCString = mla_private_cli_command_execute_outstream_verbose_c_string_to_stream_bridge;
            }

            return parser_result.matchingCommand.execute(parser_result.matchingCommand, parser_result.matchingParameters,
                                                  stringOutstream);
        } else {
            mla_error(
                mla_string_concat(mla_string("Command '"), parser_result.matchingCommand.name, mla_string(
                    "' has no execute function")));
            return false;
        }

    } else {
        mla_private_cli_write_string(outputStream, mla_string("Unknown Command :\n"));
        mla_private_cli_write_string(outputStream, mla_string("  "));
        mla_private_cli_write_string(outputStream, inputCommand);
        outputStream.write(outputStream, 0, 1,  mla_r_cast<const mla_byte_t*>("\n"));

        // There is something missing show the possible auto completions
        if (mla_array_list_size(parser_result.possibleAutoCompletions) > 0) {
            outputStream.write(outputStream, 0, 1,  mla_r_cast<const mla_byte_t*>("\n"));
            mla_private_cli_write_string(outputStream, mla_string("Do you mean:\n"));
            for (mla_size_t i = 0; i < mla_array_list_size(parser_result.possibleAutoCompletions); ++i) {
                mla_string_t *completion = mla_array_list_get_ref(parser_result.possibleAutoCompletions, i);
                mla_private_cli_write_string(outputStream, mla_string("  "));
                mla_private_cli_write_string(outputStream, inputCommand);
                mla_private_cli_write_string(outputStream, *completion);
                outputStream.write(outputStream, 0, 1,  mla_r_cast<const mla_byte_t*>("\n"));
            }
        } else {

            // No possible completions found
            mla_private_cli_write_string(outputStream, mla_string("Type 'help' to see available commands.\n"));

        }

        return false;
    }
}

mla_bool_t mla_private_cli_parser_parse_and_execute_command(mla_cli_app_t &app, const mla_string_t &command,
                                                mla_stream_output_t &outputStream) {
    // Setup the parser
    mla_cli_parser_t parser = mla_private_cli_setup_parser(app);

    // Parse the command
    const mla_cli_parser_result parser_result = mla_cli_parser_parse(parser, command);

    // Process the result
    return mla_private_cli_process_parser_result(command, parser_result, outputStream);
}

mla_cli_app_t mla_cli_app_empty() {
    return {
        mla_array_list_empty<mla_cli_module_t, mla_cli_module_initializer>(),
        mla_string_empty()
    };
}

mla_cli_app_t mla_cli_app_init(mla_cli_module_t &rootModule, mla_stream_output_t &outputStream) {
    mla_cli_app_t app = {
        mla_array_list<mla_cli_module_t, mla_cli_module_initializer>(2),
        mla_string_empty()
    };

    mla_private_cli_activate_module(app, rootModule);
    mla_private_cli_write_module_prompt(app, outputStream);
    return app;
}

mla_bool_t mla_cli_app_update_and_process_input(mla_cli_app_t &app, mla_stream_input_t &inputStream,
                                          mla_stream_output_t &outputStream) {

    // Create an own scopt so that the buffer is removed from the stack after reading
    {
        mla_byte_t buffer[mla_global_config_stream_fast_read_buffer_size] = {0};
        mla_size_t bytesRead = inputStream.read(inputStream, 0, sizeof(buffer), buffer);

        if (bytesRead == 0) {
            return false;
        }

        // Append to unprocessed input
        mla_pointer_t buffer_ptr = mla_platform_pointer_to_managed_pointer(buffer);
        mla_string_t newInput = mla_string(buffer_ptr, bytesRead);
        app.unprocessedInput = mla_string_concat(app.unprocessedInput, newInput);
    }

    mla_int32_t lineEnd = mla_string_index_of(app.unprocessedInput, mla_string("\n"));

    mla_bool_t commandProcessed = false;
    mla_bool_t commandSuccessfullyProcessed = true;

    while (lineEnd != -1) {
        // Process the line
        mla_string_t line = mla_string_substr(app.unprocessedInput, 0, lineEnd); // Exclude the newline character

        // Remove the processed line from unprocessed input
        mla_string_t remainingInput = mla_string_substr(app.unprocessedInput, lineEnd + 1);
        app.unprocessedInput = remainingInput;

        // Parse and execute the command
        if (!mla_private_cli_parser_parse_and_execute_command(app, line, outputStream)) {
            commandSuccessfullyProcessed = false;
        }
        commandProcessed = true;

        // Check for another complete line
        lineEnd = mla_string_index_of(app.unprocessedInput, mla_string("\n"));
    }

    if (commandProcessed) {
        outputStream.write(outputStream, 0, 1, mla_r_cast<const mla_byte_t*>("\n"));
        mla_private_cli_write_module_prompt(app, outputStream);
    }

    return commandSuccessfullyProcessed;
}


mla_cli_module_t mla_cli_module(const mla_string_t& name) {
    return {
        name,
        mla_string_empty(),
        mla_array_list_empty<mla_cli_command_t, mla_cli_command_initializer>(),
        mla_array_list_empty<mla_cli_module_t, mla_cli_module_initializer>()
    };
}

mla_cli_module_t mla_cli_module(const mla_string_t& name, const mla_string_t& description) {
    return {
        name,
        description,
        mla_array_list_empty<mla_cli_command_t, mla_cli_command_initializer>(),
        mla_array_list_empty<mla_cli_module_t, mla_cli_module_initializer>()
    };
}

void mla_cli_module_add_command(mla_cli_module_t& module, const mla_cli_command_t& command) {
    mla_array_list_add(module.availableCommands, command);
}

void mla_cli_module_add_sub_module(mla_cli_module_t& module, const mla_cli_module_t& subModule) {
    mla_array_list_add(module.subModules, subModule);
}

const mla_cli_command_t* mla_cli_module_find_command(const mla_cli_module_t& module, const mla_string_t& commandName) {

    for (mla_size_t i = 0; i < mla_array_list_size(module.availableCommands); ++i) {

        const mla_cli_command_t* command = mla_array_list_get_ref(module.availableCommands, i);

        if (mla_string_equals(command->name, commandName)) {
            return mla_array_list_get_ref(module.availableCommands, i);
        }

    }

    return nullptr;

}
