//
// Created by christian on 2/22/26.
//

#include "mla_task_local.h"

mla_buffer_cleanup_mode __mla_task_local_cleanup_hook(mla_pointer_t data, const mla_dynamic_data_t& userData) {

    (void)userData; // Silences the unused parameter warning

    mla_pointer_t localResource = data;
    mla_bool_t success = g_task_low_level_access.destroy_task_local(localResource);

    if (!success) {
        mla_error("Unable to destroy task local resource");
    }

    return CLEAN_UP_SKIP; // Indicate that the resource needs to be cleaned up
}

mla_task_local_t mla_task_local_invalid() {

    return {
        mla_string_empty(),
        nullptr,
        mla_buffer_reference_noOwner(),
    };
}

mla_task_local_t mla_task_local(const mla_string_t& name) {

    mla_task_local_t local = {
        name,
        nullptr,
        mla_buffer_reference_noOwner(),
    };

    // Create the native task local store
    mla_pointer_t outResource = nullptr;
    mla_bool_t success = g_task_low_level_access.create_task_local(&outResource);

    if (!success) {
        mla_string_t message = mla_string_concat("Failed to create task local: ", name);
        mla_error(message);
    }

    local.resource = outResource; // Assign the resource pointer
    local.resourceOwner = mla_buffer_reference_create(outResource, true, __mla_task_local_cleanup_hook, mla_dynamic_data_empty());
    return local;
}

mla_task_local_t mla_task_local(const mla_char_t* name, mla_size_t size) {

    return mla_task_local(mla_string(name, size));
}

mla_bool_t mla_task_local_set(const mla_task_local_t& local, mla_pointer_t value) {

    if (mla_string_length(local.name) == 0) {
        mla_error("Attempting to set value on an invalid task local");
        return false;
    }

    return g_task_low_level_access.set_task_local(local.resource, value);
}

mla_pointer_t mla_task_local_get(const mla_task_local_t& local) {

    if (mla_string_length(local.name) == 0) {
        mla_error("Attempting to get value from an invalid task local");
        return nullptr;
    }

    return g_task_low_level_access.get_task_local(local.resource);
}
