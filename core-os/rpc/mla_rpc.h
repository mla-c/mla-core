//
// Created by christian on 11/17/25.
//

#ifndef COREOS_MLA_RPC_H
#define COREOS_MLA_RPC_H

#include "../system/mla_string.h"
#include "../serializer/mla_serializer.h"
#include "../system/mla_hash_map.h"

typedef mla_bool_t (*mla_rpc_procedure_handler_unsafe_t)(const mla_pointer_t input_data, mla_pointer_t output_data);

struct mla_rpc_procedure_unsafe_t {
    mla_string_t procedureName;
    mla_serialize_definition_t inputDefinition;
    mla_serialize_definition_t outputDefinition;
    mla_rpc_procedure_handler_unsafe_t execute;
};

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe_invalid();

struct mla_rpc_procedure_unsafe_initializer {
    static mla_rpc_procedure_unsafe_t init() {
        return mla_rpc_procedure_unsafe_invalid();
    }
};

mla_rpc_procedure_unsafe_t mla_rpc_procedure_unsafe(
        const mla_string_t &procedure_name,
        const mla_serialize_definition_t &input_definition,
        const mla_serialize_definition_t &output_definition,
        const mla_rpc_procedure_handler_unsafe_t &execute_handler
);
mla_bool_t mla_rpc_register_procedure(const mla_rpc_procedure_unsafe_t &procedure);
mla_bool_t mla_rpc_unregister_procedure(const mla_string_t &procedure_name);
mla_bool_t mla_rpc_find_procedure(const mla_string_t &procedure_name, mla_rpc_procedure_unsafe_t& out_procedure);
mla_bool_t mla_rpc_is_local_procedure(const mla_string_t &procedure_name);
mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const mla_pointer_t input_data, mla_pointer_t output_data);
mla_array_list_t<mla_rpc_procedure_unsafe_t, mla_rpc_procedure_unsafe_initializer> mla_rpc_list_procedures();


////////////////////////////////////////////////////////////
///// Safe Helpers
/////////////////////////////////////////////////////////////

#define mla_rpc_safe_template_parameters typename TInput, typename TOutput

template<mla_rpc_safe_template_parameters>
struct mla_rpc_procedure_safe_t {
    mla_string_t procedureName;
    mla_serialize_definition_t inputDefinition;
    mla_serialize_definition_t outputDefinition;
    mla_bool_t (*execute)(const TInput *input_data, TOutput *output_data);
};

template<mla_rpc_safe_template_parameters>
mla_rpc_procedure_safe_t<TInput, TOutput> mla_rpc_procedure_safe(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition,
        mla_bool_t (*execute_handler)(const TInput *input_data, TOutput *output_data)
) {
    return {
        procedure_name,
        input_definition,
        output_definition,
        execute_handler
    };
}

template<mla_rpc_safe_template_parameters>
mla_rpc_procedure_unsafe_t mla_rpc_procedure_safe_to_unsafe(const mla_rpc_procedure_safe_t<TInput, TOutput> &safe_procedure) {
    return {
        safe_procedure.procedureName,
        safe_procedure.inputDefinition,
        safe_procedure.outputDefinition,
        static_cast<mla_rpc_procedure_handler_unsafe_t>(safe_procedure.execute)
    };
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_register_procedure(const mla_rpc_procedure_safe_t<TInput, TOutput> &procedure) {
    mla_rpc_procedure_unsafe_t unsafe_procedure = mla_rpc_procedure_safe_to_unsafe(procedure);
    return mla_rpc_register_procedure(unsafe_procedure);
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const TInput *input_data, TOutput *output_data) {
    return mla_rpc_execute_procedure(procedure_name, static_cast<const mla_pointer_t>(input_data), static_cast<mla_pointer_t>(output_data));
}

template<mla_rpc_safe_template_parameters>
mla_bool_t mla_rpc_find_procedure(const mla_string_t &procedure_name, mla_rpc_procedure_safe_t<TInput, TOutput>& out_procedure) {

    mla_rpc_procedure_unsafe_t unsafe_procedure = mla_rpc_procedure_unsafe_invalid();

    if (mla_rpc_find_procedure(procedure_name, unsafe_procedure)) {
        out_procedure->procedureName = unsafe_procedure.procedureName;
        out_procedure->execute = static_cast<mla_bool_t (*)(const TInput *, TOutput *)>(unsafe_procedure.execute);
        return true;
    } else {
        return false;
    }
}



#endif
