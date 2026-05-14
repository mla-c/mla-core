# WebAssembly (WASM) Platform Module

The WASM platform module provides platform-specific implementations for running mla-c applications in WebAssembly environments, supporting both browser and Node.js runtimes. It includes two compilation modes: a standard mode with POSIX compatibility and a standalone mode with zero C library dependencies.

## Architecture

The WASM platform module includes the following components:

- **Standard Platform** (`mla_global_platform_wasm.h`): WASM platform using native POSIX functions (usleep, clock_gettime) via Emscripten or similar toolchains.
- **Standalone Platform** (`mla_global_platform_wasm_standalone.h`): Zero-dependency WASM platform where all operations are imported from JavaScript, enabling minimal binary size.
- **Task Management** (`mla_global_platform_task_manager_wasm.h`): Single-threaded task scheduling (multi-threading is not available in WASM environments).
- **JavaScript Runtime** (`javascript/`): Web Worker-based runtime for executing WASM modules in browser and Node.js, with custom function extensibility.

## Compilation Modes

### Standard WASM

Uses native POSIX functions provided by the Emscripten runtime or a WASI-compatible environment:

```cpp
#include "core-platform/wasm/mla_global_platform_wasm.h"
```

- Memory operations: `memcpy`, `memset`, `memcmp`, `memmove`
- String operations: `strcpy`, `strlen`, `strstr`
- Number parsing: `strtod`, `strtoll`, `strtoull`
- Timing: `usleep`, `clock_gettime`

### Standalone WASM

All low-level operations are imported from JavaScript via WebAssembly import attributes, resulting in a minimal WASM binary with no C library dependency:

```cpp
#include "core-platform/wasm/mla_global_platform_wasm_standalone.h"
```

Each C library function is replaced by a JavaScript import (e.g., `external_memcpy`, `external_strlen`), allowing the JavaScript host to provide all implementations.

### Task Management

WASM environments use single-threaded task scheduling:

```cpp
#include "core-platform/wasm/mla_global_platform_task_manager_wasm.h"
```

This delegates to the generic `mla_task_manager_single_thread.h` implementation.

## JavaScript Runtime

The `javascript/` directory contains a complete runtime for loading and executing mla-c WASM modules.

### WASM Worker (`mla_wasm_worker.js`)

A Web Worker that provides the JavaScript side of the WASM runtime:

- **Memory Management**: Custom `malloc`/`free` implementation with heap tracking, free list management, and automatic memory page growth.
- **String Operations**: UTF-8 string reading/writing between WASM linear memory and JavaScript.
- **Number Parsing**: JavaScript implementations of `strtod`, `strtoll`, `strtoull`.
- **Timing**: `sleep` (via `Atomics.wait`) and `system_time_ms` (via `performance.now`).
- **Custom Functions**: Dynamically loadable JavaScript modules that extend the WASM import table.

### Browser Runner (`mla_wasm_runner.html`)

A browser-based UI for loading and running WASM modules:

- Drag-and-drop file loading for `.wasm` and `.js` files
- Real-time console output with color-coded log levels
- Execution statistics (module size, load time, execution time)
- Custom function loading support

### Node.js Runner (`mla_wasm_runner.js`)

A command-line tool for running WASM modules in Node.js:

```bash
node mla_wasm_runner.js <wasm-file> [options]
  -c, --custom <js-file>    Load custom JavaScript functions
  -v, --verbose              Verbose output
  --no-run                   Load module without executing
```

### Custom Functions

The runtime supports extending WASM imports with custom JavaScript functions. Use the template (`mla_wasm_runner_custom_functions_template.js`) as a starting point:

```javascript
const customMlaFunctions = {
    mla: {
        external_custom_add: (a, b) => a + b,
        external_custom_log: (messagePtr) => {
            const message = readString(messagePtr);
            postMessage({ type: 'log', message: `[Custom] ${message}` });
            return 0;
        }
    }
};
```

Custom functions have access to helper utilities: `readString`, `writeString`, `getMemoryView`, `externalMalloc`, `externalFree`, and `externalPrint`.

## Building for WASM

mla-c supports multiple WASM build configurations via CMake:

### Emscripten (Standard)
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
make
```

### Emscripten (Standalone)
```bash
cmake -DMLA_WASM_STANDALONE=ON ..
make
```

### Zig (Standalone)
```bash
cmake -DCMAKE_C_COMPILER=zig-cc -DCMAKE_CXX_COMPILER=zig-cxx ..
make
```

The standalone builds produce minimal WASM binaries with `-nostdlib -ffreestanding` flags, targeting `wasm32-freestanding`.

## Platform Capabilities

| Feature | Standard | Standalone |
|---------|----------|------------|
| Memory Operations | ✅ Native | ✅ JS Import |
| String Operations | ✅ Native | ✅ JS Import |
| Number Parsing | ✅ Native | ✅ JS Import |
| Timing/Sleep | ✅ POSIX | ✅ Atomics.wait |
| Threading | ❌ Single-threaded | ❌ Single-threaded |
| File System | ❌ Not available | ❌ Not available |
| Networking | ❌ Not available | ❌ Not available |
| Custom Functions | N/A | ✅ JS Extensible |

## Platform-Specific Notes

### Browser Execution

WASM modules run inside a Web Worker, communicating with the main thread via `postMessage`. The browser runner provides a complete UI for loading, executing, and monitoring WASM modules.

### Node.js Execution

Node.js execution uses `worker_threads` for isolation. The command-line runner supports colored terminal output and execution statistics.

### Memory Model

WASM uses linear memory shared between C++ and JavaScript. Pointers are byte offsets into this linear memory. The standalone mode includes a custom memory allocator in JavaScript with fragmentation handling and block coalescing.

## Integration with Other Modules

The WASM platform integrates with:

- **Lifecycle Module** - Boot events fire during `mla_boot_os_application()`
- **Task Module** - Single-threaded task scheduling
- **System Module** - Core data types and string operations
- **Serializer Module** - JSON/binary serialization (when compiled with standard mode)
