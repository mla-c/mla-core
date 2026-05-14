# CoreOS Library Collection

CoreOS is a cross-platform C++ framework with OS abstractions, networking, serialization, RPC, UI, and testing support.

## Repository Layout

- `core-os/` - Main CoreOS library modules ([docs](core-os/readme.md))
- `core-os-platform/` - Platform-specific integrations ([linux](core-os-platform/linux/readme.md), [windows](core-os-platform/windows/readme.md), [wasm](core-os-platform/wasm/readme.md), [raspberry](core-os-platform/raspberry/readme.md), [espidf](core-os-platform/espidf/readme.md), [generic](core-os-platform/generic/readme.md))
- `core-os-test/` - Unit/integration test definitions ([docs](core-os-test/readme.md))
- `core-os-test-support/` - Test/benchmark runtime utilities ([docs](core-os-test-support/readme.md))
- `core-os-exmaple-app/` - Example application ([docs](core-os-exmaple-app/readme.md))
- `core-os-web-ui/` - Preact/TypeScript web UI that generates embedded UI assets ([docs](core-os-web-ui/README.md))

## Core Modules (`core-os/`)

- **System** - Strings, streams, buffers, collections, ids, bytes ([docs](core-os/system/readme.md))
- **Task** - Task scheduling, mutex/rw-lock/atomic, task-local storage ([docs](core-os/task/readme.md))
- **Network/HTTP/URL** - Networking stack, HTTP client/server, WebSocket, URL parsing ([network](core-os/network/readme.md), [http](core-os/http/readme.md), [url](core-os/url/readme.md))
- **RPC** - Local/remote procedure registry with HTTP transport ([docs](core-os/rpc/readme.md))
- **Serializer** - JSON, binary, XML serializers ([docs](core-os/serializer/readme.md))
- **Filesystem** - Virtualized and native file system interfaces ([docs](core-os/filesystem/readme.md))
- **Config** - Structured config read/write using serializer definitions ([docs](core-os/config/readme.md))
- **Lifecycle** - Startup event registration and dispatch ([docs](core-os/lifecycle/readme.md))
- **Log** - Logger manager, console logger, RPC logger ([docs](core-os/log/readme.md))
- **Memory** - Allocation hook chain ([docs](core-os/memory/readme.md))
- **Hash/Utils** - SHA-1, integer hash helpers, endian/math/char utils ([hash](core-os/hash/readme.md), [utils](core-os/utils/readme.md))
- **Reflection** - Runtime struct metadata + RPC reflection helpers ([docs](core-os/reflection/readme.md))
- **UI** - Control tree + draw surfaces + web remote surface ([docs](core-os/ui/readme.md))
- **Base Types** - Core data types and pointer memory managers (`mla_data_types.*`, `mla_pointer.*`)

## Testing

`core-os-test/` currently includes test headers such as:

- System/tests: `mla_string_test.h`, `mla_buffer_test.h`, `mla_number_test.h`, `mla_stream_test.h`, `mla_bytes_test.h`, `mla_id_test.h`, `mla_pointer_test.h`, `mla_user_data_test.h`, `native_string_test.h`, `native_list_test.h`
- Collections/concurrency: `mla_array_list_test.h`, `mla_link_list_test.h`, `mla_hash_map_test.h`, `mla_atomic_test.h`, `mla_mutex_test.h`, `mla_rw_lock_test.h`, `mla_task_manager_test.h`, `mla_task_local_test.h`, `mla_task_cli_module_test.h`
- Network/protocol: `mla_network_test.h`, `mla_http_client_test.h`, `mla_http_server_test.h`, `mla_http_header_test.h`, `mla_http_chunked_stream_test.h`, `mla_websocket_client_test.h`, `mla_url_test.h`, `mla_rpc_test.h`, `mla_http_rpc_test.h`
- Other modules: `mla_serializer_test.h`, `mla_config_test.h`, `mla_cli_app_test.h`, `mla_cli_parser_test.h`, `mla_reflection_test.h`, `mla_reflection_rpc_test.h`, `mla_file_system_test.h`, `mla_hash_test.h`, `mla_sha1_test.h`, `mla_math_utils_test.h`, `mla_memory_hook_test.h`, `mla_memory_test.h`, `mla_logger_console_test.h`, `mla_logger_rpc_test.h`, `mla_data_types_test.h`

See full details in [core-os-test/readme.md](core-os-test/readme.md).

## Web UI

The `core-os-web-ui/` build outputs an embedded C header:

- Generated file: `core-os/ui/web/mla_ui_web_embedded.h`
- Build command: `npm run build` (inside `core-os-web-ui/`)

## Notes

- Source groups used by external CMake integrations are listed in `sources.cmake`.
- CoreOS can be integrated into host projects with their own build systems/toolchains.
