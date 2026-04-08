//
// Created by chris on 9/12/2025.
//

#ifndef COREOS_MLA_BENCHMARK_TIMER_STD_H
#define COREOS_MLA_BENCHMARK_TIMER_STD_H

#include <chrono>
#include "../Benchmark/mla_benchmark.h"

mla_test_uint64_t __current_nanoseconds_std() {
    auto now = std::chrono::high_resolution_clock::now();
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    return static_cast<mla_test_uint64_t>(now_ns.time_since_epoch().count());
}

mla_benchmark_timer_t g_benchmark_timer = {
    __current_nanoseconds_std
};

#endif