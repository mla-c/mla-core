//
// Created by chris on 1/9/2026.
//

#ifndef COREOS_MLA_TEST_PRINT_EXTERNAL_H
#define COREOS_MLA_TEST_PRINT_EXTERNAL_H

#include "../mla_test_data_types.h"

extern "C" {
    __attribute__((import_module("mla_test"), import_name("external_test_printf")))
    void external_test_printf(const mla_test_char_t* format, ...);
}

mla_test_print_t g_test_print = {
    external_test_printf
};

#endif