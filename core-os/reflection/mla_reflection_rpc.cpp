//
// Created by chris on 12/18/2025.
//

#include "mla_reflection_rpc.h"
#include "mla_reflection.h"
#include "../rpc/mla_rpc.h"

mla_reflection_rpc_struct_metadata_t mla_reflection_rpc_struct_metadata_invalid() {
    return { mla_string_empty(), mla_array_list_empty<mla_reflection_rpc_struct_metadata_field_t, mla_reflection_rpc_struct_metadata_field_initializer>() };
}

mla_bool_t mla_reflection_metadata_rpc_get_metadata(const mla_reflection_struct_metadata_request_t* request, mla_reflection_struct_metadata_response_t* response) {

    if (request == nullptr) {
        return false;
    }

    response->meta_data = mla_reflection_rpc_struct_metadata_invalid();

    if (mla_string_is_empty(request->struct_name)) {
        response->found = false;
        return true;
    }

    mla_reflection_struct_metadata_t structMeta = mla_reflection_struct_metadata_invalid();

    if (!mla_reflection_get_struct_metadata(request->struct_name, structMeta)) {
        response->found = false;
        return true;
    }

    response->found = true;
    mla_reflection_rpc_struct_metadata_t* responseMeta = &response->meta_data;
    responseMeta->name = structMeta.name;
    mla_array_list_resize(responseMeta->fields, mla_array_list_size(structMeta.fields));

    for (mla_size_t i = 0; i < mla_array_list_size(structMeta.fields); ++i) {

        const mla_reflection_struct_field_t& field = mla_array_list_get_unsafe(structMeta.fields, i);
        mla_reflection_rpc_struct_metadata_field_t responseField = {
            field.name,
            field.type,
            field.element_type,
            field.struct_name
        };
        mla_array_list_add(responseMeta->fields, responseField);
    }

    return true;
}

mla_rpc_auto_register_procedure(mla_rpc_procedure_get_meta_data_name, mla_reflection_struct_metadata_request_t, mla_reflection_struct_metadata_response_t, mla_reflection_metadata_rpc_get_metadata)