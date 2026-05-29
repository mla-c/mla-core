# mla-c Library Collection

A comprehensive, cross-platform C++ framework for building modern applications with support for Windows, Linux, WASM, Raspberry Pi, and ESP-IDF platforms.

## 📋 Overview

mla-c is a modular, lightweight operating system abstraction library that provides essential functionality for application development across multiple platforms. It offers a consistent API surface while maintaining platform-specific optimizations.

## 🎯 Key Features

- ✅ **Cross-Platform Support** - Windows, Linux, WASM, Raspberry Pi, ESP-IDF
- ✅ **Zero External Dependencies** - Pure C++ implementation
- ✅ **Memory Management** - Custom allocators and memory hooks
- ✅ **Concurrency Primitives** - Tasks, mutexes, and read-write locks
- ✅ **HTTP Client & Server** - Built-in networking capabilities
- ✅ **Serialization** - JSON and binary serialization support
- ✅ **Dependency Injection** - Service locator pattern implementation
- ✅ **Testing Framework** - Built-in test and benchmark support

## 📦 Library Components

### mla-c (`base-lib/core/`)

The main library providing fundamental operating system abstractions and utilities. [Documentation](base-lib/core/readme.md)

#### **System** (`system/`) - [Documentation](base-lib/core/system/readme.md)
- **String Management** - `mla_string.h` - UTF-8/16/32 string handling with conversion utilities
- **Buffers** - `mla_buffer.h` - Dynamic byte buffer management
- **Collections**:
  - `mla_array_list.h` - Dynamic array implementation
  - `mla_link_list.h` - Linked list data structure
  - `mla_hash_map.h` - Hash table implementation
- **Numbers** - `mla_number.h` - Numeric type utilities and conversions
- **Streams** - `mla_stream.h` - I/O stream abstractions
- **Reference Counting** - `mla_reference.h` - Smart pointer utilities

#### **Task Management** (`task/`) - [Documentation](base-lib/core/task/readme.md)
- **Task Manager** - `mla_task_manager.h` - Async task execution and scheduling
- **Mutex** - `mla_mutx.h` - Mutual exclusion primitives
- **RW Lock** - `mla_rw_lock.h` - Reader-writer synchronization
- **Task** - `mla_task.h` - Task abstraction layer
- **CLI Module** - `mla_task_cli_module.h` - CLI integration for task management

#### **HTTP & Networking** (`http/`, `network/`, `url/`)
- **HTTP Client** - `mla_http_client.h` - HTTP/HTTPS client implementation - [HTTP Documentation](base-lib/core/http/readme.md)
- **HTTP Server** - `mla_http_server.h` - Lightweight HTTP server - [HTTP Documentation](base-lib/core/http/readme.md)
- **HTTP Headers** - `mla_http_header.h` - Header parsing and management - [HTTP Documentation](base-lib/core/http/readme.md)
- **Network** - `mla_network.h` - Socket abstractions - [Network Documentation](base-lib/core/network/readme.md)
- **URL Parser** - `mla_url.h` - URL parsing and manipulation - [URL Documentation](base-lib/core/url/readme.md)

#### **Remote Procedure Call (RPC)** (`rpc/`) - [Documentation](base-lib/core/rpc/readme.md)
- **RPC Framework** - `mla_rpc.h` - Comprehensive RPC implementation with support for both safe and unsafe procedure handlers, remote endpoint registration, and automated procedure registration using lifecycle events.
- **HTTP Integration** - `mla_rpc_http_client.h`, `mla_rpc_http_server.h` - Built-in support for RPC over HTTP, enabling seamless client-server communication.

#### **Lifecycle Management** (`lifecycle/`) - [Documentation](base-lib/core/lifecycle/readme.md)
- **Boot Events** - `mla_lifecycle_events.h` - Prioritized boot event management system that allows for flexible and ordered initialization of components. Callbacks can be registered with specific priorities to ensure proper startup sequencing.

#### **Serialization** (`serializer/`) - [Documentation](base-lib/core/serializer/readme.md)
- **JSON Serializer** - `mla_json_serializer.h` - JSON encode/decode
- **Binary Serializer** - `mla_binary_serializer.h` - Binary data serialization
- **Serializer Interface** - `mla_serializer.h` - Common serialization API

#### **Logging** (`log/`) - [Documentation](base-lib/core/log/readme.md)
- **Logger** - `mla_logger.h` - Flexible logging framework
- **Console Logger** - `mla_logger_console.h` - Standard output logging
- **Log Levels** - `mla_logging.h` - Configurable log severity levels

