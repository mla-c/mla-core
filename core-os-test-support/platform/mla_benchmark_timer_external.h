//
// Created by chris on 1/9/2026.
//

#ifndef MLA_MLA_BENCHMARK_TIMER_EXTERNAL_H
#define MLA_MLA_BENCHMARK_TIMER_EXTERNAL_H

#include "../Benchmark/mla_benchmark.h"

extern "C" {
    __attribute__((import_module("mla_test"), import_name("external_test_current_nanoseconds")))
    mla_test_uint64_t external_test_current_nanoseconds();
}

mla_benchmark_timer_t g_benchmark_timer = {
    external_test_current_nanoseconds
};

#endif