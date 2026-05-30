//
// Created by christian on 2/22/26.
//

#ifndef MLA_TASK_LOCAL_H
#define MLA_TASK_LOCAL_H

#include "../mla_data_types.h"
#include "../log/mla_logging.h"
#include "mla_task_manager_data_types.h"

struct mla_task_local_t {
    mla_pointer_t resource; // Pointer to the task local resource
};

mla_task_local_t mla_task_local_invalid();
mla_task_local_t mla_task_local();
mla_bool_t mla_task_local_set(const mla_task_local_t& local, mla_platform_pointer_t value);
mla_platform_pointer_t mla_task_local_get(const mla_task_local_t& local);

#define mla_task_local_create() mla_task_local()

#endif
