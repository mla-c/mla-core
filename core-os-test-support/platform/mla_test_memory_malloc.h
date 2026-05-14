//
// Created by chris on 1/9/2026.
//

#ifndef MLA_MLA_TEST_MEMORY_MALLOC_FREE_H
#define MLA_MLA_TEST_MEMORY_MALLOC_FREE_H

#include "../mla_test_data_types.h"
#include <cstdlib>

mla_platform_pointer_t __internal_test_malloc(mla_test_uint32_t size) {
    return malloc(size);
}

void __internal_test_free(mla_platform_pointer_t ptr) {
    free(ptr);
}

mla_test_memory_allocator_t g_test_memory_allocator {
    __internal_test_malloc,
    __internal_test_free
};

#endif