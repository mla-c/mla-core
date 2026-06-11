# Generic Platform Module

The Generic platform module provides platform-agnostic implementations and utilities that can be used across different platforms. It serves as a foundation for platform-specific modules and includes common functionality that doesn't require platform-specific implementations.

## Architecture

The Generic platform module provides:

- **Generic Platform Utilities** (`mla_global_platform_generic.h`): Common platform initialization and helper functions
- **Fast Float Parsing** (`mla_global_platform_fast_float.h`): Optimized floating-point parsing implementation
- **Fast Float Library** (`fast_float.h`): Header-only fast-float library for number parsing
- **Pthread Task Manager** (`mla_task_manager_pthread.h`): POSIX thread-based task management
- **Single-Threaded Task Manager** (`mla_task_manager_single_thread.h`): Cooperative task scheduling for single-threaded environments
- **In-Memory Configuration** (`mla_global_config_inmemory.h`): Configuration for memory-only operation

## Features

### Fast Float Parsing

The generic module includes the fast-float library, which provides significantly faster floating-point parsing compared to standard library functions:

```cpp
#include "mla_global_platform_fast_float.h"

// Parse double with fast-float
double __fast_float_strtod(const char* str, char** endptr);

// Parse long long
long long __fast_float_strtoll(const char* str, char** endptr, int base);

// Parse unsigned long long
unsigned long long __fast_float_strtoull(const char* str, char** endptr, int base);
```

**Performance**: Approximately 40% faster than standard library implementations  
**Trade-off**: Increases code size by about 15KB

### Task Manager Implementations

The generic module provides two task manager implementations that can be used on any platform:

#### Pthread-Based Task Manager

For multi-threaded platforms with POSIX thread support:

```cpp
#include "mla_task_manager_pthread.h"

// Provides multi-threaded task execution
// Suitable for: Linux, macOS, other POSIX systems
```

Features:
- Thread pool management
- Priority-based task scheduling
- Thread-safe task queuing
- Efficient CPU utilization

#### Single-Threaded Task Manager

For single-threaded or resource-constrained platforms:

```cpp
#include "mla_task_manager_single_thread.h"

// Provides cooperative multitasking
// Suitable for: Embedded systems, WASM, simple platforms
```

Features:
- Cooperative task scheduling
- Minimal memory overhead
- Deterministic execution
- No threading synchronization overhead

### Generic Platform Functions

Common utilities used across platforms:

```cpp
#include "mla_global_platform_generic.h"

// Generic implementations for:
// - String conversion
// - Number parsing
// - Basic utilities
// - Platform-independent operations
```

### In-Memory Configuration

For systems without persistent storage or testing:

```cpp
#include "mla_global_config_inmemory.h"

// Configuration stored in RAM only
// Ideal for: Testing, temporary operation, memory-only systems
```

## Usage

### Choosing a Task Manager

Select the appropriate task manager for your platform:

```cpp
// For multi-threaded platforms (Linux, Windows, etc.)
#include "mla_task_manager_pthread.h"

// For single-threaded platforms (WASM, simple embedded)
#include "mla_task_manager_single_thread.h"
```

### Enabling Fast Float Parsing

Platform-specific headers typically include fast-float conditionally:

```cpp
#if mla_use_fast_float == 1
    #include "mla_global_platform_fast_float.h"
    #define mla_platform_strtod __fast_float_strtod
#else
    #define mla_platform_strtod mla_internal_generic_strtod
#endif
```

### Using Generic Utilities

Generic utilities are typically included by platform-specific headers:

```cpp
#include "mla_global_platform_generic.h"
// Provides fallback implementations for common operations
```

## Platform-Specific Notes

### When to Use Generic Module

Use the generic module:
- As a foundation for new platform implementations
- For testing without platform-specific dependencies
- When porting to a new platform
- For platform-agnostic libraries

### Task Manager Selection Guide

| Platform | Recommended Task Manager | Reason |
|----------|-------------------------|---------|
| Linux/macOS | pthread | Full threading support |
| Windows | pthread (or native) | POSIX compatibility layer |
| ESP32 | FreeRTOS | Native RTOS integration |
| ESP8266 | Single-threaded | Limited resources |
| Raspberry Pi Pico | Single-threaded or FreeRTOS | Depends on requirements |
| WebAssembly | Single-threaded | Browser limitation |

### Fast Float Considerations

Enable fast-float when:
- Parsing many floating-point numbers
- Performance is critical
- 15KB code size is acceptable

Disable fast-float when:
- Code size is constrained
- Minimal parsing required
- Using standard library preferred

## Integration with Platform-Specific Modules

Platform-specific modules build on the generic module:

```cpp
// Example: Linux platform header
#include "../generic/mla_global_platform_generic.h"

#if mla_use_fast_float == 1
    #include "../generic/mla_global_platform_fast_float.h"
    #define mla_platform_strtod __fast_float_strtod
#else
    #define mla_platform_strtod mla_internal_generic_strtod
#endif

// Add Linux-specific implementations...
```

## Creating New Platform Implementations

To create a new platform module:

1. **Include generic headers**:
   ```cpp
   #include "../generic/mla_global_platform_generic.h"
   ```

2. **Select task manager**:
   ```cpp
   #include "../generic/mla_task_manager_pthread.h"
   // or
   #include "../generic/mla_task_manager_single_thread.h"
   ```

3. **Configure fast-float**:
   ```cpp
   #if mla_use_fast_float == 1
       #include "../generic/mla_global_platform_fast_float.h"
   #endif
   ```

4. **Implement platform-specific features**:
   - Network operations
   - File system access
   - Sleep/timing functions
   - Platform initialization

5. **Define platform macros**:
   ```cpp
   #define mla_platform_strtod __fast_float_strtod
   #define mla_platform_sleep __your_platform_sleep
   // etc.
   ```

## Advantages of Generic Module

- **Code Reuse**: Common code shared across platforms
- **Consistency**: Same behavior on different platforms
- **Testing**: Easy to test platform-agnostic code
- **Portability**: Simplifies porting to new platforms
- **Maintenance**: Single location for common functionality

## Components Overview

### Fast Float Library

- **Source**: [github.com/fastfloat/fast_float](https://github.com/fastfloat/fast_float)
- **Type**: Header-only C++ library
- **License**: Apache 2.0 / MIT (permissive)
- **Performance**: 2-4x faster than standard parsing

### Task Managers

Both task managers implement the same interface, allowing platform-independent code:

```cpp
// Same API regardless of implementation
mla_task_manager_register_task(task);
mla_task_manager_process_all_tasks();
```

### Generic Platform Header

Provides fallback implementations for:
- String to number conversions
- Memory operations
- Basic type conversions
- Platform-independent utilities

This ensures code works even without platform-specific optimizations.
