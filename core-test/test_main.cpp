//
// Linux test entry point for the mla-core CMake build.
// Included by CMakeLists.txt when building the mla_tests target.
//

// ----- Platform implementations (Linux) ------
#include "core-platform/linux/mla_global_platform_linux.h"
#include "core-platform/linux/mla_global_config_linux.h"
#include "core-platform/linux/mla_global_platform_task_manager_linux.h"

#if !defined(mla_test_disable_network) || mla_test_disable_network != 1
#include "core-platform/linux/mla_global_network_linux.h"
#endif

#if !defined(mla_test_disable_file_system) || mla_test_disable_file_system != 1
#include "core-platform/linux/mla_global_file_system_linux.h"
#endif

#if !defined(mla_test_disable_external_task) || mla_test_disable_external_task != 1
#include "core-platform/linux/mla_global_external_task_linux.h"
#endif

// ----- Test framework platform implementations -----
#include "core-test-support/platform/mla_test_memory_malloc.h"
#include "core-test-support/platform/mla_benchmark_timer_std.h"
#include "core-test-support/platform/mla_test_print_std.h"
#include "core-test-support/platform/mla_test_mutex_std.h"

// ----- Test suite -----
#include "core-test-support/mla_test_cli_utils.h"
#include "core-test-support/mla_test_data_types.h"
#include "core-test/main_test.h"

int main(int argc, char** argv) {

    mla_test_bool_t runTest;
    mla_test_bool_t runBenchmark;
    mla_test_output_format_t benchmarkOutputFormat;

    mla_test_parse_cmd(argc, argv, runTest, runBenchmark, benchmarkOutputFormat);

#if !defined(mla_test_disable_file_system) || mla_test_disable_file_system != 1
    mla_file_system_t fs = mla_file_system_native_create_data_restricted(mla_string_const("Test"));
    mla_file_system_initialize(mla_string_const("/"), fs);
#endif

    mla_boot_os_application();
    return run(runTest, runBenchmark, benchmarkOutputFormat);
}
