//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_CLI_APP_H
#define COREOS_MLA_CLI_APP_H

#include "mla_cli_command_t.h"
#include "../system/mla_stream.h"

struct mla_cli_module_initializer;

struct mla_cli_module_t {
    mla_string_t moduleName;
    mla_array_list_t<mla_cli_command_t, mla_cli_command_initializer> availableCommands;
    mla_array_list_t<mla_cli_module_t, mla_cli_module_initializer> subModules;

};

struct mla_cli_module_initializer {
    static mla_cli_module_t init() {
        return {
            mla_string_empty(),
            mla_array_list_empty<mla_cli_command_t, mla_cli_command_initializer>(),
            mla_array_list_empty<mla_cli_module_t, mla_cli_module_initializer>()
        };
    }
};

inline mla_cli_module_t mla_cli_module(const mla_string_t& name) {
    return {
        name,
        mla_array_list_empty<mla_cli_command_t, mla_cli_command_initializer>(),
        mla_array_list_empty<mla_cli_module_t, mla_cli_module_initializer>()
    };
}

inline mla_cli_module_t mla_cli_module(const mla_char_t* name) {
    return {
        mla_string(name),
        mla_array_list_empty<mla_cli_command_t, mla_cli_command_initializer>(),
        mla_array_list_empty<mla_cli_module_t, mla_cli_module_initializer>()
    };
}

struct mla_cli_app_t {
    mla_array_list_t<mla_cli_module_t, mla_cli_module_initializer> activeModules;
    mla_string_t unprocessedInput;
};

mla_cli_app_t mla_cli_app_init(mla_cli_module_t& rootModule, const mla_stream_output_t& outputStream);
void mla_cli_app_update_and_process_input(mla_cli_app_t& app, const mla_stream_input_t& inputStream, const mla_stream_output_t& outputStream);



#endif