#### **Command Line Interface** (`cli/`) - [Documentation](base-lib/core/cli/readme.md)
- **CLI App** - `mla_cli_app.h` - Command-line application framework
- **CLI Parser** - `mla_cli_parser.h` - Argument parsing utilities
- **Commands** - `mla_cli_command.h` - Command definition and execution

#### **Configuration** (`config/`) - [Documentation](base-lib/core/config/readme.md)
- **Config Manager** - `mla_config.h` - Configuration file management and parsing

#### **Dependency Injection** (`inject/`)
- **Service Injector** - `mla_inject.h` - Dependency injection container
- **Service Registry** - `mla_inject_services.h` - Service registration and lookup

#### **File System** (`filesystem/`) - [Documentation](base-lib/core/filesystem/readme.md)
- **File System** - `mla_file_system.h` - File and directory operations

#### **Memory Management** (`memory/`) - [Documentation](base-lib/core/memory/readme.md)
- **Memory Hook** - `mla_memory_hook.h` - Custom memory allocator integration

#### **Utilities** (`utils/`, `hash/`)
- **Character Utils** - `mla_char_utils.h` - Character manipulation functions - [Utils Documentation](base-lib/core/utils/readme.md)
- **Endian Utils** - `mla_endian_utils.h` - Handling byte order - [Utils Documentation](base-lib/core/utils/readme.md)
- **Hash Functions** - `mla_hash.h` - Hashing algorithms - [Hash Documentation](base-lib/core/hash/readme.md)

#### **User Interface** (`ui/`) - [Documentation](base-lib/core/ui/readme.md)
- **UI Controls** - `mla_ui_button.h`, `mla_ui_label.h`, `mla_ui_text_edit.h`, `mla_ui_window.h` - Component-based UI widgets
- **UI Surfaces** - `mla_ui_surface.h`, `mla_ui_surface_draw.h` - Drawing primitives and surface abstraction
- **UI HTTP Server** - `mla_ui_http_server.h` - Web-based user interface framework
- **UI RPC** - `mla_ui_rpc.h` - RPC integration for UI communication
- **Embedded Web** - `mla_ui_web_embedded.h` - Embedded web resources for serving UI assets

#### **Reflection** (`reflection/`) - [Documentation](base-lib/core/reflection/readme.md)
- **Reflection System** - `mla_reflection.h` - Runtime type information and metadata
- **Reflection RPC** - `mla_reflection_rpc.h` - RPC integration with reflection system

#### **Data Types** (`mla_data_types.h`)
- Platform-independent type definitions (int8, int16, int32, int64, etc.)
- Cross-platform compatibility layer

### Platform Abstractions (`base-lib/core-platform/`)

Platform-specific implementations providing native functionality:

- **Generic** - Portable implementations for all platforms - [Documentation](base-lib/core-platform/generic/readme.md)
- **Windows** - Windows API integration - [Documentation](base-lib/core-platform/windows/readme.md)
- **Linux** - POSIX-compliant implementations - [Documentation](base-lib/core-platform/linux/readme.md)
- **WASM** - WebAssembly browser/Node.js runtime - [Documentation](base-lib/core-platform/wasm/readme.md)
- **Raspberry Pi** - ARM-specific optimizations - [Documentation](base-lib/core-platform/raspberry/readme.md)
- **ESP-IDF** - ESP32/ESP8266 embedded support - [Documentation](base-lib/core-platform/espidf/readme.md)

### Test Framework (`core-test/`) - [Documentation](core-test/readme.md)

Comprehensive test suite covering all core functionality:

- Array List Tests
- Buffer Tests
- CLI Tests (App & Parser)
- Configuration Tests
- Data Types Tests
- File System Tests
- Hash Map Tests
- HTTP Tests (Client, Server, Headers)
- Dependency Injection Tests
- Link List Tests
- Logger Tests
- Memory Hook Tests
- Mutex & RW Lock Tests
- Number Tests
- Reference Counting Tests
- Serialization Tests
- String Tests
- Task Manager Tests
- URL Tests
- Native Integration Tests

### Test Support Library (`base-lib/core-test-support/`) - [Documentation](base-lib/core-test-support/readme.md)

Testing and benchmarking infrastructure:

#### **Testing Framework**
- **Test Executor** - `mla_test_executor.h` - Test runner and result aggregation
- **Test Utilities** - `mla_test_cli_utils.h` - CLI integration for tests
- **Test Types** - `mla_test_data_types.h` - Test data structures
- **Platform Print** - Platform-specific output (STD, Arduino Serial)

#### **Benchmarking**
- **Benchmark Executor** - `mla_benchmark_executor.h` - Performance testing framework
- **Benchmark Timer** - Platform-specific timing (STD, ESP8266)

