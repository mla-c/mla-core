//
// Created by chris on 1/9/2026.
//

#ifndef COREOS_MLA_TEST_PRINT_EXTERNAL_H
#define COREOS_MLA_TEST_PRINT_EXTERNAL_H

#include "../mla_test_data_types.h"

extern "C" {
    __attribute__((import_module("mla_test"), import_name("external_test_print")))
    void external_test_print(const mla_test_char_t* str, mla_test_uint32_t length);
}

mla_test_print_t g_test_print = {
    external_test_print
};

#endif