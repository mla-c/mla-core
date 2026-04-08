# Build System

The CoreOS build system uses CMake for cross-platform compilation, supporting multiple compilers and target platforms including native (Windows, Linux), embedded (ESP-IDF, Raspberry Pi), and WebAssembly.

## Architecture

The build system consists of:

- **`default-compile-config.cmake`**: Compiler-specific flags and configuration for all supported toolchains.
- **`default-toolchain.cmake`**: Toolchain setup including C/C++ standard versions, cross-compilation targets, and debug configuration.
- **`tools/zig/`**: Zig compiler wrapper scripts for building WASM targets.
- **`sources.cmake`** (root): Master source file list organizing all CoreOS library components.

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
cmake -DCMAKE_C_COMPILER=core-os/build/tools/zig/zig-cc.sh \
      -DCMAKE_CXX_COMPILER=core-os/build/tools/zig/zig-cxx.sh ..
make
```

## Source Organization

The `sources.cmake` file at the repository root defines the complete set of source files organized by category:

- **Core Sources** (55 files): Data types, strings, buffers, collections, streams, serializers, logging, CLI, reflection, RPC, lifecycle, tasks, UI controls
- **Network Sources** (12 files): HTTP client/server, WebSocket, URL, network, RPC HTTP transport, web UI server
- **File System Sources**: Platform-abstracted file operations
- **UI Display Sources**: Native display surface implementations
- **Test Support Sources** (6 files): Test framework, benchmark framework, test utilities
