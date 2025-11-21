//
// Created by christian on 11/17/25.
//

#include "mla_rpc.h"

#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"

struct  mla_rpc_container_t {
    mla_hash_map_t<mla_string_t, mla_rpc_procedure_unsafe_t, mla_string_hash_t, mla_string_initializer, mla_rpc_procedure_unsafe_initializer> procedures;
    mla_bool_t isLocked;
};

mla_rpc_container_t g_rpc_container = {
    mla_hash_map<mla_string_t, mla_rpc_procedure_unsafe_t, mla_string_hash_t, mla_string_initializer, mla_rpc_procedure_unsafe_initializer>(),
    false
};

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe_invalid() {
    return {
        mla_string_empty(),
        mla_serialize_definition_invalid(),
        mla_serialize_definition_invalid(),
        nullptr
    };
}

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe(
        const mla_string_t &procedure_name,
        const mla_serialize_definition_t &input_definition,
        const mla_serialize_definition_t &output_definition,
        const mla_rpc_procedure_handler_unsafe_t &execute_handler) {
    return {
        procedure_name,
        input_definition,
        output_definition,
        execute_handler
    };
}

mla_bool_t mla_rpc_register_procedure(const mla_rpc_procedure_unsafe_t &procedure) {

    if (g_rpc_container.isLocked) {
        mla_warning(mla_string_concat("Attempted to register RPC procedure ", procedure.procedureName, " while RPC container is locked."));
        return false; // RPC Container is locked
    }

    return mla_hash_map_push(g_rpc_container.procedures, procedure.procedureName, procedure) == MLA_HASH_MAP_PUSH_ADDED;
}

mla_bool_t mla_rpc_unregister_procedure(const mla_string_t &procedure_name) {

    if (g_rpc_container.isLocked) {
        mla_warning(mla_string_concat("Attempted to unregister RPC procedure ", procedure_name, " while RPC container is locked."));
        return false; // RPC Container is locked
    }

    return mla_hash_map_remove(g_rpc_container.procedures, procedure_name);
}

mla_bool_t mla_rpc_find_procedure(const mla_string_t &procedure_name, mla_rpc_procedure_unsafe_t& out_procedure) {

    return mla_hash_map_get(g_rpc_container.procedures, procedure_name, out_procedure);
}

mla_bool_t mla_rpc_is_local_procedure(const mla_string_t &procedure_name) {

    return mla_hash_map_contains(g_rpc_container.procedures, procedure_name);
}

mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const mla_pointer_t input_data, mla_pointer_t output_data) {

    mla_rpc_procedure_unsafe_t procedure = mla_rpc_procedure_unsafe_invalid();

    if (!mla_rpc_find_procedure(procedure_name, procedure)) {
        return false; // Procedure not found
    }

    if (procedure.execute == nullptr) {
        return false; // Procedure has no handler
    }

    return procedure.execute(input_data, output_data);
}

mla_array_list_t<mla_rpc_procedure_unsafe_t, mla_rpc_procedure_unsafe_initializer> mla_rpc_list_procedures() {
    return mla_hash_map_values(g_rpc_container.procedures);
}

void __mla_rpc_container_lock() {
    g_rpc_container.isLocked = true;
}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_setup, __mla_rpc_container_lock);