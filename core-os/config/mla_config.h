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
    const mla_serialize_definition_t definition;
};

mla_bool_t mla_config_manager_read(const mla_config_definition_t& definition, mla_pointer_t config);
mla_bool_t mla_config_manager_write(const mla_config_definition_t& definition, mla_pointer_t config);


//// Template Helpers
///

template <typename T>
mla_config_definition_t mla_config_definition(mla_string_t name, const mla_serialize_definition_t serialize_definition) {
    return {
            name,
        serialize_definition
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