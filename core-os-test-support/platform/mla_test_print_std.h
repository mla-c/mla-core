//
// Created by chris on 9/13/2025.
//

#ifndef COREOS_MLA_TEST_PRINT_STD_H
#define COREOS_MLA_TEST_PRINT_STD_H

#include "../mla_test_data_types.h"
#include <cstdarg>

void std_printf(const mla_test_char_t* format, ...) {
    va_list args;
    va_start(args, format);
    printf(format, args);
    va_end(args);
}

mla_test_print_t g_test_print = {
    .printf = std_printf
};

#endif