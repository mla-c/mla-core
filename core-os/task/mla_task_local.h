//
// Created by christian on 2/22/26.
//

#ifndef COREOS_MLA_TASK_LOCAL_H
#define COREOS_MLA_TASK_LOCAL_H

#include "../mla_data_types.h"
#include "../log/mla_logging.h"
#include "mla_task_manager_data_types.h"
#include "../system/mla_string_concat.h"

struct mla_task_local_t {
    mla_string_t name;
    mla_pointer_t resource; // Pointer to the task local resource
    mla_buffer_reference_t resourceOwner; // Resource owner for the task local
};

mla_task_local_t mla_task_local_invalid();
mla_task_local_t mla_task_local(const mla_string_t& name);
mla_task_local_t mla_task_local(const mla_char_t* name, mla_size_t size);
mla_bool_t mla_task_local_set(const mla_task_local_t& local, mla_pointer_t value);
mla_pointer_t mla_task_local_get(const mla_task_local_t& local);

#define mla_task_local_create(name) mla_task_local(name, sizeof(name))

#endif
