# mla-c Test Suite

The `core-test/` directory contains the complete set of unit and integration test headers for all mla-c modules. Each test file covers a specific module or component, ensuring comprehensive validation of the library's functionality.

## Architecture

Tests are defined as header files (`.h`) that register test cases using the mla-c test framework (see [Test Support Library](../base-lib/test-support/readme.md)). Tests use `mla_test_t` definitions with assert macros for validation.

## Test Categories

### Core Data Types and System

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_data_types_test.h` | Data Types | Platform-independent type definitions |
| `mla_buffer_test.h` | System | Dynamic byte buffer operations |
| `mla_string_test.h` | System | UTF-8 string handling and manipulation |
| `mla_native_string_test.h` | System | Native string interoperability |
| `mla_number_test.h` | System | Numeric type utilities and conversions |
| `mla_bytes_test.h` | System | Byte manipulation utilities |
| `mla_id_test.h` | System | ID generation |
| `mla_user_data_test.h` | System | User data container management |

### Collections

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_array_list_test.h` | System | Dynamic array list operations |
| `mla_link_list_test.h` | System | Linked list operations |
| `mla_hash_map_test.h` | System | Hash map operations |

### Concurrency

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_atomic_test.h` | Task | Atomic operations |
| `mla_mutex_test.h` | Task | Mutex locking and synchronization |
| `mla_rw_lock_test.h` | Task | Reader-writer lock |
| `mla_task_manager_test.h` | Task | Task scheduling and execution |
| `mla_task_local_test.h` | Task | Task-local storage |
| `mla_task_cli_module_test.h` | Task | CLI integration for task management |

### Serialization and Streams

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_serializer_test.h` | Serializer | General serialization interface |
| `mla_json_serializer_test.h` | Serializer | JSON serialization and deserialization |
| `mla_binary_serializer_test.h` | Serializer | Binary serialization and deserialization |
| `mla_xml_serializer_test.h` | Serializer | XML serialization and deserialization |
| `mla_stream_test.h` | System | Stream abstraction operations |
| `mla_stream_deflate_test.h` | System | Deflate compression stream |

### Networking and HTTP

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_http_client_test.h` | HTTP | HTTP client requests and responses |
| `mla_http_server_test.h` | HTTP | HTTP server handling and routing |
| `mla_http_header_test.h` | HTTP | HTTP header parsing and management |
| `mla_http_chunked_stream_test.h` | HTTP | Chunked transfer encoding |
| `mla_websocket_test.h` | HTTP | WebSocket protocol handling |
| `mla_url_test.h` | URL | URL parsing and manipulation |
| `mla_rpc_test.h` | RPC | Remote procedure call framework |

### CLI and Configuration

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_cli_app_test.h` | CLI | CLI application framework |
| `mla_cli_parser_test.h` | CLI | Command-line argument parsing |
| `mla_config_test.h` | Config | Configuration management |

### Reflection and Other

| Test File | Module | Description |
|-----------|--------|-------------|
| `mla_reflection_test.h` | Reflection | Runtime type information |
| `mla_reflection_rpc_test.h` | Reflection | Reflection RPC integration |
| `mla_file_system_test.h` | File System | File and directory operations |
| `mla_hash_test.h` | Hash | SHA-1 and integer hashing |
| `mla_math_utils_test.h` | Utils | Math utility functions |
| `mla_memory_hook_test.h` | Memory | Memory hook system |
| `mla_reference_test.h` | System | Reference counting |
| `mla_lifecycle_test.h` | Lifecycle | Boot event management |

### Integration

| Test File | Description |
|-----------|-------------|
| `main_test.h` | Main test entry point that aggregates all tests |

## Running Tests

Tests are compiled and linked with the test support library and a platform-specific main entry point. See the [Test Support Library](../base-lib/test-support/readme.md) for framework details and the build system for compilation instructions.
