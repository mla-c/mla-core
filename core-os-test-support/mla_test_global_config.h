#ifndef COREOS_TEST_MLA_TEST_GLOBAL_CONFIG_H
#define COREOS_TEST_MLA_TEST_GLOBAL_CONFIG_H

/**
 * @file mla_test_global_config.h
 * @brief Global configuration parameters for the MLA Test Support framework.
 *
 * This file contains default values for benchmark and unit test settings.
 * All parameters are wrapped in #ifndef guards to allow for compile-time overrides.
 */

/////////////////////////////////////////////////
// Benchmark Configuration
/////////////////////////////////////////////////

/**
 * @brief Default number of iterations for benchmark execution.
 * Can be reduced in CI environments to speed up test runs.
 */
#ifndef mla_global_config_benchmark_iterations
#define mla_global_config_benchmark_iterations 1000000
#endif

/**
 * @brief Flag to enable memory usage tracking during benchmarks.
 * Set to 1 to enable arena-based memory measurement, 0 to disable.
 */
#ifndef mla_global_feature_flag_benchmark_memory
#define mla_global_feature_flag_benchmark_memory 1
#endif

/**
 * @brief Maximum size in bytes for the memory arena used in benchmarks.
 */
#ifndef mla_global_config_benchmark_max_arena_size
#define mla_global_config_benchmark_max_arena_size (100 * 1024 * 1024) // 100 MB
#endif

/**
 * @brief Memory alignment requirement for benchmark arena allocations.
 */
#ifndef mla_global_config_benchmark_arena_alignment
#define mla_global_config_benchmark_arena_alignment 8u
#endif

/**
 * @brief Mode for reporting benchmark execution time.
 * Set to 1 to use median time (robust against outliers), 0 to use simple average.
 */
#ifndef mla_global_feature_flag_benchmark_use_median
#define mla_global_feature_flag_benchmark_use_median 1
#endif

/////////////////////////////////////////////////
// Test Configuration
/////////////////////////////////////////////////

/**
 * @brief Flag to enable allocation failure testing and memory tracking in unit tests.
 * Set to 1 to enable, 0 to disable.
 */
#ifndef mla_global_feature_flag_test_memory
#define mla_global_feature_flag_test_memory 1
#endif

#endif // COREOS_TEST_MLA_TEST_GLOBAL_CONFIG_H
