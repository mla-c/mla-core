# ESP-IDF Platform Module

The ESP-IDF platform module provides platform-specific implementations and configurations for running CoreOS on ESP32 and ESP8266 microcontrollers. It integrates with the Espressif IoT Development Framework (ESP-IDF) and provides optimized implementations for embedded systems.

## Architecture

The ESP-IDF platform module implements the following components:

- **ESP32 Platform** (`mla_global_platform_esp32.h`): ESP32-specific platform initialization and utilities
- **ESP8266 Platform** (`mla_global_platform_esp8266.h`): ESP8266-specific platform initialization and utilities
- **Network Operations** (`mla_global_network_esp32.h`): ESP32 Wi-Fi and networking implementations
- **File System** (`mla_global_file_system_esp32.h`): SPIFFS/LittleFS file system support
- **Task Management** (`mla_global_platform_task_manager_esp32.h`, `mla_global_platform_task_manager_esp8266.h`): FreeRTOS-based task scheduling
- **Configuration** (`mla_global_config_esp32.h`): Platform-specific configuration for ESP microcontrollers

## Features

### Fast Float Parsing

The ESP platform supports the fast-float library for optimized floating-point number parsing, which is approximately 40% faster than standard library implementations. This feature can be enabled via the `mla_use_fast_float` configuration flag.

```cpp
// When enabled, these platform functions use fast-float
#define mla_platform_strtod __fast_float_strtod
#define mla_platform_strtoll __fast_float_strtoll
#define mla_platform_strtoull __fast_float_strtoull
```

Note: Enabling fast-float increases code size by approximately 15KB, which should be considered on memory-constrained devices.

### Sleep Implementation

The ESP platform provides sleep functionality using FreeRTOS task delay:

```cpp
void __esp32_sleep(mla_uint32_t milliseconds) {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}
```

### Network Implementation

The ESP platform integrates with ESP32's Wi-Fi and networking stack:
- Wi-Fi station and access point modes
- TCP and UDP sockets via lwIP (lightweight IP)
- DNS resolution
- Support for both IPv4 and IPv6

### File System Implementation

The ESP platform supports multiple file system options:
- **SPIFFS**: Simple file system for flash storage
- **LittleFS**: Resilient file system for embedded devices
- **FAT**: For SD card support
- Virtual file system (VFS) integration

### Task Management

ESP task management leverages FreeRTOS:
- Native FreeRTOS task creation and scheduling
- Priority-based task execution
- Task notifications and synchronization
- Watchdog timer integration for system stability

### Watchdog Integration

The ESP32 implementation includes watchdog timer support to ensure system stability:

```cpp
#include <esp_task_wdt.h>
// Watchdog is configured per task to prevent system hangs
```

## Usage

### Including the Platform Headers

To use the ESP-IDF platform, include the appropriate platform header based on your target:

```cpp
// For ESP32
#include "core-os-platform/espidf/mla_global_platform_esp32.h"

// For ESP8266
#include "core-os-platform/espidf/mla_global_platform_esp8266.h"
```

### Configuration

The platform configuration can be customized via `mla_global_config_esp32.h`. This includes:
- Memory allocation strategies (considering limited RAM)
- Fast float parsing enablement (consider code size impact)
- Task manager FreeRTOS configuration
- Network buffer sizes (optimized for ESP constraints)
- Flash file system settings

### Building for ESP32/ESP8266

When building CoreOS for ESP platforms, use the ESP-IDF build system:

```bash
# Configure for ESP32
idf.py set-target esp32

# Or for ESP8266
idf.py set-target esp8266

# Build
idf.py build

# Flash to device
idf.py flash
```

## Platform-Specific Notes

### Dependencies

The ESP-IDF platform requires:
- ESP-IDF v4.0 or later (v4.4+ recommended for ESP32)
- ESP8266 RTOS SDK v3.0 or later (for ESP8266)
- FreeRTOS (included with ESP-IDF)
- lwIP networking stack (included with ESP-IDF)

### Memory Considerations

ESP32 and ESP8266 have limited RAM:
- **ESP32**: ~520KB SRAM (varies by model)
- **ESP8266**: ~80KB SRAM

Optimize your application by:
- Minimizing dynamic memory allocation
- Using PSRAM on ESP32 models that support it
- Carefully managing task stack sizes
- Considering fast-float code size impact (15KB)

### FreeRTOS Integration

CoreOS task management integrates with FreeRTOS:
- CoreOS tasks map to FreeRTOS tasks
- Use FreeRTOS APIs for synchronization if needed
- Configure task priorities appropriately
- Monitor stack usage to prevent overflow

### Network Setup

Before using network features, initialize Wi-Fi:

```cpp
// Initialize Wi-Fi and connect to network
// Then CoreOS network module can use the connection
```

### File System Setup

Before using file system features, mount the appropriate file system:

```cpp
// Mount SPIFFS or LittleFS
// Then CoreOS file system module can access files
```

### Watchdog Considerations

The ESP32 has a task watchdog timer. Long-running operations should:
- Feed the watchdog periodically
- Use appropriate task yields
- Configure watchdog timeout based on task requirements

### Performance Considerations

- Use DMA for high-speed data transfers
- Leverage ESP32's dual-core capabilities where available
- Optimize network buffer sizes for your application
- Consider using external flash for code storage (XIP)
- Profile memory usage to avoid heap fragmentation

## Integration with Other Modules

The ESP-IDF platform module works with CoreOS modules, providing implementations for:
- Network module (ESP32 Wi-Fi and lwIP sockets)
- File System module (SPIFFS/LittleFS)
- Task module (FreeRTOS-based scheduling)
- HTTP module (lightweight server and client for IoT)
- All other modules adapted for embedded constraints

## Supported Devices

This platform module supports:
- **ESP32** family: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6
- **ESP8266** series (with limitations due to lower resources)

Different ESP32 variants may have different capabilities (Wi-Fi, Bluetooth, cores, RAM, etc.). Consult your specific chip's documentation for details.
