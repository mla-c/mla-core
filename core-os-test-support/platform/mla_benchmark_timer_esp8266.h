//
// Created by chris on 9/12/2025.
//

#ifndef COREOS_MLA_BENCHMARK_TIMER_ESP8266_H
#define COREOS_MLA_BENCHMARK_TIMER_ESP8266_H

#include "../Benchmark/mla_benchmark.h"
#include "espressif/esp_system.h"



mla_test_uint64_t __current_nanoseconds_esp8266() {

    static unsigned int last_us = 0;   // 32-bit
    static mla_test_uint64_t high = 0; // Oberes 32-bit, akkumuliert Overflows

    const unsigned int now_us = system_get_time();

    // Detect 32-bit wrap (~71.6 min) und akkumulieren
    if (now_us < last_us) {
        high += (mla_test_uint64_t)1 << 32;
    }
    last_us = now_us;

    const mla_test_uint64_t us64 = high + now_us;
    return us64 * 1000ULL; // in Nanosekunden (Auflösung bleibt µs)
}

mla_benchmark_timer_t g_benchmark_timer = {
    __current_nanoseconds_esp8266
};

#endif