//
// Created by chris on 9/13/2025.
//

#ifndef MLA_C_MLA_TEST_PRINT_STD_H
#define MLA_C_MLA_TEST_PRINT_STD_H

#include "../mla_test_data_types.h"
#include <cstdarg>
#include <cstdio>

// Simple printf implementation for standard output
// Note: This implementation directly uses vprintf.

inline void std_printf(const mla_test_char_t* str, mla_test_uint32_t length) {
    fwrite(str, 1, length, stdout);
}

mla_test_print_t g_test_print = {
    std_printf
};

#endif