//
// Created by chris on 10/17/2025.
//

#ifndef COREOS_TEST_MLA_TEST_CLI_UTILS_H
#define COREOS_TEST_MLA_TEST_CLI_UTILS_H

#include "mla_test_data_types.h"
#include "mla_test_utils.h"

static const mla_test_char_t* mla_test_cli_match_prefix(const mla_test_char_t* p_Str, const mla_test_char_t* p_Prefix) {
    while (*p_Prefix != '\0') {
        if (*p_Str != *p_Prefix) return nullptr;
        p_Str++;
        p_Prefix++;
    }
    return p_Str;
}

static mla_test_uint32_t mla_test_cli_parse_uint32(const mla_test_char_t* p_Str) {
    mla_test_uint32_t result = 0;
    while (*p_Str >= '0' && *p_Str <= '9') {
        result = result * 10 + (mla_test_uint32_t)(*p_Str - '0');
        p_Str++;
    }
    return result;
}

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

void mla_test_parse_allocation_failure_cmd(int argc, char** argv, mla_test_uint32_t& p_AllocationFailureSeed, mla_test_uint32_t& p_AllocationFailureSeedCount) {

    p_AllocationFailureSeed = 0;
    p_AllocationFailureSeedCount = 0;

    for (int i = 1; i < argc; ++i) {
        const mla_test_char_t* value = mla_test_cli_match_prefix(argv[i], "--allocation-failure-seed-count=");
        if (value != nullptr) {
            p_AllocationFailureSeedCount = mla_test_cli_parse_uint32(value);
            continue;
        }

        value = mla_test_cli_match_prefix(argv[i], "--allocation-failure-seed=");
        if (value != nullptr) {
            p_AllocationFailureSeed = mla_test_cli_parse_uint32(value);
        }
    }

}

#endif