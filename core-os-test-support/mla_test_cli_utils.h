//
// Created by chris on 10/17/2025.
//

#ifndef COREOS_TEST_MLA_TEST_CLI_UTILS_H
#define COREOS_TEST_MLA_TEST_CLI_UTILS_H

#include "mla_test_data_types.h"
#include "mla_test_utils.h"

void mla_test_parse_cmd(int argc, char** argv, mla_test_bool_t& runTest, mla_test_bool_t& runBenchmark, mla_test_output_format_t& benchmarkOutputFormat) {

    runTest = false;
    runBenchmark = false;
    benchmarkOutputFormat = mla_test_output_format_text;

    for (int i = 1; i < argc; ++i) {
        if (mla_test_strcmp(argv[i], "--test") == 0) {
            runTest = true;
        }

        if (mla_test_strcmp(argv[i], "--benchmark") == 0) {
            runBenchmark = true;
        }

        if (mla_test_strcmp(argv[i], "--benchmark-output=json") == 0) {
            benchmarkOutputFormat = mla_test_output_format_json;
        }
    }

    if (!runTest && !runBenchmark) {
        // If no flags are provided, run both tests and benchmarks by default
        runTest = true;
        runBenchmark = true;
    }


}

#endif