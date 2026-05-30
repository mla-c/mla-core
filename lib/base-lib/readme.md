# base-lib

`base-lib` contains the reusable mla-c library layers: the platform-independent core, platform implementations, and test-support utilities.

## Structure

### Core (`core/`)

Main library modules and APIs. Entry documentation: [core/readme.md](core/readme.md)

- **System** (`system/`) — strings, buffers, collections, streams, numbers, references  
  [Documentation](core/system/readme.md)
- **Task Management** (`task/`) — task manager, task abstraction, mutex, rw-lock, CLI integration  
  [Documentation](core/task/readme.md)
- **HTTP / Network / URL** (`http/`, `network/`, `url/`) — client/server, headers, sockets, URL parsing  
  [HTTP](core/http/readme.md), [Network](core/network/readme.md), [URL](core/url/readme.md)
- **RPC** (`rpc/`) — RPC core + HTTP integration  
  [Documentation](core/rpc/readme.md)
- **Lifecycle** (`lifecycle/`) — prioritized boot lifecycle events  
  [Documentation](core/lifecycle/readme.md)
- **Serialization** (`serializer/`) — JSON + binary serializers  
  [Documentation](core/serializer/readme.md)
- **Logging** (`log/`) — logger, console logger, log levels  
  [Documentation](core/log/readme.md)
- **CLI** (`cli/`) — app framework, parser, command model  
  [Documentation](core/cli/readme.md)
- **Configuration** (`config/`) — configuration management  
  [Documentation](core/config/readme.md)
- **Dependency Injection** (`inject/`) — injector + service registry
- **Filesystem** (`filesystem/`) — file and directory abstraction  
  [Documentation](core/filesystem/readme.md)
- **Memory** (`memory/`) — memory hooks and allocation integration  
  [Documentation](core/memory/readme.md)
- **Utilities / Hash** (`utils/`, `hash/`) — character/endian helpers and hashing  
  [Utils](core/utils/readme.md), [Hash](core/hash/readme.md)
- **UI** (`ui/`) — controls, surfaces, web embedding, UI RPC integration  
  [Documentation](core/ui/readme.md)
- **Reflection** (`reflection/`) — runtime metadata + RPC reflection integration  
  [Documentation](core/reflection/readme.md)
- **Data Types** (`mla_data_types.h`) — cross-platform type definitions

### Platform (`platform/`)

Platform-specific integrations and abstractions:

- [generic](platform/generic/readme.md)
- [windows](platform/windows/readme.md)
- [linux](platform/linux/readme.md)
- [wasm](platform/wasm/readme.md)
- [raspberry](platform/raspberry/readme.md)
- [espidf](platform/espidf/readme.md)

### Test Support (`test-support/`)

Testing and benchmarking support library used by test suites.  
[Documentation](test-support/readme.md)

## Coding Style Notes (mla-c)

- Use MLA data types from `core/mla_data_types.h` in module APIs.
- Follow `mla_<module>_<action>` naming and `_t` type suffixes.
- Prefer references and `const` correctness for read-only parameters.
- Use MLA low-level wrappers for memory/string ops (`mla_memcpy`, `mla_memset`, `mla_strlen`, ...).
- Use `mla_pointer_t` for owned heap memory and `mla_platform_pointer_t` only for transient/raw access.

