# Linux Platform Module

The Linux platform module provides platform-specific implementations and configurations for running mla-c on Linux systems. It includes optimized implementations for networking, file system operations, task management, and low-level platform utilities.

## Architecture

The Linux platform module implements the following components:

- **Platform Initialization** (`mla_global_platform_linux.h`): Core platform setup and lifecycle management
- **Network Operations** (`mla_global_network_linux.h`): Linux-specific socket and network implementations
- **File System** (`mla_global_file_system_linux.h`): POSIX file system operations
- **Task Management** (`mla_global_platform_task_manager_linux.h`): Linux threading and task scheduling
- **Configuration** (`mla_global_config_linux.h`): Platform-specific configuration settings

## Features

### Fast Float Parsing

The Linux platform supports the fast-float library for optimized floating-point number parsing, which is approximately 40% faster than standard library implementations. This feature can be enabled via the `mla_use_fast_float` configuration flag.

```cpp
// When enabled, these platform functions use fast-float
#define mla_platform_strtod __fast_float_strtod
#define mla_platform_strtoll __fast_float_strtoll
#define mla_platform_strtoull __fast_float_strtoull
```

### Sleep Implementation

The Linux platform provides sleep functionality using the POSIX `usleep` function:

```cpp
void __linux_sleep(mla_uint32_t milliseconds);
```

### Network Implementation

The Linux platform uses POSIX sockets for network operations, providing:
- TCP and UDP socket support
- Non-blocking I/O
- Socket options configuration
- DNS resolution

### File System Implementation

The Linux platform leverages standard POSIX file system APIs for:
- File creation, reading, writing, and deletion
- Directory operations
- Path manipulation
- File metadata access

### Task Management

Linux task management is implemented using POSIX threads (pthreads) or single-threaded scheduling, depending on configuration. See the generic platform's `mla_task_manager_pthread.h` or `mla_task_manager_single_thread.h` for implementation details.

## Usage

### Including the Platform Headers

To use the Linux platform, include the main platform header in your application:

```cpp
#include "base-lib/core-platform/linux/mla_global_platform_linux.h"
```

### Configuration

The platform configuration can be customized via `mla_global_config_linux.h`. This includes:
- Memory allocation strategies
- Fast float parsing enablement
- Task manager configuration
- Network buffer sizes

### Building for Linux

When building mla-c for Linux, ensure that your build system defines the appropriate platform macro and links against the Linux platform implementation:

```bash
# Example CMake configuration
cmake -DPLATFORM=linux ..
make
```

## Platform-Specific Notes

### Dependencies

The Linux platform requires:
- POSIX-compliant operating system (Linux kernel 2.6+)
- pthreads library for multi-threaded task management
- Standard C library with socket support

### Thread Safety

The Linux platform implementation is thread-safe when using the pthread-based task manager. Ensure proper synchronization when accessing shared resources.

### Performance Considerations

- Enable fast-float parsing for applications with heavy floating-point parsing workloads
- Use non-blocking I/O for network operations to maximize throughput
- Configure task manager thread pool size based on workload and hardware capabilities

## Integration with Other Modules

The Linux platform module works seamlessly with all mla-c modules, providing the low-level implementations required for:
- Network module (TCP/UDP sockets)
- File System module (POSIX file operations)
- Task module (pthread-based scheduling)
- HTTP module (socket-based server and client)
- All other modules that depend on platform-specific functionality
