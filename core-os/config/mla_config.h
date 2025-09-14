//
// Created by chris on 9/14/2025.
//

#ifndef COREOS_MLA_CONFIG_H
#define COREOS_MLA_CONFIG_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../serializer/mla_serializer.h"


typedef mla_bool_t (*mla_config_definition_read_function_t)(const mla_deserializer_t& deserializer, mla_pointer_t config);
typedef mla_bool_t (*mla_config_definition_write_function_t)(const mla_serializer_t& serializer, mla_pointer_t config);

struct mla_config_definition_t {
    const mla_string_t config_name;
    const mla_size_t config_size;
    const mla_config_definition_read_function_t read_function;
    const mla_config_definition_write_function_t write_function;
};

mla_bool_t mla_config_manager_read(const mla_config_definition_t& definition, mla_pointer_t config);
mla_bool_t mla_config_manager_write(const mla_config_definition_t& definition, mla_pointer_t config);


//// Template Helpers
///

template <typename T>
mla_config_definition_t mla_config_definition(const mla_config_definition_read_function_t& read_function, const mla_config_definition_write_function_t& write_function) {
    return {
            mla_string_const(typeid(T).name()),
            sizeof(T),
            read_function,
            write_function
    };
}

template <typename T>
mla_config_definition_t mla_config_definition_with_dynamic_size(const mla_config_definition_read_function_t& read_function, const mla_config_definition_write_function_t& write_function) {
    return {
        mla_string_const(typeid(T).name()),
        0,
        read_function,
        write_function
    };
}

template <typename T>
mla_bool_t mla_config_manager_read(const mla_config_definition_t& definition, T& config) {
    return mla_config_manager_read(definition, &config);
}

template <typename T>
mla_bool_t mla_config_manager_write(const mla_config_definition_t& definition, T& config) {
    return mla_config_manager_write(definition, &config);
}



#endif