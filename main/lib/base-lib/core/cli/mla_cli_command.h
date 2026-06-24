//
// Created by christian on 9/13/25.
//

#ifndef MLA_CLI_COMMAND_H
#define MLA_CLI_COMMAND_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_hash_map.h"
#include "../system/mla_stream.h"
#include "../system/mla_user_data.h"

#define mla_cli_command_verbose_parameter_name "verbose"

struct mla_cli_command_parameter_t {
    mla_string_t parameterName;
    mla_string_t description;
    mla_bool_t mandatory;
};

inline mla_cli_command_parameter_t mla_cli_command_parameter(const mla_string_t& p_ParameterName, mla_bool_t p_Mandatory) {
    return {
        p_ParameterName,
        mla_string_empty(),
        p_Mandatory
    };
}

inline mla_cli_command_parameter_t mla_cli_command_parameter(const mla_string_t& p_ParameterName, const mla_string_t& p_Description, mla_bool_t p_Mandatory) {
    return {
        p_ParameterName,
        p_Description,
        p_Mandatory
    };
}

struct mla_cli_command_parameter_initializer {
    static mla_cli_command_parameter_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            false
        };
    }
};

struct mla_cli_command_t;

struct mla_cli_command_execute_outstream_t {
    mla_user_data_t userdata;

    // Regular
    void (*write)(const mla_user_data_t& userdata, const mla_string_t &data);
    void (*writeBuffer)(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length);
    void (*writeCString)(const mla_user_data_t& userdata, const mla_char_t* data);

    // Verbose
    void (*writeVerbose)(const mla_user_data_t& userdata, const mla_string_t &data);
    void (*writeVerboseBuffer)(const mla_user_data_t& userdata, const mla_char_t* data, mla_size_t length);
    void (*writeVerboseCString)(const mla_user_data_t& userdata, const mla_char_t* data);
};

mla_stream_output_t mla_cli_command_execute_outstream_as_stream_output(const mla_cli_command_execute_outstream_t &out);
mla_stream_output_t mla_cli_command_execute_outstream_verbose_as_stream_output(const mla_cli_command_execute_outstream_t &out);

typedef mla_bool_t (*mla_cli_command_execute_t)(const mla_cli_command_t &command,
                                          const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t,
                                              mla_string_initializer, mla_string_initializer> &parameters,
                                          const mla_cli_command_execute_outstream_t &out);

struct mla_cli_command_t {
    mla_string_t name;
    mla_string_t description;
    mla_array_list_t<mla_cli_command_parameter_t, mla_cli_command_parameter_initializer> parameters;
    mla_cli_command_execute_t execute;
    mla_user_data_t user_data;
};

struct mla_cli_command_initializer {
    static mla_cli_command_t init() {
        return {
            mla_string_empty(),
            mla_string_empty(),
            mla_array_list_empty<mla_cli_command_parameter_t, mla_cli_command_parameter_initializer>(),
            nullptr,
            mla_user_data_empty()
        };
    }
};

inline mla_cli_command_t mla_cli_command(const mla_string_t &p_Name, const mla_string_t &p_Description, const mla_cli_command_execute_t &p_Execute) {
    return {
        p_Name,
        p_Description,
        mla_array_list_empty<mla_cli_command_parameter_t,
            mla_cli_command_parameter_initializer>(),
        p_Execute,
        mla_user_data_empty()
    };
}

inline mla_cli_command_t mla_cli_command(const mla_string_t &p_Name, const mla_cli_command_execute_t &p_Execute) {
    return {
        p_Name,
        mla_string_empty(),
        mla_array_list_empty<mla_cli_command_parameter_t,
            mla_cli_command_parameter_initializer>(),
        p_Execute,
        mla_user_data_empty()
    };
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_cli_command_parameter_t &parameter) {
    mla_array_list_add(command.parameters, parameter);
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_string_t &parameterName, mla_bool_t mandatory) {
    mla_cli_command_add_parameter(command, mla_cli_command_parameter(parameterName, mandatory));
}

inline void mla_cli_command_add_parameter(mla_cli_command_t &command, const mla_string_t &parameterName, const mla_string_t &description, mla_bool_t mandatory) {
    mla_cli_command_add_parameter(command, mla_cli_command_parameter(parameterName, description, mandatory));
}

inline void mla_cli_command_add_parameter_verbose_output(mla_cli_command_t &command) {
    mla_cli_command_add_parameter(command, mla_string(mla_cli_command_verbose_parameter_name), mla_string("Enable verbose output"), false);
}

inline mla_bool_t mla_cli_command_parameter_verbose_output_active(const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t,
    mla_string_initializer, mla_string_initializer> &parameters) {

    return mla_hash_map_contains(parameters, mla_string(mla_cli_command_verbose_parameter_name));
}

#endif
