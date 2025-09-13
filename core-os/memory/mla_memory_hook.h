//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_MLA_MEMORY_HOOK_H
#define COREOS_MLA_MEMORY_HOOK_H

#include "../mla_data_types.h"

#define CONST_MLA_MAX_MEMORY_HOOKS 10

typedef mla_pointer_t(*mla_memory_malloc_t)(mla_size_t size);
typedef void (*mla_memory_free_t)(mla_pointer_t ptr);


typedef mla_pointer_t(*mla_memory_malloc_hook_t)(mla_size_t size);
typedef mla_bool_t (*mla_memory_free_hook_t)(mla_pointer_t ptr);

struct mla_memory_hook_t {
    mla_memory_malloc_hook_t malloc_hook;
    mla_memory_free_hook_t free_hook;
};

struct mla_global_memory_hook_list_t {
    mla_memory_malloc_t original_malloc;
    mla_memory_free_t original_free;
    mla_int8_t hook_count;
    mla_memory_hook_t hooks[CONST_MLA_MAX_MEMORY_HOOKS];
};

mla_memory_hook_t mla_memory_hook_install(mla_memory_malloc_hook_t malloc_hook, mla_memory_free_hook_t free_hook);
mla_bool_t mla_memory_hook_uninstall(const mla_memory_hook_t &hook);


#endif
