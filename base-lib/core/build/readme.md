# Build System

The mla-c build system uses CMake for cross-platform compilation, supporting multiple compilers and target platforms including native (Windows, Linux), embedded (ESP-IDF, Raspberry Pi), and WebAssembly.

## Architecture

The build system consists of:

- **`default-compile-config.cmake`**: Compiler-specific flags and configuration for all supported toolchains.
- **`default-toolchain.cmake`**: Toolchain setup including C/C++ standard versions, cross-compilation targets, debug configuration, and clang-tidy integration. Any project that includes this file automatically has the `MLA_ENABLE_CLANG_TIDY` option and `mla_enable_clang_tidy()` helper available.
- **`clang-tidy.cmake`**: Reusable module included by `default-toolchain.cmake`. Declares the `MLA_ENABLE_CLANG_TIDY` option, locates the clang-tidy binary, and provides the `mla_enable_clang_tidy(<target>)` helper function.
- **`tools/zig/`**: Zig compiler wrapper scripts for building WASM targets.
- **`sources.cmake`** (root): Master source file list organizing all mla-c library components.

## Supported Compilers

| Compiler | Platforms | Notes |
|----------|-----------|-------|
| GCC/G++ | Linux, Raspberry Pi | Address Sanitizer in Debug mode |
| Clang | Linux, macOS | Address Sanitizer in Debug mode |
| MSVC | Windows | `/W4` warning level, static linking |
| Emscripten | WASM (Browser/Node.js) | Standard and standalone modes |
| Zig | WASM (Standalone) | `wasm32-freestanding` target |

## Compiler Configuration

All compilers share common flags: `-Wall -Wextra -Wpedantic -Werror -fno-exceptions`.

### Debug Builds

Debug builds enable Address Sanitizer on GCC and Clang (Linux) for memory error detection. The debug flag `mla_debug_build=1` is defined automatically for Debug configurations.

### WASM Builds

Three WASM compilation modes are supported:

- **Standalone WASM** (`MLA_WASM_STANDALONE`): Minimal binary with `-nostdlib -ffreestanding`, no C library dependency.
- **Standalone JS** (`MLA_JS_STANDALONE`): JavaScript output via Emscripten with `emmalloc` allocator.
- **Default Emscripten**: Full Emscripten runtime with WASM output.

## Language Standards

- **C++**: C++11
- **C**: C11

## Toolchain Configuration

### Native Builds

```bash
mkdir build && cd build
cmake ..
make
```

### Cross-Compilation (WASM via Emscripten)

```bash
cmake -DMLA_EMSDK_PATH=/path/to/emsdk ..
make
```

### Cross-Compilation (WASM via Zig)

The `tools/zig/` directory provides compiler wrapper scripts (`zig-cc.sh`, `zig-cxx.sh` for Unix; `zig-cc.bat`, `zig-cxx.bat` for Windows) that invoke the Zig compiler as a C/C++ frontend:

```bash
cmake -DCMAKE_C_COMPILER=core/build/tools/zig/zig-cc.sh \
      -DCMAKE_CXX_COMPILER=core/build/tools/zig/zig-cxx.sh ..
make
```

## Source Organization

The `sources.cmake` file at `base-lib/core/` defines the complete set of source files organized by category:

- **Core Sources** (55 files): Data types, strings, buffers, collections, streams, serializers, logging, CLI, reflection, RPC, lifecycle, tasks, UI controls
- **Network Sources** (12 files): HTTP client/server, WebSocket, URL, network, RPC HTTP transport, web UI server
- **File System Sources**: Platform-abstracted file operations
- **UI Display Sources**: Native display surface implementations
- **Test Support Sources** (6 files): Test framework, benchmark framework, test utilities

## Clang-tidy Static Analysis

Static analysis is integrated directly into `default-toolchain.cmake` via `clang-tidy.cmake`.  The shared check configuration lives in `.clang-tidy` at `base-lib/core/build/` and is automatically picked up by clang-tidy for every file in the project tree.

Every project that includes `default-toolchain.cmake` automatically gets:

- The `MLA_ENABLE_CLANG_TIDY` option (default `ON`).
- The `mla_enable_clang_tidy(<target>)` CMake function.

After creating a target, call `mla_enable_clang_tidy` to attach analysis to it:

```cmake
include(lib/mla-core/core/build/default-toolchain.cmake)
include(lib/mla-core/core/build/default-compile-config.cmake)

add_executable(my_app ...)

mla_enable_clang_tidy(my_app)
```

Clang-tidy runs on every translation unit as part of the normal build.  Findings are reported inline alongside the compiler output.

Pass `-DMLA_ENABLE_CLANG_TIDY=OFF` to disable analysis (e.g. for fast iteration):

```bash
cmake -DMLA_ENABLE_CLANG_TIDY=OFF ..
```

The `.clang-tidy` file at `base-lib/core/build/` is picked up automatically when clang-tidy processes files inside that directory tree.  Projects may place their own `.clang-tidy` at their repository root (with `InheritParentConfig: false`) to extend or override the shared checks.
