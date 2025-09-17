#include "mla_config.h"


mla_deserializer_t __mla_get_deserializer_for_context(const mla_string_t& context) {
    (void)context;
    return {
        mla_stream_noop_input(),
        0,
        {MLA_DESERIALIZER_NULL, {mla_string_empty(), mla_string_empty(), mla_bytes_empty()}, {0}}

    };
}

mla_serializer_t __mla_get_serializer_for_context(const mla_string_t& context) {
    (void)context;
    return {
        mla_stream_noop_output(),
        0,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
        };
}

mla_bool_t mla_config_manager_read(const mla_config_definition_t& definition, mla_pointer_t config) {

    if (definition.definition.read_function == nullptr) {
        return false;
    }

    mla_deserializer_t deserializer = __mla_get_deserializer_for_context(definition.config_name);
    return mla_deserializer_read_struct(deserializer, config, definition.definition.read_function);

}

mla_bool_t mla_config_manager_write(const mla_config_definition_t& definition, mla_pointer_t config) {

    if (definition.definition.write_function == nullptr) {
        return false;
    }

    mla_serializer_t serializer = __mla_get_serializer_for_context(definition.config_name);
    mla_serializer_write_struct(serializer, config, definition.definition.write_function);
    return true;
}