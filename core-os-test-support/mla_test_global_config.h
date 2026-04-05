#ifndef COREOS_TEST_MLA_TEST_GLOBAL_CONFIG_H
#define COREOS_TEST_MLA_TEST_GLOBAL_CONFIG_H

/////////////////////////////////////////////////
// Configuration Parameters for the MLA Test Support framework
/////////////////////////////////////////////////

// Benchmark Configuration
#ifndef mla_benchmark_iternations
#define mla_benchmark_iternations 1000000
#endif

#ifndef mla_benchmark_memory
#define mla_benchmark_memory 1
#endif

#ifndef mla_benchmark_max_arena_size
#define mla_benchmark_max_arena_size (100 * 1024 * 1024) // 100 MB
#endif

#ifndef mla_benchmark_arena_alignment
#define mla_benchmark_arena_alignment 8u // 8 bytes alignment
#endif

#ifndef mla_benchmark_use_median
#define mla_benchmark_use_median 1  // Default to median (1), set to 0 for average
#endif

// Test Configuration
#ifndef mla_test_memory
#define mla_test_memory 1
#endif

#endif // COREOS_TEST_MLA_TEST_GLOBAL_CONFIG_H
