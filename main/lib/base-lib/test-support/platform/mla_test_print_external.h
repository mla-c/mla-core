//
// Created by chris on 1/9/2026.
//

#ifndef MLA_TEST_PRINT_EXTERNAL_H
#define MLA_TEST_PRINT_EXTERNAL_H

#include "../mla_test_data_types.h"

extern "C" {
    mla_wasm_import("mla_test", "external_test_print")
    void external_test_print(const mla_test_char_t* str, mla_test_uint32_t length);
}

mla_test_print_t g_test_print = {
    external_test_print
};

#endif