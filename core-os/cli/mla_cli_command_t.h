//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_CLI_COMMAND_H
#define COREOS_MLA_CLI_COMMAND_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_hash_map.h"

struct mla_cli_command_parameter_t {
    mla_string_t parameterName;
    mla_bool_t mandatory;
};

inline mla_cli_command_parameter_t mla_cli_command_parameter(const mla_char_t* p_ParameterName, mla_bool_t p_Mandatory) {
    return {
        mla_string(p_ParameterName),
        p_Mandatory
    };
}

struct mla_cli_command_parameter_initializer {

    static mla_cli_command_parameter_t init() {
        return  {
            mla_string_empty(),
            false
        };
    }
};

struct mla_cli_command_t;
struct mla_cli_command_execute_outstream_t {
    mla_callback_userdata userdata;
    void (*write)(mla_callback_userdata userdata, const mla_string_t& data);
};
typedef void (*mla_cli_command_execute_t)(const mla_cli_command_t& command, const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>& parameters, const mla_cli_command_execute_outstream_t& out);

struct mla_cli_command_t {
    mla_string_t name;
    mla_array_list_t<mla_cli_command_parameter_t, mla_cli_command_parameter_initializer> parameters;
    mla_cli_command_execute_t execute;
    mla_callback_userdata userdata;
    mla_callback_userdata userdata2;
};

struct mla_cli_command_initializer {

    static mla_cli_command_t init() {
        return  {
            mla_string_empty(),
            mla_array_list_empty<mla_cli_command_parameter_t, mla_cli_command_parameter_initializer>(),
            nullptr,
          0,
            0
        };
    }
};


// Recursive adder (base)
inline void mla_cli_command_add_all(mla_cli_command_t&) {}

// Recursive adder (step)
template<typename... Rest>
void mla_cli_command_add_all(mla_cli_command_t& cmd,
                                    const mla_cli_command_parameter_t& first,
                                    const Rest&... rest) {
    mla_array_list_add(cmd.parameters, first);
    mla_cli_command_add_all(cmd, rest...);
}


template<typename... Params>
mla_cli_command_t mla_cli_command(const mla_char_t* p_Name,
                                         const Params&... params) {

    mla_cli_command_t cmd = {
        mla_string(p_Name),
        mla_array_list_empty<mla_cli_command_parameter_t,
                             mla_cli_command_parameter_initializer>(),
        nullptr,
        0,
        0
    };
    mla_cli_command_add_all(cmd, params...);
    return cmd;
}

template<typename... Params>
mla_cli_command_t mla_cli_command(mla_string_t p_Name,
                                         const Params&... params) {

    mla_cli_command_t cmd = {
        p_Name,
        mla_array_list_empty<mla_cli_command_parameter_t,
                             mla_cli_command_parameter_initializer>(),
        nullptr,
        0,
        0
    };
    mla_cli_command_add_all(cmd, params...);
    return cmd;
}

#endif
