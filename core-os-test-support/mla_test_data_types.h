#ifndef COREOS_TESTDATATYPES_H
#define COREOS_TESTDATATYPES_H

#include "stdio.h"
#include "cstring"

#define mla_test_bool_t bool
#define mla_test_int8_t signed char
#define mla_test_uint8_t unsigned char
#define mla_test_int16_t signed int
#define mla_test_uint16_t unsigned int
#define mla_test_int32_t signed long
#define mla_test_uint32_t unsigned long
#define mla_test_int64_t signed long long
#define mla_test_uint64_t unsigned long long
#define mla_test_float_t float
#define mla_test_double_t double
#define mla_test_char_t char

#define mla_test_pointer_t void*

#define mla_test__FILENAME_ONLY__ \
(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : \
(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))


// Define a print function structure to allow custom print functions
// This is useful for embedded systems where printf may not be available

struct mla_test_print_t {
    void (*printf)(const char* format, ...);
};

extern mla_test_print_t g_test_print;

#define mla_test_printf(...) g_test_print.printf(__VA_ARGS__);


#endif
