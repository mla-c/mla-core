//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_MLA_MEMORY_HOOK_H
#define COREOS_MLA_MEMORY_HOOK_H

#include "../mla_data_types.h"

typedef mla_pointer_t(*mla_memory_malloc_t)(mla_size_t size);
typedef void (*mla_memory_free_t)(mla_pointer_t ptr);
typedef mla_bool_t (*mla_memory_is_gcc_pointer_t)(const mla_pointer_t ptr);

enum MLA_MEMORY_POINTER_TYPE {
    MLA_MEMORY_POINTER_TYPE_UNKNOWN = 0,
    MLA_MEMORY_POINTER_TYPE_GCC = 1,
    MLA_MEMORY_POINTER_TYPE_REGULAR = 2
};

typedef mla_bool_t(*mla_memory_malloc_hook_t)(mla_size_t size, mla_pointer_t* out_ptr);
typedef mla_bool_t (*mla_memory_free_hook_t)(mla_pointer_t ptr);
typedef MLA_MEMORY_POINTER_TYPE (*mla_memory_is_gcc_pointer_hook_t)(const mla_pointer_t ptr);

struct mla_memory_hook_t {
    mla_memory_malloc_hook_t malloc_hook;
    mla_memory_free_hook_t free_hook;
    mla_memory_is_gcc_pointer_hook_t is_gcc_pointer;
};

struct mla_global_memory_hook_list_t {
    mla_memory_malloc_t original_malloc;
    mla_memory_free_t original_free;
    mla_memory_is_gcc_pointer_t original_is_gcc_pointer;
    mla_int8_t hook_count;
    mla_memory_hook_t hooks[mla_global_config_max_memory_hooks];
};

mla_memory_hook_t mla_memory_hook_install(mla_memory_malloc_hook_t malloc_hook, mla_memory_free_hook_t free_hook, mla_memory_is_gcc_pointer_hook_t is_gcc_pointer = nullptr);
mla_bool_t mla_memory_hook_uninstall(const mla_memory_hook_t &hook);


#endif
