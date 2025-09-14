#include "mla_config.h"


mla_deserializer_t __mla_get_deserializer_for_context(const mla_string_t& context) {

}

mla_serializer_t __mla_get_serializer_for_context(const mla_string_t& context) {

}

mla_bool_t mla_config_manager_read(const mla_config_definition_t& definition, mla_pointer_t config) {

    if (definition.read_function == nullptr) {
        return false;
    }

    mla_deserializer_t deserializer = __mla_get_deserializer_for_context(definition.config_name);
    return definition.read_function(deserializer, config);

}

mla_bool_t mla_config_manager_write(const mla_config_definition_t& definition, mla_pointer_t config) {

    if (definition.write_function == nullptr) {
        return false;
    }

    mla_serializer_t serializer = __mla_get_serializer_for_context(definition.config_name);

    return definition.write_function(serializer, config);
}