//
// Created by christian on 11/17/25.
//

#include "mla_rpc.h"

#include "../system/mla_id.h"
#include "../task/mla_rw_lock.h"
#include "../log/mla_logging.h"
#include "../system/mla_string_concat.h"
#include "../serializer/mla_serializer.h"
#include "../serializer/mla_binary_serializer.h"

struct  mla_rpc_container_t {
    mla_array_list_t<mla_rpc_remote_endpoint_t, mla_rpc_remote_endpoint_initializer> remote_endpoints;
    mla_rw_lock_t remote_endpoints_lock;
    mla_hash_map_t<mla_string_t, mla_rpc_procedure_unsafe_t, mla_string_hash_t, mla_string_initializer, mla_rpc_procedure_unsafe_initializer> procedures;
    mla_bool_t isLocked;
};

mla_rpc_container_t g_rpc_container = {
    mla_array_list<mla_rpc_remote_endpoint_t, mla_rpc_remote_endpoint_initializer>(),
    mla_rw_lock_create("rpc_endpoints"),
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

    if (mla_string_starts_with(procedure.procedureName, mla_string_const("/"))) {
        mla_warning(mla_string_concat("Attempted to register RPC procedure ", procedure.procedureName, " with reserved '/' prefix."));
        return false; // Reserved prefix
    }

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

mla_bool_t mla_rpc_execute_procedure(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t input_data, mla_pointer_t output_data) {

    mla_rpc_procedure_unsafe_t procedure = mla_rpc_procedure_unsafe_invalid();

    if (!mla_rpc_find_procedure(procedure_name, procedure)) {
        // Try to execute the procedure remotely
        return mla_rpc_execute_procedure_remote(procedure_name, input_definition, output_definition, input_data, output_data);
    }

    if (procedure.execute == nullptr) {
        return false; // Procedure has no handler
    }

    // Serialize also local calls to have the same behavior as remote calls
    mla_pointer_t serialized_input = nullptr;

    // Serialize input data
    if (input_data != nullptr && input_definition.data_size > 0) {
        serialized_input = mla_malloc(input_definition.data_size);

        if (serialized_input == nullptr) {
            return false; // Memory allocation failed
        }

        mla_serializer_t binarySerializer = mla_binary_serializer();
        mla_serializer_write_struct(binarySerializer, input_data, input_definition.write_function);

        mla_deserializer_t binaryDeserializer = mla_binary_deserializer();
        if (!mla_deserializer_read_struct(binaryDeserializer, serialized_input, input_definition.read_function)) {
            mla_free(serialized_input);
            return false; // Serialization failed
        }

    }

    // Create Output Buffer
    mla_pointer_t serialized_output = nullptr;

    if (output_data != nullptr && output_definition.data_size > 0) {
        serialized_output = mla_malloc(output_definition.data_size);

        if (serialized_output == nullptr) {
            if (serialized_input != nullptr) {
                mla_free(serialized_input);
            }
            return false; // Memory allocation failed
        }
    }

    mla_bool_t result = procedure.execute(serialized_input, serialized_output);
    mla_free(serialized_input);


    if (result) {
        // Deserialize output data
        mla_serializer_t binarySerializer = mla_binary_serializer();
        mla_serializer_write_struct(binarySerializer, serialized_output, input_definition.write_function);
        mla_free(serialized_output);

        mla_deserializer_t binaryDeserializer = mla_binary_deserializer();
        if (!mla_deserializer_read_struct(binaryDeserializer, output_data, output_definition.read_function)) {
            return false; // Serialization failed
        }

        return true;

    } else {
        if (serialized_output != nullptr) {
            mla_free(serialized_output);
        }

        return false; // Procedure execution failed
    }
}

mla_bool_t mla_rpc_execute_procedure_remote(const mla_string_t &procedure_name, const mla_serialize_definition_t &input_definition, const mla_serialize_definition_t &output_definition, const mla_pointer_t input_data, mla_pointer_t output_data) {


    mla_rpc_remote_endpoint_t endpoint = mla_rpc_remote_endpoint_invalid();

    if (!mla_rpc_remote_endpoint_find_handler(procedure_name, endpoint)) {
        return false; // No remote endpoint can handle this procedure
    }

    return endpoint.execute_procedure(endpoint.procedure_userdata, procedure_name, input_data, output_data);
}

mla_array_list_t<mla_rpc_procedure_unsafe_t, mla_rpc_procedure_unsafe_initializer> mla_rpc_list_procedures() {
    return mla_hash_map_values(g_rpc_container.procedures);
}

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_invalid() {
    return {
        0,
        0,
        mla_string_empty(),
        nullptr,
        nullptr,
        mla_buffer_reference_noOwner(),
        mla_buffer_reference_noOwner()
    };
}

mla_bool_t __mla_rpc_remote_endpoint_all_checker(const mla_callback_userdata &userdata, const mla_string_t &procedure_name) {
    (void)userdata;
    (void)procedure_name;
    return true;
}

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_all(mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_callback_userdata procedure_userdata, mla_buffer_reference_t procedure_userDataOwner) {

    return {
        0,
        procedure_userdata,
        mla_generate_runtime_id(),
        __mla_rpc_remote_endpoint_all_checker,
        execute_procedure_handler,
        mla_buffer_reference_noOwner(),
        procedure_userDataOwner
    };
}

mla_bool_t __mla_rpc_remote_endpoint_start_with_checker(const mla_callback_userdata &userdata,
                                                    const mla_string_t &procedure_name) {
    mla_char_t *pathPrefix = reinterpret_cast<mla_char_t *>(userdata);

    if (pathPrefix == nullptr)
        return false;

    mla_string_t str_prefix = mla_string_from_buffer_without_ownership(pathPrefix, mla_strlen(pathPrefix));

    return mla_string_equals(procedure_name, str_prefix);
}


mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint_start_with(mla_string_t start_string, mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_callback_userdata procedure_userdata, mla_buffer_reference_t procedure_userDataOwner) {

    mla_c_string_t c_string = mla_string_to_cString(start_string, true);

    if (c_string.c_str == nullptr) {
        return mla_rpc_remote_endpoint_invalid();
    }

    if (!c_string.isOwner) {
        return mla_rpc_remote_endpoint_invalid();
    }

    return {
        reinterpret_cast<mla_callback_userdata>(c_string.c_str),
        procedure_userdata,
        mla_generate_runtime_id(),
        __mla_rpc_remote_endpoint_start_with_checker,
        execute_procedure_handler,
        mla_buffer_reference(c_string.c_str),
        procedure_userDataOwner
    };
}

mla_rpc_remote_endpoint_t mla_rpc_remote_endpoint(mla_rpc_remote_endpoint_can_handle can_handle_handler, mla_rpc_remote_endpoint_execute_procedure execute_procedure_handler, mla_callback_userdata checker_userdata, mla_buffer_reference_t checker_userDataOwner, mla_callback_userdata procedure_userdata, mla_buffer_reference_t procedure_userDataOwner) {

    return {
        checker_userdata,
        procedure_userdata,
        mla_generate_runtime_id(),
        can_handle_handler,
        execute_procedure_handler,
        checker_userDataOwner,
        procedure_userDataOwner
    };
}

mla_bool_t mla_rpc_register_remote_endpoint(const mla_rpc_remote_endpoint_t &endpoint) {

    if (!mla_rw_lock_write(g_rpc_container.remote_endpoints_lock)) {
        return false;
    }

    if (endpoint.can_handle == nullptr || endpoint.execute_procedure == nullptr) {
        mla_rw_unlock_write(g_rpc_container.remote_endpoints_lock);
        return false;
    }

    mla_bool_t result = mla_array_list_add(g_rpc_container.remote_endpoints, endpoint);
    mla_rw_unlock_write(g_rpc_container.remote_endpoints_lock);

    return result;
}

mla_bool_t mla_rpc_unregister_remote_endpoint(const mla_rpc_remote_endpoint_t &endpoint) {

    if (!mla_rw_lock_write(g_rpc_container.remote_endpoints_lock)) {
        return false;
    }

    mla_bool_t result = false;

    for (mla_size_t i = 0; i < mla_array_list_size(g_rpc_container.remote_endpoints); i++) {
        mla_rpc_remote_endpoint_t& current_endpoint = mla_array_list_get_unsafe(g_rpc_container.remote_endpoints, i);
        if (mla_string_equals(endpoint.endpoint_id, current_endpoint.endpoint_id)) {
            mla_array_list_remove(g_rpc_container.remote_endpoints, i);
            result = true;
            break;
        }
    }

    mla_rw_unlock_write(g_rpc_container.remote_endpoints_lock);
    return result;
}

mla_bool_t mla_rpc_remote_endpoint_find_handler(const mla_string_t &procedure_name, mla_rpc_remote_endpoint_t& out_endpoint) {

    if (!mla_rw_lock_read(g_rpc_container.remote_endpoints_lock)) {
        return false;
    }

    for (mla_size_t i = 0; i < mla_array_list_size(g_rpc_container.remote_endpoints); i++) {
        mla_rpc_remote_endpoint_t& endpoint = mla_array_list_get_unsafe(g_rpc_container.remote_endpoints, i);

        if (endpoint.can_handle(endpoint.checker_userdata, procedure_name)) {
            out_endpoint = endpoint;
            mla_rw_unlock_read(g_rpc_container.remote_endpoints_lock);
            return true;
        }
    }

    mla_rw_unlock_read(g_rpc_container.remote_endpoints_lock);
    return false;

}

mla_bool_t mla_rpc_remote_endpoint_valid(const mla_rpc_remote_endpoint_t &endpoint) {
    return endpoint.execute_procedure != nullptr;
}

void __mla_rpc_container_lock() {
    g_rpc_container.isLocked = true;
}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_rpc_setup, __mla_rpc_container_lock);