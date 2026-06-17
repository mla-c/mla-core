//
// Created by christian on 2/22/26.
//

#include "mla_task_local.h"

mla_task_local_t mla_task_local_invalid() {

    return {
        mla_pointer_null()
    };
}

mla_task_local_t mla_task_local() {

    mla_task_local_t local = {
        mla_pointer_null()
    };

    // Create the native task local store
    mla_pointer_t outResource = mla_pointer_null();
    mla_bool_t success = g_task_low_level_access.create_task_local(outResource);

    if (!success) {
        mla_error("Failed to create task local");
    }

    local.resource = outResource; // Assign the resource pointer
    return local;
}

mla_bool_t mla_task_local_set(const mla_task_local_t& local, mla_platform_pointer_t value) {

    if (mla_pointer_is_null(local.resource)) {
        mla_error("Attempting to set value on an invalid task local");
        return false;
    }

    return g_task_low_level_access.set_task_local(local.resource, value);
}

mla_platform_pointer_t mla_task_local_get(const mla_task_local_t& local) {

    if (mla_pointer_is_null(local.resource)) {
        mla_error("Attempting to get value from an invalid task local");
        return nullptr;
    }

    return g_task_low_level_access.get_task_local(local.resource);
}
