//
// Created by Jules.
//

#ifndef MLA_AREA_POINTER_MEMORY_MANAGER_H
#define MLA_AREA_POINTER_MEMORY_MANAGER_H

#include "mla_data_types.h"
#include "task/mla_atomic.h"

struct mla_area_page_header_t {
    mla_size_t page_size;
    mla_int32_t refCount;
    mla_atomic_int32_t OtherTaskRefCount;
    mla_atomic_int32_t CurrentPosition; // Made atomic for lock-free sub-allocation
    mla_area_page_header_t* NextPage;
    mla_task_id_t creatorTaskId;
};

struct mla_area_pointer_memory_manager_t {
    mla_pointer_memory_manager_t manager;
    mla_area_page_header_t* currentPage;
    mla_size_t defaultPageSize;
    mla_atomic_int32_t lock; // Only for page list manipulation
};

mla_area_pointer_memory_manager_t mla_area_pointer_memory_manager_create(mla_size_t p_PageSize);

#endif
