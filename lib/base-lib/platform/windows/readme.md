# Windows Platform Module

The Windows platform module provides platform-specific implementations and configurations for running mla-c on Windows systems. It includes native Windows implementations for networking, file system operations, task management, and low-level platform utilities.

## Architecture

The Windows platform module implements the following components:

- **Platform Initialization** (`mla_global_platform_windows.h`): Core platform setup and lifecycle management
- **Network Operations** (`mla_global_network_windows.h`): Windows Winsock-based network implementations
- **File System** (`mla_global_file_system_windows.h`): Windows file system API operations
- **Task Management** (`mla_global_platform_task_manager_windows.h`): Windows threading and task scheduling
- **Configuration** (`mla_global_config_windows.h`): Platform-specific configuration settings
- **UI Surface Renderers** (`mla_global_ui_surface_windows_direct2d.h`, `mla_global_ui_surface_windows_gdiplus.h`, `mla_global_ui_surface_windows_opengl.h`): Native and accelerated Windows UI rendering backends

## Features

- **GDI+ Renderer** - `mla_global_ui_surface_windows_gdiplus.h` provides a dedicated GDI+ renderer implementation for the Windows surface backend.

### Fast Float Parsing

The Windows platform supports the fast-float library for optimized floating-point number parsing, which is approximately 40% faster than standard library implementations. This feature can be enabled via the `mla_use_fast_float` configuration flag.

```cpp
// When enabled, these platform functions use fast-float
#define mla_platform_strtod __fast_float_strtod
#define mla_platform_strtoll __fast_float_strtoll
#define mla_platform_strtoull __fast_float_strtoull
```

### Sleep Implementation

The Windows platform provides sleep functionality using the Windows `Sleep` API:

```cpp
void mla_internal_windows_sleep(mla_uint32_t milliseconds);
```

### Network Implementation

The Windows platform uses Winsock (Windows Sockets API) for network operations, providing:
- TCP and UDP socket support
- Non-blocking I/O with I/O completion ports
- Socket options configuration
- DNS resolution via Windows networking APIs

### File System Implementation

The Windows platform leverages Windows file system APIs for:
- File creation, reading, writing, and deletion
- Directory operations with support for Windows path conventions
- Path manipulation (handling both forward and backward slashes)
- File metadata and attributes access
- Support for long file paths (> 260 characters)

### Task Management

Windows task management is implemented using native Windows threads and synchronization primitives, providing:
- Thread creation and management
- Priority-based scheduling
- Synchronization with mutexes, events, and critical sections
- Integration with Windows thread pool

## Usage

### Including the Platform Headers

To use the Windows platform, include the main platform header in your application:

```cpp
#include "lib/base-lib/platform/windows/mla_global_platform_windows.h"
```

### Configuration

The platform configuration can be customized via `mla_global_config_windows.h`. This includes:
- Memory allocation strategies
- Fast float parsing enablement
- Task manager configuration
- Network buffer sizes
- Winsock initialization settings

### Building for Windows

When building mla-c for Windows, ensure that your build system defines the appropriate platform macro and links against required Windows libraries:

```bash
# Example CMake configuration
cmake -DPLATFORM=windows ..
cmake --build .
```

Required libraries:
- `ws2_32.lib` (Winsock)
- `kernel32.lib` (Core Windows APIs)

## Platform-Specific Notes

### Dependencies

The Windows platform requires:
- Windows 7 or later (Windows 10+ recommended)
- Winsock 2.2 or later for network functionality
- Visual Studio 2017 or later (or compatible compiler with Windows SDK)

### Winsock Initialization

The Windows platform automatically initializes Winsock when the network module is first used. Ensure proper cleanup by calling the appropriate shutdown functions when terminating the application.

### Thread Safety

The Windows platform implementation is thread-safe when using the Windows thread-based task manager. Ensure proper synchronization when accessing shared resources using Windows synchronization primitives.

### Performance Considerations

- Enable fast-float parsing for applications with heavy floating-point parsing workloads
- Use I/O completion ports for high-performance network applications
- Configure task manager thread pool size based on workload and hardware capabilities
- Consider using Windows-specific optimizations like memory-mapped files for large file operations

### Path Handling

The Windows platform handles both Windows-style backslash (`\`) and Unix-style forward slash (`/`) path separators, normalizing them internally for consistent behavior.

## Integration with Other Modules

The Windows platform module works seamlessly with all mla-c modules, providing the low-level implementations required for:
- Network module (Winsock-based TCP/UDP sockets)
- File System module (Windows file system APIs)
- Task module (Windows thread-based scheduling)
- HTTP module (Winsock-based server and client)
- All other modules that depend on platform-specific functionality

## Compatibility

The Windows platform module is compatible with:
- Windows 7 / Server 2008 R2 and later
- Both 32-bit and 64-bit architectures
- Visual Studio 2017, 2019, 2022
- MinGW-w64 (with appropriate configuration)
- Clang on Windows (with MSVC or MinGW target)
