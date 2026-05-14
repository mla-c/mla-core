# CoreOS Test Suite

`core-os-test/` contains the test header set consumed by `main_test.h`.

## Available Test Files

### Core and System

- `mla_data_types_test.h`
- `mla_buffer_test.h`
- `mla_string_test.h`
- `native_string_test.h`
- `mla_number_test.h`
- `mla_bytes_test.h`
- `mla_id_test.h`
- `mla_pointer_test.h`
- `mla_user_data_test.h`

### Collections

- `mla_array_list_test.h`
- `mla_link_list_test.h`
- `mla_hash_map_test.h`
- `native_list_test.h`
- `mla_list_contains_const.h`

### Concurrency and Tasks

- `mla_atomic_test.h`
- `mla_mutex_test.h`
- `mla_rw_lock_test.h`
- `mla_task_manager_test.h`
- `mla_task_local_test.h`
- `mla_task_cli_module_test.h`

### Serialization and Streams

- `mla_serializer_test.h`
- `mla_stream_test.h`
- `mla_stream_deflate_test.h`

### Networking, HTTP, RPC, URL

- `mla_network_test.h`
- `mla_http_client_test.h`
- `mla_http_server_test.h`
- `mla_http_header_test.h`
- `mla_http_chunked_stream_test.h`
- `mla_websocket_client_test.h`
- `mla_url_test.h`
- `mla_rpc_test.h`
- `mla_http_rpc_test.h`

### CLI, Config, Logging, Reflection, and Other Modules

- `mla_cli_app_test.h`
- `mla_cli_parser_test.h`
- `mla_config_test.h`
- `mla_logger_console_test.h`
- `mla_logger_rpc_test.h`
- `mla_reflection_test.h`
- `mla_reflection_rpc_test.h`
- `mla_file_system_test.h`
- `mla_hash_test.h`
- `mla_sha1_test.h`
- `mla_math_utils_test.h`
- `mla_memory_hook_test.h`
- `mla_memory_test.h`

## Entry Point

- `main_test.h` aggregates and runs registered tests.

## Running

Tests are executed via a host application that includes platform headers, test-support platform adapters, and `main_test.h`.
See [core-os-test-support/readme.md](../core-os-test-support/readme.md) and `ai/instructions/build_project.instructions.md`.