Current Benchmark Result: [HERE](https://cs-benchmark.github.io/benchmark-app/?project=mla-c-Application-Test)

### Example Application (`core-exmaple-app/`) - [Documentation](core-exmaple-app/readme.md)

Reference implementation demonstrating library usage:

- **Main App** - `main_app.h` - Application entry point
- **CLI Integration** - `main_app_cli.h` - Command-line interface example
- **Web UI** - `main_app_web_ui.h` - HTTP server with RPC and web-based UI surfaces
- **Window UI** - `main_app_window_ui.h` - Native display surface rendering
- **Main Window** - `main_app_main_window.h` - UI controls showcase (labels, buttons, text inputs)
- **Background Task** - `main_app_background_task.h` - Periodic background task example

### Web UI Application (`core-web-ui/`) - [Documentation](core-web-ui/README.md)

Browser-based interface for mla-c applications built with Preact and TypeScript. The production build is embedded into the C++ library as a compressed byte array and served by the mla-c HTTP server at runtime.


## Code Style (mla-c)

mla-c follows a **C-style API with C++ features** to keep the library portable and consistent across all targets.

### 1) Type System: Use MLA data types

Use types from `base-lib/core/mla_data_types.h` in APIs and module code (`mla_int32_t`, `mla_size_t`, `mla_bool_t`, `mla_char_t`, ...).

```cpp
mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2);
mla_pointer_t mla_create_char_array(const mla_size_t p_Length);
```

### 2) Naming: `mla_<module>_<action>` and `mla_<name>_t`

Function names follow `mla_<module>_<action>`. Struct/type names use `_t`.

```cpp
struct mla_string_t { ... };
mla_string_t mla_string_copy(const mla_char_t *p_Data, mla_size_t p_Length);
void mla_string_destroy(mla_string_t &p_String);
```

### 3) Function Signatures: references + const correctness

The codebase uses C++ references for struct parameters and `const` for read-only inputs. Parameter names are typically prefixed with `p_`.

```cpp
mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2);
mla_string_t mla_string_copy(const mla_string_t &p_String);
```

### 4) Memory Access and Ownership

- Use MLA low-level wrappers (`mla_memcpy`, `mla_memset`, `mla_strlen`, ...).
- Use `mla_pointer_t` for heap ownership.
- Use `mla_platform_pointer_t` for transient/raw access only.
- Keep platform-specific implementation in `base-lib/core-platform/*` so shared module APIs remain cross-platform.

## 🧠 Heap Memory Ownership: `mla_pointer_t`

`mla_pointer_t` is the project’s managed pointer type for heap-owned data.

- Allocate owned memory with `mla_malloc(...)`, `mla_malloc_buffer(...)`, or `mla_malloc_struct(T)`.
- Access payload data with `mla_pointer_get_data<T>(ptr)` / `mla_pointer_get_platform_pointer(ptr)` and always null-check before dereferencing.
- Use `mla_pointer_null()` and `mla_pointer_is_null(...)` for null-state handling.
- Copying `mla_pointer_t` shares ownership through reference counting (`incReferences` on copy / assignment, `decReferences` in destructor).
- Avoid using raw `mla_platform_malloc` as an ownership container in module code; prefer `mla_pointer_t` for automatic cleanup and consistent memory-manager behavior.

Example:

```cpp
mla_pointer_t ptr = mla_malloc_struct(mla_string_internal_heap_t);
mla_string_internal_heap_t* data = mla_pointer_get_data<mla_string_internal_heap_t>(ptr);
if (data != nullptr) {
    data->length = 0;
}
```

For module-level guidance, see [base-lib/core/readme.md](base-lib/core/readme.md) and [base-lib/core/memory/readme.md](base-lib/core/memory/readme.md).

## 📚 Architecture

mla-c follows a layered architecture:

```
┌─────────────────────────────────────┐
│    Applications & Example Apps      │
├─────────────────────────────────────┤
│         mla-c Library             │
│  (System, HTTP, Tasks, Logging...)  │
├─────────────────────────────────────┤
│      Platform Abstractions          │
│  (Windows, Linux, WASM, RPi, ESP)   │
├─────────────────────────────────────┤
│         Operating System            │
└─────────────────────────────────────┘
```

### Design Principles

- **Modularity** - Each component can be used independently
- **Platform Abstraction** - Write once, run anywhere
- **Zero Cost Abstraction** - Minimal overhead over native APIs
- **Header-Only Where Possible** - Easy integration
- **No STL Dependency** - Custom implementations for embedded systems

## 🧪 Testing

The library includes comprehensive test coverage:

- **Unit Tests** - Individual component testing
- **Integration Tests** - Cross-component functionality
- **Platform Tests** - Platform-specific validation
- **Benchmarks** - Performance measurements



**Built for performance, designed for portability** 🚀
