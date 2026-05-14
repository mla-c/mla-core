//
// Created by chris on 9/12/2025.
//

#ifndef MLA_LIST_CONTAINS_CONST_H
#define MLA_LIST_CONTAINS_CONST_H

#include "../core-test-support/mla_test_data_types.h"

// You can define mla_benchmark_constains_count to override the default number of contains checks
// e.g. #define mla_benchmark_constains_count 50000
// This can be useful for CI systems where you want to reduce the number of iterations
// to speed up the tests.
#if defined(mla_benchmark_constains_count)
const int CONST_LIST_CONTAINS_COUNT = mla_benchmark_constains_count;
#else
const int CONST_LIST_CONTAINS_COUNT = 10000;
#endif

#endif