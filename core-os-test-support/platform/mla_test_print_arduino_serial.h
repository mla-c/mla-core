//
// Created by chris on 9/13/2025.
//

#ifndef COREOS_MLA_TEST_PRINT_ARDUINO_SERIAL_H
#define COREOS_MLA_TEST_PRINT_ARDUINO_SERIAL_H

#include <Arduino.h>

#include "../mla_test_data_types.h"

void arduino_printf(const mla_test_char_t* format, ...) {

    mla_test_char_t* l_Result = new mla_test_char_t[2048];

    va_list args;
    va_start(args, format);
    snprintf(l_Result, 2048, format, args);
    va_end(args);
    Serial.print(l_Result);
}

mla_test_print_t g_test_print = {
    .printf = arduino_printf
};

#endif