//
// Created by christian on 9/10/25.
//

#ifndef MAIN_TEST_H
#define MAIN_TEST_H


// Include Global Storage
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

#include "mla_data_types_test.h"
#include "mla_string_test.h"
#include "mla_number_test.h"
#include "mla_buffer_test.h"
#include "mla_reference_test.h"
#include "mla_memory_hook_test.h"
#include "mla_array_list_test.h"
#include "mla_link_list_test.h"
#include "mla_hash_map_test.h"
#include "mla_logger_console_test.h"
#include "mla_task_manager_test.h"
#include "mla_rw_lock_test.h"
#include "mla_mutex_test.h"
#include "mla_inject_test.h"
#include "mla_cli_parser_test.h"
#include "mla_cli_app_test.h"
#include "mla_task_cli_module_test.h"
#include "mla_serializer_test.h"
#include "mla_config_test.h"

// Network
#include "mla_http_header_test.h"
#include "mla_url_test.h"
#include "mla_http_client_test.h"

// File System
#include "mla_file_system_test.h"

#include "native_string_test.h"
#include "native_list_test.h"


int run(mla_test_bool_t runTest, mla_test_bool_t runBenchmark) {

    mla_test_executor_t l_TestExecutor = mla_test_executor(250);
    RegisterDataTypesTests(l_TestExecutor);
    RegisterStringTests(l_TestExecutor);
    RegisterNumberTests(l_TestExecutor);
    RegisterBufferTests(l_TestExecutor);
    RegisterReferenceTests(l_TestExecutor);
    RegisterMemoryHookTests(l_TestExecutor);
    RegisterArrayListTests(l_TestExecutor);
    RegisterLinkListTests(l_TestExecutor);
    RegisterHashMapTests(l_TestExecutor);
    RegisterLoggerConsoleTests(l_TestExecutor);
    RegisterTaskManagerTests(l_TestExecutor);
    RegisterReadWriteLockTests(l_TestExecutor);
    RegisterMutexTests(l_TestExecutor);
    RegisterInjectTests(l_TestExecutor);
    RegisterCliParserTests(l_TestExecutor);
    RegisterCliAppTests(l_TestExecutor);
    RegisterTaskCliModuleTests(l_TestExecutor);
    RegisterSerializerTests(l_TestExecutor);
    RegisterConfigTests(l_TestExecutor);

    // Network Tests
    RegisterHttpHeaderTests(l_TestExecutor);
    RegisterUrlTests(l_TestExecutor);
    RegisterHttpClientTests(l_TestExecutor);

    // File System Tests
    RegisterFileSystemPathTests(l_TestExecutor);


    mla_benchmark_executor_t l_BenchmarkExecutor = mla_benchmark_executor(75);
    RegisterStringBenchmarks(l_BenchmarkExecutor);
    RegisterNumberBenchmarks(l_BenchmarkExecutor);
    RegisterBufferBenchmarks(l_BenchmarkExecutor);
    RegisterMemoryHookBenchmarks(l_BenchmarkExecutor);
    RegisterArrayListBenchmarks(l_BenchmarkExecutor);
    RegisterLinkListBenchmarks(l_BenchmarkExecutor);
    RegisterHashMapBenchmarks(l_BenchmarkExecutor);
    RegisterMutexBenchmarks(l_BenchmarkExecutor);
    RegisterReadWriteLockBenchmarks(l_BenchmarkExecutor);
    RegisterInjectBenchmarks(l_BenchmarkExecutor);
    RegisterSerializerBenchmarks(l_BenchmarkExecutor);

    // Native Test for comparison with C++ std::string
    RegisterNativeStringBenchmarks(l_BenchmarkExecutor);
    RegisterNativeListBenchmark(l_BenchmarkExecutor);


    // Run the tests
    ////////////////////////////////////////

    mla_test_int32_t l_FailedTest = 0;

    if (runTest) {
        printf("Running Tests...\n");
        l_FailedTest = mla_test_executor_run_all_tests(l_TestExecutor);
        //l_FailedTest = mla_test_executor_run_test(l_TestExecutor, 33);
        //l_FailedTest = mla_test_executor_run_test(l_TestExecutor, 34);
        //l_FailedTest = mla_test_executor_run_test(l_TestExecutor, 35);
        //l_FailedTest = mla_test_executor_run_test(l_TestExecutor, 57);
        //l_FailedTest = mla_test_executor_run_test(l_TestExecutor, 128);
        printf("Tests completed with %d failed tests\n", l_FailedTest);
    }

    // Running benchmarks
    ////////////////////////////////////////

    printf("\n");

    if (runBenchmark) {

        printf("Running Benchmarks...\n\n");
        mla_benchmark_executor_run_all(l_BenchmarkExecutor);
        //mla_benchmark_executor_run(l_BenchmarkExecutor, 15);
        //mla_benchmark_executor_run(l_BenchmarkExecutor, 19);
        //mla_benchmark_executor_run(l_BenchmarkExecutor, 28);
        printf("\nBenchmarks completed\n");
    }

    // Clean up resources
    mla_test_executor_destroy(l_TestExecutor);
    mla_benchmark_executor_destroy(l_BenchmarkExecutor);

    return (int)l_FailedTest;
}

#endif //MAIN_TEST_H
