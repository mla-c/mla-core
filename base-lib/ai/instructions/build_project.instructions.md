---
applyTo: '**'
description: 'Instructions on how to build the project and run tests/benchmarks'
---

# Build Instructions

## Project Structure and Build System

The project has no fixed main file and can be built with any build infrastructure (e.g., CMake, Zig, etc.). The repository contains a `sources.cmake` file which lists the source files, but note that it uses a `lib/` prefix for paths which might not exist physically in the file system. The actual directories (`core`, `platform`, `core-test`, `test-support`) are located directly at the repository root.

**Note:** These instructions are only valid if the repository contains no other built infrastructure. If the repository is imported as a git submodule or similar technology, the main repository is responsible for the build.

## Building for Testing

To build the project for testing or benchmarking, you need to create your own temporary `main` file and choose a build infrastructure that works for your environment.

### 1. Platform Headers
First, you must include all necessary platform headers for the low-level API. For a Linux environment, these typically include:

```cpp
#include "lib/base-lib/platform/linux/mla_global_platform_linux.h"
#include "lib/base-lib/platform/linux/mla_global_config_linux.h"
#include "lib/base-lib/platform/linux/mla_global_network_linux.h"
#include "lib/base-lib/platform/linux/mla_global_file_system_linux.h"
#include "lib/base-lib/platform/linux/mla_global_platform_task_manager_linux.h"
#include "lib/base-lib/test-support/platform/mla_test_memory_malloc.h"
#include "lib/base-lib/test-support/platform/mla_benchmark_timer_std.h"
#include "lib/base-lib/test-support/platform/mla_test_print_std.h"
#include "lib/base-lib/test-support/platform/mla_test_mutex_std.h"

#include "lib/base-lib/test-support/mla_test_cli_utils.h"
#include "lib/core-test/main_test.h"
#include "lib/base-lib/test-support/mla_test_data_types.h"
```

*Note: Depending on your include path configuration, you may need to adjust these paths or ensure the compiler can find them.*

### 2. General Main File Template

Below is a general `main` file template that sets up the environment and runs tests/benchmarks:

```cpp
int main(int argc, char** argv) {

    mla_test_bool_t runTest;
    mla_test_bool_t runBenchmark;
    mla_test_output_format_t benchmarkOutputFormat;

    mla_test_parse_cmd(argc, argv, runTest, runBenchmark, benchmarkOutputFormat);

    // Setup the file system
    mla_file_system_t fs = mla_file_system_native_create_data_restricted(mla_string_const("Test"));
    mla_file_system_initialize(mla_string_const("/"), fs);

    mla_boot_os_application();
    return run(runTest, runBenchmark, benchmarkOutputFormat);
}
```

## Critical Rules

1.  **Never commit temporary build files.**
2.  **Never commit the temporary main file.**
3.  Ensure you have the necessary platform implementations linked or included in your build.
