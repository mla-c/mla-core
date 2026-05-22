//
// Created by christian on 9/10/25.
//

#ifndef MAIN_TEST_H
#define MAIN_TEST_H


// Include Global Storage
#include "../core-test-support/mla_test_executor.h"
#include "../core-test-support/mla_benchmark_executor.h"

#include "mla_data_types_test.h"
#include "mla_pointer_test.h"
#include "mla_string_test.h"
#include "mla_string_builder_test.h"
#include "mla_number_test.h"
#include "mla_memory_hook_test.h"
#include "mla_array_list_test.h"
#include "mla_link_list_test.h"
#include "mla_hash_map_test.h"
#include "mla_logger_console_test.h"
#include "mla_task_manager_test.h"
#include "mla_rw_lock_test.h"
#include "mla_mutex_test.h"
#include "mla_cli_parser_test.h"
#include "mla_cli_app_test.h"
#include "mla_task_cli_module_test.h"
#include "mla_task_local_test.h"
#include "mla_serializer_test.h"
#include "mla_config_test.h"
#include "mla_id_test.h"
#include "mla_bytes_test.h"
#include "mla_atomic_test.h"
#include "mla_sha1_test.h"
#include "mla_hash_test.h"
#include "mla_rpc_test.h"
#include "mla_stream_test.h"
#include "mla_stream_deflate_test.h"
#include "mla_logger_rpc_test.h"
#include "mla_reflection_test.h"
#include "mla_reflection_rpc_test.h"
#include "mla_memory_test.h"
#include "mla_math_utils_test.h"
#include "mla_user_data_test.h"

#if !defined mla_test_disable_external_task || mla_test_disable_external_task != 1
#include "mla_external_task_test.h"
#endif

#if !defined mla_test_disable_network || mla_test_disable_network != 1
// Network
#include "mla_http_chunked_stream_test.h"
#include "mla_http_header_test.h"
#include "mla_url_test.h"
#include "mla_http_client_test.h"
#include "mla_http_server_test.h"
#include "mla_websocket_client_test.h"
#include "mla_http_rpc_test.h"
#include "mla_network_test.h"

#endif

#if !defined mla_test_disable_file_system|| mla_test_disable_file_system != 1
// File System
#include "mla_file_system_test.h"

#endif

#include "native_string_test.h"
#include "native_list_test.h"

