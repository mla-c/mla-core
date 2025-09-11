//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_NATIVE_LIST_TEST_H
#define COREOS_NATIVE_LIST_TEST_H

#include "vector"
#include <algorithm>
#include "../core-os-test-support/mla_benchmark_executor.h"

void CArrayContainsBenchmark() {
    int* l_Array = new int[100];

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

    delete[] l_Array;
}

void CArrayAddMuchItemsBenchmark() {
    int* l_Array = new int[1000];
    for (int i = 0; i < 1000; ++i) {
        l_Array[i] = i;
    }

    delete[] l_Array;
}

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

static mla_char_t* g_StringList[1000];

void SetupCArrayCleanUpBenchmark() {

    for (int i = 0; i < 1000; ++i) {
        mla_string_t data = mla_string_concat(mla_string("Test"), mla_string("String"));
        g_StringList[i] = const_cast<mla_char_t*>(mla_string_to_cString(data, true).c_str);
    }

}

void CArrayCleanUpBenchmark() {

    for (int i = 0; i < 1000; ++i) {
        mla_free(g_StringList[i]);
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

    benchmark = mla_benchmark("StdVectorContains", "NativeList", StdVectorContainsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("StdVectorAddMuchItems", "NativeList", StdVectorAddMuchItemsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Memory Managment
    ////////////////////////////////////////////
    benchmark = mla_benchmark("CArrayCleanUp", "NativeList", CArrayCleanUpBenchmark, SetupCArrayCleanUpBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);


}

#endif
