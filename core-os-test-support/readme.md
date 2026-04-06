# Test Support Library

The `core-os-test-support/` directory provides the testing and benchmarking infrastructure for CoreOS. It includes a custom test framework, a benchmark framework, and platform-specific utilities for running tests across all supported platforms.

## Architecture

The library is organized into three directories:

- **`Test/`**: Core test framework with assertion macros, test execution, and result reporting.
- **`Benchmark/`**: Performance benchmarking framework with CPU warmup, iteration control, and timing.
- **`platform/`**: Platform-specific implementations for timers, memory allocation, mutexes, and output.

## Test Framework

### Defining Tests

Tests are defined using the `mla_test_t` structure with a name, category, run function, and optional setup/teardown:

```cpp
#include "mla_test.h"

void my_test_run() {
    int result = 2 + 2;
    assert_equal(result, 4, "Addition should work correctly");
}

mla_test_t my_test = mla_test(
    "addition_test",    // name
    "math",             // category
    my_test_run,        // run function
    nullptr,            // setUp (optional)
    nullptr             // tearDown (optional)
);
```

### Assert Macros

The framework provides a comprehensive set of assertion macros:

| Macro | Description |
|-------|-------------|
| `assert_true(condition, message)` | Assert condition is true |
| `assert_false(condition, message)` | Assert condition is false |
| `assert_equal(actual, expected, message)` | Assert equality (supports all data types) |
| `assert_not_equal(actual, expected, message)` | Assert inequality |
| `assert_null(pointer, message)` | Assert pointer is null |
| `assert_not_null(pointer, message)` | Assert pointer is not null |
| `assert_struct_equal(T, actual, expected, message)` | Assert struct equality |
| `assert_fail(message)` | Unconditional test failure |

Supported data types for `assert_equal`/`assert_not_equal`: `bool`, `char`, `int8`, `int16`, `int32`, `int64`, `uint8`, `uint16`, `uint32`, `uint64`, `float`, `double`, `char*`, `void*`.

### Running Tests

```cpp
mla_test_bool_t success = mla_test_run(my_test);
```

### Memory Tracking

The test framework tracks memory allocations during test execution via `mla_test_result_t.allocated_memory` and can optionally block memory allocations with `block_memory_allocations` to verify zero-allocation code paths.

## Benchmark Framework

### Defining Benchmarks

Benchmarks follow the same structure as tests with additional iteration control:

```cpp
#include "mla_benchmark.h"

void my_benchmark_run() {
    // Code to benchmark (runs 1,000,000 iterations by default)
    volatile int x = 0;
    x++;
}

mla_benchmark_t my_benchmark = mla_benchmark(
    "increment_benchmark", // name
    "math",                // category
    my_benchmark_run,      // run function
    nullptr,               // setUp (optional)
    nullptr                // tearDown (optional)
);
```

### Configuration

- **Default iterations**: 1,000,000 (customizable via `#define mla_benchmark_iterations`)
- **CPU warmup iterations**: 1,000,000 (ensures consistent timing)
- **Iteration division**: Adjustable via `mla_benchmark_set_iteration_division()` for benchmarks with internal loops.

### Running Benchmarks

```cpp
mla_benchmark_run(my_benchmark, output_format);
```

### Timer Interface

The benchmark framework uses a pluggable timer interface:

```cpp
struct mla_benchmark_timer_t {
    mla_test_uint64_t (*current_nanoseconds)(void);
};
extern mla_benchmark_timer_t g_benchmark_timer;
```

## Platform Utilities

The `platform/` directory provides platform-specific implementations:

| Component | Implementations | Description |
|-----------|----------------|-------------|
| Timer | `std`, `esp8266`, `external` | Nanosecond-resolution timing for benchmarks |
| Memory | `malloc`, `external` | Memory allocation for test infrastructure |
| Mutex | `std`, `single_thread` | Synchronization for thread-safe test execution |
| Print | `std`, `Arduino serial`, `external` | Output backends for test results |
| WASM | `mla_test_module.js` | JavaScript test module for WASM environments |

## Integration

The test support library is used by all test files in `core-os-test/` and works across all supported CoreOS platforms (Linux, Windows, ESP-IDF, Raspberry Pi, WASM).

Current benchmark results: [CoreOS Benchmark Dashboard](https://cs-benchmark.github.io/benchmark-app/?project=CoreOS-Application-Test)
