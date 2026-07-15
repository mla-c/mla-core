//
// Created by chris on 1/9/2026.
//

#ifndef MLA_TEST_MEMORY_EXTERNAL_H
#define MLA_TEST_MEMORY_EXTERNAL_H

#include "../mla_test_data_types.h"

extern "C" {
    mla_wasm_import("mla_test", "external_test_malloc")
    void* external_test_malloc(mla_test_uint32_t size);
    mla_wasm_import("mla_test", "external_test_free")
    void external_test_free(void* ptr);
}

mla_test_memory_allocator_t g_test_memory_allocator {
    external_test_malloc,
    external_test_free
};

#endif