# Core OS Library (`core-os/`)

This directory contains the base CoreOS layer and all reusable modules.

## Root-Level Core Files

- `mla_data_types.h/.cpp` - Core data types and platform abstraction macros
- `mla_pointer.cpp` - Pointer management entry points
- `mla_default_pointer_memory_manager.cpp` - Default allocation strategy
- `mla_noop_pointer_memory_manager.cpp` - No-op allocator strategy
- `mla_native_resource_pointer_memory_manager.cpp` - Native-resource-aware pointer manager
- `mla_global_config.h` - Global configuration switches/constants

## Module Directories

- `system/` - Strings, buffers, streams, collections, ids, bytes ([readme](system/readme.md))
- `task/` - Task scheduling and synchronization ([readme](task/readme.md))
- `network/`, `http/`, `url/` - Networking protocol and URL modules ([network](network/readme.md), [http](http/readme.md), [url](url/readme.md))
- `rpc/` - RPC registry + HTTP transport ([readme](rpc/readme.md))
- `serializer/` - JSON/Binary/XML serialization ([readme](serializer/readme.md))
- `config/` - Persistent configuration support ([readme](config/readme.md))
- `filesystem/` - File system abstraction and helpers ([readme](filesystem/readme.md))
- `lifecycle/` - Boot event registration ([readme](lifecycle/readme.md))
- `log/` - Logging framework ([readme](log/readme.md))
- `memory/` - Allocation hook module ([readme](memory/readme.md))
- `hash/` - SHA-1 and hash helpers ([readme](hash/readme.md))
- `utils/` - Character/endian/math utilities ([readme](utils/readme.md))
- `reflection/` - Runtime metadata and reflection RPC helpers ([readme](reflection/readme.md))
- `ui/` - UI controls, surfaces, display integration, web remote surface ([readme](ui/readme.md))
- `build/` - Build/toolchain notes ([readme](build/readme.md))