int run(mla_test_bool_t runTest, mla_test_bool_t runBenchmark, mla_test_output_format_t benchmarkOutputFormat,
        mla_test_uint32_t p_AllocationFailureSeed = 0, mla_test_uint32_t p_AllocationFailureSeedCount = 0) {

    mla_test_executor_t l_TestExecutor = mla_test_executor();
    RegisterDataTypesTests(l_TestExecutor);
    RegisterPointerTests(l_TestExecutor);
    RegisterStringTests(l_TestExecutor);
    RegisterStringBuilderTests(l_TestExecutor);
    RegisterNumberTests(l_TestExecutor);
    RegisterMemoryHookTests(l_TestExecutor);
    RegisterArrayListTests(l_TestExecutor);
    RegisterLinkListTests(l_TestExecutor);
    RegisterHashMapTests(l_TestExecutor);
    RegisterLoggerConsoleTests(l_TestExecutor);
    RegisterTaskManagerTests(l_TestExecutor);
    RegisterReadWriteLockTests(l_TestExecutor);
    RegisterMutexTests(l_TestExecutor);
    RegisterCliParserTests(l_TestExecutor);
    RegisterCliAppTests(l_TestExecutor);
    RegisterTaskCliModuleTests(l_TestExecutor);
    RegisterTaskLocalTests(l_TestExecutor);
    RegisterSerializerTests(l_TestExecutor);
    RegisterConfigTests(l_TestExecutor);
    RegisterIdTests(l_TestExecutor);
    RegisterBytesTests(l_TestExecutor);
    RegisterAtomicTests(l_TestExecutor);
    RegisterSha1Tests(l_TestExecutor);
    RegisterHashTests(l_TestExecutor);
    RegisterRpcTests(l_TestExecutor);
    RegisterStreamTests(l_TestExecutor);
    RegisterStreamDeflateTests(l_TestExecutor);
    RegisterLoggerRpcTests(l_TestExecutor);
    RegisterReflectionTests(l_TestExecutor);
    RegisterReflectionRpcTests(l_TestExecutor);
    RegisterMathUtilsTests(l_TestExecutor);
    RegisterUserDataTests(l_TestExecutor);

#if !defined mla_test_disable_external_task || mla_test_disable_external_task != 1
    RegisterExternalTaskTests(l_TestExecutor);
#endif

#if !defined mla_test_disable_network || mla_test_disable_network != 1
    // Network Tests
    RegisterHttpChunkedStreamTests(l_TestExecutor);
    RegisterHttpHeaderTests(l_TestExecutor);
    RegisterUrlTests(l_TestExecutor);
    RegisterHttpClientTests(l_TestExecutor);
    RegisterHttpServerTests(l_TestExecutor);
    RegisterWebsocketClientTests(l_TestExecutor);
    RegisterHttpRpcTests(l_TestExecutor);
    RegisterNetworkTests(l_TestExecutor);

#endif

#if !defined mla_test_disable_file_system|| mla_test_disable_file_system != 1
    // File System Tests
    RegisterFileSystemPathTests(l_TestExecutor);
#endif

    mla_benchmark_executor_t l_BenchmarkExecutor = mla_benchmark_executor();
    RegisterPointerBenchmarks(l_BenchmarkExecutor);
    RegisterStringBenchmarks(l_BenchmarkExecutor);
    RegisterNumberBenchmarks(l_BenchmarkExecutor);
    RegisterMemoryHookBenchmarks(l_BenchmarkExecutor);
    RegisterArrayListBenchmarks(l_BenchmarkExecutor);
    RegisterLinkListBenchmarks(l_BenchmarkExecutor);
    RegisterHashMapBenchmarks(l_BenchmarkExecutor);
    RegisterMutexBenchmarks(l_BenchmarkExecutor);
    RegisterReadWriteLockBenchmarks(l_BenchmarkExecutor);
    RegisterSerializerBenchmarks(l_BenchmarkExecutor);
    RegisterAtomicBenchmarks(l_BenchmarkExecutor);
    RegisterSha1Benchmarks(l_BenchmarkExecutor);
    RegisterHashBenchmarks(l_BenchmarkExecutor);
    RegisterTaskLocalBenchmarks(l_BenchmarkExecutor);
    RegisterUserDataBenchmarks(l_BenchmarkExecutor);
    RegisterStreamDeflateBenchmarks(l_BenchmarkExecutor);

#if !defined mla_test_disable_network || mla_test_disable_network != 1
    // Network Benchmarks

    RegisterHttpServerBenchmarks(l_BenchmarkExecutor);

#endif

    // Native Test for comparison with mla_memory functions
    RegisterMemoryBenchmarks(l_BenchmarkExecutor);
    // Native Test for comparison with C++ std::string
    RegisterNativeStringBenchmarks(l_BenchmarkExecutor);
    RegisterNativeListBenchmark(l_BenchmarkExecutor);


    // Run the tests
    ////////////////////////////////////////

    mla_test_int32_t l_FailedTest = 0;
    mla_test_bool_t* l_SuccessMap = nullptr;

#if (!defined(mla_test_global_feature_flag_test_memory) || (mla_test_global_feature_flag_test_memory == 1))

    if (p_AllocationFailureSeed > 0) {
        // Fixed seed mode: only run allocation failure tests with this seed
        mla_test_print("Fixed Seed Mode: Running only allocation failure tests...\n", 58);
        l_FailedTest = mla_test_executor_run_all_tests_with_allocation_failure(l_TestExecutor, p_AllocationFailureSeed);

        mla_test_executor_destroy(l_TestExecutor);
        mla_benchmark_executor_destroy(l_BenchmarkExecutor);
        return (int) l_FailedTest;
    }

    if (p_AllocationFailureSeedCount > 0) {
        // Seed count mode: run regular tests first, then seed-based tests only for passing ones
        l_SuccessMap = (mla_test_bool_t*) mla_test_malloc(sizeof(mla_test_bool_t) * l_TestExecutor.count);
        for (mla_test_uint32_t i = 0; i < l_TestExecutor.count; i++) l_SuccessMap[i] = false;

        mla_test_print("Seed Count Mode: Running regular tests first...\n", 48);
        l_FailedTest = mla_test_executor_run_all_tests(l_TestExecutor, l_SuccessMap);

        mla_test_print("Tests completed with ", 21);
        mla_test_char_t buffer[12];
        mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), (mla_test_uint32_t)l_FailedTest);
        mla_test_print(buffer, strLength);
        mla_test_print(" failed tests\n", 14);

        l_FailedTest += mla_test_executor_run_all_tests_with_generated_allocation_failures(l_TestExecutor, p_AllocationFailureSeedCount, l_SuccessMap);

        if (l_SuccessMap) mla_test_free(l_SuccessMap);
    } else
#endif
    {
        // Regular mode
        if (runTest) {
            mla_test_print("Running Tests...\n", 17);
            //l_FailedTest = mla_test_executor_run_all_tests(l_TestExecutor);

            mla_test_print("Tests completed with ", 21);
            mla_test_char_t buffer[12];
            mla_test_uint32_t strLength = mla_uint32_to_string(buffer, sizeof(buffer), (mla_test_uint32_t)l_FailedTest);
            mla_test_print(buffer, strLength);
            mla_test_print(" failed tests\n", 14);
        }
    }

    // Running benchmarks
    ////////////////////////////////////////

    mla_test_print("\n", 1);

    if (runBenchmark && p_AllocationFailureSeed == 0 && p_AllocationFailureSeedCount == 0) {

        if (benchmarkOutputFormat == mla_test_output_format_text) {
            mla_test_print("Running Benchmarks...\n\n", 23);
        }

        mla_benchmark_executor_run_all(l_BenchmarkExecutor, benchmarkOutputFormat);
        //mla_benchmark_executor_run(l_BenchmarkExecutor, 90, benchmarkOutputFormat);
        //mla_benchmark_executor_run(l_BenchmarkExecutor, 19, benchmarkOutputFormat);
        //mla_benchmark_executor_run(l_BenchmarkExecutor, 82, benchmarkOutputFormat);

        if (benchmarkOutputFormat == mla_test_output_format_text) {
            mla_test_print("\nBenchmarks completed\n", 22);
        }
    }

    // Clean up resources
    mla_test_executor_destroy(l_TestExecutor);
    mla_benchmark_executor_destroy(l_BenchmarkExecutor);

    return (int) l_FailedTest;
}

#endif //MAIN_TEST_H
