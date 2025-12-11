# CoreOS Library Collection

A comprehensive, cross-platform C++ framework for building modern applications with support for Windows, Linux, WASM, Raspberry Pi, and ESP-IDF platforms.

## 📋 Overview

CoreOS is a modular, lightweight operating system abstraction library that provides essential functionality for application development across multiple platforms. It offers a consistent API surface while maintaining platform-specific optimizations.

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

### Core OS (`core-os/`)

The main library providing fundamental operating system abstractions and utilities.

#### **System** (`system/`)
- **String Management** - `mla_string.h` - UTF-8/16/32 string handling with conversion utilities
- **Buffers** - `mla_buffer.h` - Dynamic byte buffer management
- **Collections**:
  - `mla_array_list.h` - Dynamic array implementation
  - `mla_link_list.h` - Linked list data structure
  - `mla_hash_map.h` - Hash table implementation
- **Numbers** - `mla_number.h` - Numeric type utilities and conversions
- **Streams** - `mla_stream.h` - I/O stream abstractions
- **Reference Counting** - `mla_reference.h` - Smart pointer utilities

#### **Task Management** (`task/`)
- **Task Manager** - `mla_task_manager.h` - Async task execution and scheduling
- **Mutex** - `mla_mutx.h` - Mutual exclusion primitives
- **RW Lock** - `mla_rw_lock.h` - Reader-writer synchronization
- **Task** - `mla_task.h` - Task abstraction layer
- **CLI Module** - `mla_task_cli_module.h` - CLI integration for task management

#### **HTTP & Networking** (`http/`, `network/`, `url/`)
- **HTTP Client** - `mla_http_client.h` - HTTP/HTTPS client implementation
- **HTTP Server** - `mla_http_server.h` - Lightweight HTTP server
- **HTTP Headers** - `mla_http_header.h` - Header parsing and management
- **Network** - `mla_network.h` - Socket abstractions
- **URL Parser** - `mla_url.h` - URL parsing and manipulation

#### **Remote Procedure Call (RPC)** (`rpc/`)
- **RPC Framework** - `mla_rpc.h` - Comprehensive RPC implementation with support for both safe and unsafe procedure handlers, remote endpoint registration, and automated procedure registration using lifecycle events.
- **HTTP Integration** - `mla_rpc_http_client.h`, `mla_rpc_http_server.h` - Built-in support for RPC over HTTP, enabling seamless client-server communication.

#### **Lifecycle Management** (`lifecycle/`)
- **Boot Events** - `mla_lifecycle_events.h` - Prioritized boot event management system that allows for flexible and ordered initialization of components. Callbacks can be registered with specific priorities to ensure proper startup sequencing.

#### **Serialization** (`serializer/`)
- **JSON Serializer** - `mla_json_serializer.h` - JSON encode/decode
- **Binary Serializer** - `mla_binary_serializer.h` - Binary data serialization
- **Serializer Interface** - `mla_serializer.h` - Common serialization API

#### **Logging** (`log/`)
- **Logger** - `mla_logger.h` - Flexible logging framework
- **Console Logger** - `mla_logger_console.h` - Standard output logging
- **Log Levels** - `mla_logging.h` - Configurable log severity levels

#### **Command Line Interface** (`cli/`)
- **CLI App** - `mla_cli_app.h` - Command-line application framework
- **CLI Parser** - `mla_cli_parser.h` - Argument parsing utilities
- **Commands** - `mla_cli_command.h` - Command definition and execution

#### **Configuration** (`config/`)
- **Config Manager** - `mla_config.h` - Configuration file management and parsing

#### **Dependency Injection** (`inject/`)
- **Service Injector** - `mla_inject.h` - Dependency injection container
- **Service Registry** - `mla_inject_services.h` - Service registration and lookup

#### **File System** (`filesystem/`)
- **File System** - `mla_file_system.h` - File and directory operations

#### **Memory Management** (`memory/`)
- **Memory Hook** - `mla_memory_hook.h` - Custom memory allocator integration

#### **Utilities** (`utils/`, `hash/`)
- **Character Utils** - `mla_char_utils.h` - Character manipulation functions
- **Hash Functions** - `mla_hash.h` - Hashing algorithms

#### **Data Types** (`mla_data_types.h`)
- Platform-independent type definitions (int8, int16, int32, int64, etc.)
- Cross-platform compatibility layer

### Platform Abstractions (`core-os-platform/`)

Platform-specific implementations providing native functionality:

- **Windows** - Windows API integration
- **Linux** - POSIX-compliant implementations
- **WASM** - WebAssembly browser/Node.js runtime
- **Raspberry Pi** - ARM-specific optimizations
- **ESP-IDF** - ESP32/ESP8266 embedded support

### Test Framework (`core-os-test/`)

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

### Test Support Library (`core-os-test-support/`)

Testing and benchmarking infrastructure:

#### **Testing Framework**
- **Test Executor** - `mla_test_executor.h` - Test runner and result aggregation
- **Test Utilities** - `mla_test_cli_utils.h` - CLI integration for tests
- **Test Types** - `mla_test_data_types.h` - Test data structures
- **Platform Print** - Platform-specific output (STD, Arduino Serial)

#### **Benchmarking**
- **Benchmark Executor** - `mla_benchmark_executor.h` - Performance testing framework
- **Benchmark Timer** - Platform-specific timing (STD, ESP8266)

Current Benchmark Result: [HERE](https://cs-benchmark.github.io/benchmark-app/?project=CoreOS-Application-Test)

### Example Application (`core-os-exmaple-app/`)

Reference implementation demonstrating library usage:

- **Main App** - `main_app.h` - Application entry point
- **CLI Integration** - `main_app_cli.h` - Command-line interface example


## 📚 Architecture

CoreOS follows a layered architecture:

```
┌─────────────────────────────────────┐
│    Applications & Example Apps      │
├─────────────────────────────────────┤
│         Core OS Library             │
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

