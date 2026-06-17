# AGENTS Notes for `mla-core`

## Build & Configuration (project-specific)

- Top-level CMake target names used by this repo on Linux:
  - `MLA_C_App_Linux`
  - `MLA_C_Test_Linux_Single_Thread`
  - `MLA_C_Test_Linux_Multi_Thread`
- CMake toolchain and compile config are pinned from repo files:
  - `lib/base-lib/build/default-toolchain.cmake`
  - `lib/base-lib/build/default-compile-config.cmake`
- `ZLIB` is optional. `CMakeLists.txt` auto-detects it and enables zlib-dependent tests via compile definition `mla_test_featureflag_zlib=1` when present.
- In this workspace, use existing CLion profiles and build directories (do not create new ones):
  - `Debug` → `cmake-build-debug`
  - `Release` → `cmake-build-release`
  - `MinSizeRel` → `cmake-build-minsizerel`
  - `MinSizeRel (1)` → `cmake-build-minsizerel-1`

### Build commands

- Preferred build invocation from project root:

```bash
cmake --build cmake-build-debug --target MLA_C_Test_Linux_Single_Thread
```

- If your shell/profile does not expose `cmake`, build from CLion or use the profile environment that has CMake configured.

## Testing Workflow

### How tests are organized

- Tests live as header files in `core-test/tests/*.h`.
- Test executable entrypoints are platform files in `core-test/` (`main_test_linux_single_thread.cpp`, etc.).
- Aggregation/registration happens in `core-test/main_test.h` via calls like `RegisterXxxTests(l_TestExecutor);`.

### Running tests

- CLI flags are parsed in `lib/base-lib/test-support/mla_test_cli_utils.h`:
  - `--test` runs tests
  - `--benchmark` runs benchmarks
  - if no flags are passed, both tests and benchmarks run
  - `--benchmark-output=json` switches benchmark output format

#### Verified simple test run (executed)

- Command used in this environment:

```bash
LSAN_OPTIONS=detect_leaks=0 ./cmake-build-debug/MLA_C_Test_Linux_Single_Thread --test
```

- Result observed: exit code `0` and final line `Tests completed with 0 failed tests`.
- Note: without `LSAN_OPTIONS=detect_leaks=0`, this environment reported leak diagnostics and non-zero exit. Keep leak detection enabled for debugging sessions, but disable it when you need deterministic baseline execution in this workspace.

### Adding a new test

1. Create a new header in `core-test/tests/` (naming pattern: `mla_<module>_test.h`).
2. Define tests with `mla_test_t` and assertions from test-support.
3. Add a registration function (pattern: `Register<Module>Tests(mla_test_executor_t&)`).
4. Include the new header in `core-test/main_test.h`.
5. Register the test function in `run(...)` in `core-test/main_test.h`.
6. Rebuild test target and execute with `--test`.

## Development/Debugging Notes

- Codebase is C/C++ with heavy header-based test definitions and explicit platform include stacks; preserve include ordering and existing macro guards style.
- Naming style is `mla_<domain>_<name>` for functions/types and uppercase include guards for headers.
- Keep changes platform-aware: tests/features are commonly gated with compile definitions such as `mla_test_disable_network`, `mla_test_disable_file_system`, and `mla_test_disable_external_task`.
- For debugging test regressions, check:
  - registration in `core-test/main_test.h`
  - platform entrypoint selected by target
  - optional dependency gates (e.g., zlib feature flag)