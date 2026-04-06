//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_BENCHMARK_H
#define COREOS_MLA_BENCHMARK_H

#include "../mla_test_data_types.h"

const mla_test_int32_t CONST_CPU_WARMUP_ITERATIONS = mla_global_config_benchmark_iterations;
const mla_test_int32_t CONST_BENCHMARK_ITERATIONS = mla_global_config_benchmark_iterations;

struct mla_benchmark_t {
    const mla_test_char_t *name;
    const mla_test_char_t *category;
    mla_test_uint32_t iterationDivision;

    void (*run)(void);

    void (*setUp)(void);

    void (*tearDown)(void);
};

mla_benchmark_t mla_benchmark(const mla_test_char_t *name,
                              const mla_test_char_t *category,
                              void (*run)(void),
                              void (*setUp)(void) = nullptr,
                              void (*tearDown)(void) = nullptr);

void mla_benchmark_set_iteration_division(mla_benchmark_t& benchmark, mla_test_uint32_t division);
void mla_benchmark_destroy(mla_benchmark_t &benchmark);
void mla_benchmark_run(mla_benchmark_t &benchmark, mla_test_output_format_t output_format);

struct mla_benchmark_timer_t {
    mla_test_uint64_t (*current_nanoseconds)(void);
};

extern mla_benchmark_timer_t g_benchmark_timer;


#define benchmark_category mla_test__FILENAME_ONLY__


#endif
