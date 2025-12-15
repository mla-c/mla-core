//
// Created by chris on 12/15/2025.
//

#include "mla_reflection.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../system/mla_hash_map.h"

struct mla_reflection_container_t {
    mla_hash_map_t<mla_string_t, mla_reflection_struct_metadata_provider_t, mla_string_hash_t, mla_string_initializer> structs;
};

mla_reflection_container_t g_reflection_container = {
    mla_hash_map<mla_string_t, mla_reflection_struct_metadata_provider_t, mla_string_hash_t, mla_string_initializer>()
};

mla_reflection_struct_metadata_t mla_reflection_struct_metadata_invalid() {
    return {mla_string_empty(), 0, true, mla_array_list_empty<mla_reflection_struct_field_t, mla_reflection_struct_field_initializer>()};
}

mla_reflection_struct_metadata_t mla_reflection_struct_metadata(const mla_string_t &name, mla_size_t size) {
    return {name, size, false, mla_array_list_empty<mla_reflection_struct_field_t, mla_reflection_struct_field_initializer>()};
}

void mla_reflection_struct_metadata_add_field(mla_reflection_struct_metadata_t& metadata, const mla_reflection_struct_field_t &field) {

    if (metadata.frozen) {
        mla_error(mla_string_concat("Attempted to add field '", field.name, "' to frozen struct metadata '", metadata.name, "'"));
        return;
    }

    mla_array_list_add(metadata.fields, field);
}

void mla_reflection_struct_metadata_freeze(mla_reflection_struct_metadata_t& metadata) {
    metadata.frozen = true;
}

mla_reflection_struct_field_t mla_reflection_struct_field(const mla_string_t &name, mla_size_t offset, mla_reflection_type_t type, mla_reflection_type_t element_type, const mla_string_t &struct_name) {
    return {name, offset, type, element_type, struct_name};
}

mla_bool_t mla_reflection_register_struct(const mla_string_t &name, mla_reflection_struct_metadata_provider_t provider) {

    if (mla_hash_map_contains(g_reflection_container.structs, name)) {
        mla_warning(mla_string_concat("Struct '", name, "' is already registered in reflection system"));
        return false;
    }

    mla_hash_map_push(g_reflection_container.structs, name, provider);
    return true;
}
