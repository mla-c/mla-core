//
// Created by chris on 8/3/2025.
//

#ifndef MLA_NATIVE_LIST_TEST_H
#define MLA_NATIVE_LIST_TEST_H

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
#include "vector"
#include <algorithm>
#endif

#include "../lib/base-lib/test-support/mla_benchmark_executor.h"

void CArrayContainsBenchmark() {
    int* l_Array = (int*)mla_test_malloc(sizeof(int) * 100);

    for (int i = 0; i < 100; ++i) {
        l_Array[i] = i;
    }

    for (int i = 0; i < 100; ++i) {
        bool  l_Found = false;
        for (int j = 0; j < 100; ++j) {
            if (l_Array[j] == i) {
                l_Found = true;
                break;
            }
        }

        if (!l_Found) {
            // Fail
            static_assert(true, "Element not found in array");
        }

    }

    mla_test_free(l_Array);
}

void CArrayAddMuchItemsBenchmark() {
    int* l_Array = (int*)mla_test_malloc(sizeof(int) * 1000);
    for (int i = 0; i < 1000; ++i) {
        l_Array[i] = i;
    }

    mla_test_free(l_Array);
}

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
void StdVectorContainsBenchmark() {

    std::vector<int>* l_Vector = new std::vector<int>(100);
    for (int i = 0; i < 100; ++i) {
        l_Vector->at(i) = i;
    }

    for (int i = 0; i < 100; ++i) {
        if ((std::find(l_Vector->begin(), l_Vector->end(), i) != l_Vector->end()) == false) {
            // Fail
            static_assert(true, "Element not found in vector");

        }
    }

    delete l_Vector;
}

void StdVectorAddMuchItemsBenchmark() {
    std::vector<int>* l_Vector = new std::vector<int>(1000);
    for (int i = 0; i < 1000; ++i) {
        l_Vector->push_back(i);
    }

    delete l_Vector;
}
#endif

static mla_char_t* g_StringList[1000];

void SetupCArrayCleanUpBenchmark() {

    for (int i = 0; i < 1000; ++i) {
        g_StringList[i] = reinterpret_cast<mla_char_t*>(mla_platform_malloc(sizeof(mla_char_t) * 12));
        g_StringList[i][0] = 'T';
        g_StringList[i][1] = 'e';
        g_StringList[i][2] = 's';
        g_StringList[i][3] = 't';
        g_StringList[i][4] = 'S';
        g_StringList[i][5] = 't';
        g_StringList[i][6] = 'r';
        g_StringList[i][7] = 'i';
        g_StringList[i][8] = 'n';
        g_StringList[i][9] = 'g';
        g_StringList[i][10] = '\0';

    }

}

void CArrayCleanUpBenchmark() {

    for (int i = 0; i < 1000; ++i) {
        mla_platform_free(g_StringList[i]);
        g_StringList[i] = nullptr;
    }

}


void RegisterNativeListBenchmark(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("CArrayContains", "NativeList", CArrayContainsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("CArrayAddMuchItems", "NativeList", CArrayAddMuchItemsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

#if !defined(mla_benchmark_std) || (mla_benchmark_std == 1)
    benchmark = mla_benchmark("StdVectorContains", "NativeList", StdVectorContainsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("StdVectorAddMuchItems", "NativeList", StdVectorAddMuchItemsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
#endif

    // Memory Managment
    ////////////////////////////////////////////
    benchmark = mla_benchmark("CArrayCleanUp", "NativeList", CArrayCleanUpBenchmark, SetupCArrayCleanUpBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);


}

#endif
