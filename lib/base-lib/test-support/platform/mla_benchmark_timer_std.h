//
// Created by chris on 9/12/2025.
//

#ifndef MLA_BENCHMARK_TIMER_STD_H
#define MLA_BENCHMARK_TIMER_STD_H

#include "../Benchmark/mla_benchmark.h"

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>

// Calibrate TSC frequency once against QPC (busy-waits ~10 ms at startup)
static double mla_private_tsc_ns_per_tick() {
    static double ns_per_tick = 0.0;
    if (ns_per_tick != 0.0) {
        return ns_per_tick;
    }

    LARGE_INTEGER freq;
    LARGE_INTEGER start_qpc;
    LARGE_INTEGER end_qpc;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start_qpc);
    unsigned long long start_tsc = __rdtsc();

    // Busy-wait ~10 ms
    LARGE_INTEGER target;
    target.QuadPart = start_qpc.QuadPart + (freq.QuadPart / 100);
    LARGE_INTEGER cur;
    do { QueryPerformanceCounter(&cur); } while (cur.QuadPart < target.QuadPart);

    unsigned long long end_tsc = __rdtsc();
    QueryPerformanceCounter(&end_qpc);

    double qpc_ns = static_cast<double>(end_qpc.QuadPart - start_qpc.QuadPart) * 1e9
                    / static_cast<double>(freq.QuadPart);
    ns_per_tick = qpc_ns / static_cast<double>(end_tsc - start_tsc);
    return ns_per_tick;
}

static mla_test_uint64_t mla_private_current_nanoseconds_std() {
    static double ns_per_tick = mla_private_tsc_ns_per_tick();
    return static_cast<mla_test_uint64_t>(static_cast<double>(__rdtsc()) * ns_per_tick);
}

#else
#include <time.h>

static mla_test_uint64_t mla_private_current_nanoseconds_std() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<mla_test_uint64_t>(ts.tv_sec) * 1000000000ULL
         + static_cast<mla_test_uint64_t>(ts.tv_nsec);
}

#endif

mla_benchmark_timer_t g_benchmark_timer = {
    mla_private_current_nanoseconds_std
};

#endif