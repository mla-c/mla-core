# Raspberry Pi Pico Platform Module

The Raspberry Pi Pico platform module provides platform-specific implementations and configurations for running mla-c on Raspberry Pi Pico and other RP2040-based microcontrollers. It integrates with the Pico SDK and provides optimized implementations for this ARM Cortex-M0+ based platform.

## Architecture

The Raspberry Pi Pico platform module implements the following components:

- **Platform Initialization** (`mla_global_platform_raspberry-pico.h`): RP2040-specific platform setup and utilities
- **Network Operations** (`mla_global_network_raspberry-pico.h`): Network implementations for Pico W (Wi-Fi model)
- **File System** (`mla_global_file_system_raspberry-pico.h`): Flash-based file system support
- **Task Management** (`mla_global_platform_task_manager_raspberry-pico.h`): Task scheduling for RP2040
- **Configuration** (`mla_global_config_raspberry-pico.h`): Platform-specific configuration settings

## Features

### Dual-Core Support

The RP2040 features dual ARM Cortex-M0+ cores running at up to 133MHz. The platform module can leverage both cores for parallel task execution.

### Sleep Implementation

The Pico platform provides sleep functionality optimized for the RP2040:

```cpp
// Sleep implementation uses Pico SDK timing functions
// for accurate delays and low-power sleep modes
```

### Network Implementation (Pico W)

For Raspberry Pi Pico W models with Wi-Fi:
- CYW43 Wi-Fi chip integration
- TCP and UDP socket support via lwIP
- Wi-Fi station mode
- DNS resolution

Note: Standard Pico models without Wi-Fi can use external network modules via SPI/UART.

### File System Implementation

The Pico platform supports:
- **LittleFS**: Resilient file system for internal flash
- **FAT**: For external SD card storage
- Flash storage optimization for RP2040's 2MB+ flash

### Task Management

Pico task management can use:
- Custom lightweight scheduler for single-core operation
- FreeRTOS port for RP2040 (if enabled)
- Dual-core task distribution
- Efficient context switching optimized for Cortex-M0+

### Hardware Features

The platform module provides access to RP2040-specific features:
- Programmable I/O (PIO) state machines
- DMA controllers
- USB device and host support
- ADC for analog inputs
- PWM outputs
- I2C, SPI, UART peripherals

## Usage

### Including the Platform Headers

To use the Raspberry Pi Pico platform, include the main platform header:

```cpp
#include "core-platform/raspberry/mla_global_platform_raspberry-pico.h"
```

### Configuration

The platform configuration can be customized via `mla_global_config_raspberry-pico.h`. This includes:
- Memory allocation strategies (264KB SRAM available)
- Task manager configuration
- Network buffer sizes (for Pico W)
- Flash file system settings
- Clock frequency configuration

### Building for Raspberry Pi Pico

When building mla-c for Raspberry Pi Pico, use the Pico SDK build system:

```bash
# Set Pico SDK path
export PICO_SDK_PATH=/path/to/pico-sdk

# Configure CMake
cmake -DPLATFORM=raspberry-pico ..

# Build
make

# Flash to Pico (copy UF2 file to mounted Pico drive)
cp your_program.uf2 /media/RPI-RP2/
```

## Platform-Specific Notes

### Dependencies

The Raspberry Pi Pico platform requires:
- Pico SDK 1.3.0 or later
- CMake 3.13 or later
- ARM GCC toolchain (arm-none-eabi)
- Optional: FreeRTOS (if using RTOS-based task management)

### Memory Considerations

The RP2040 has 264KB of on-chip SRAM and 2MB+ of flash:
- Optimize code size to fit in flash
- Manage SRAM carefully for data and stack
- Consider using flash for read-only data
- Use DMA to reduce CPU load for data transfers

### Dual-Core Optimization

Leverage both cores for better performance:
- Run network/I/O on one core
- Run application logic on the other core
- Use inter-core FIFOs for communication
- Synchronize shared resources with spinlocks

### USB Support

The RP2040 has native USB 1.1 support:
- USB device mode for serial communication
- Can be used as virtual COM port
- USB host mode for peripherals (experimental)

### Clock Configuration

Configure system clock based on requirements:
- Default: 125 MHz
- Can overclock up to 133 MHz (or higher with caution)
- Adjust for power vs. performance trade-offs

### Peripheral Integration

Access RP2040 peripherals through the platform:
- Use PIO for custom protocols
- Configure DMA for efficient transfers
- Set up PWM for motor control, LED dimming, etc.

### Performance Considerations

- Utilize both CPU cores for parallel tasks
- Use DMA for SPI/I2C/UART transfers
- Optimize flash access patterns (XIP cache)
- Consider clock speed vs. power consumption
- Use PIO state machines for time-critical operations

### Power Management

The Pico platform supports various power modes:
- Active mode: Full-speed operation
- Sleep mode: Reduced power with quick wake
- Dormant mode: Lowest power, slower wake

## Integration with Other Modules

The Raspberry Pi Pico platform module works with mla-c modules:
- Network module (Pico W Wi-Fi or external modules)
- File System module (LittleFS on flash, FAT on SD)
- Task module (lightweight or FreeRTOS-based)
- HTTP module (lightweight server for IoT)
- All other modules adapted for Cortex-M0+ constraints

## Supported Boards

This platform module supports:
- **Raspberry Pi Pico**: Basic RP2040 board
- **Raspberry Pi Pico W**: With CYW43 Wi-Fi and Bluetooth
- **Raspberry Pi Pico H**: With pre-soldered headers
- **Third-party RP2040 boards**: Various designs from other manufacturers

Ensure your board's specific features (Wi-Fi, extra flash, etc.) are properly configured in the build.

## Debug and Development

The Pico platform supports multiple debugging options:
- USB serial output for logging
- SWD debugging with probe (e.g., Raspberry Pi Debug Probe)
- PIO-based logic analyzer functionality
- Built-in temperature sensor for monitoring

## Project Conventions (Latest)

This module follows the current mla-c project-wide conventions:

- **Code style:** Use MLA data types and naming conventions described in the [main README](../../README.md).
- **Heap ownership:** Use `mla_pointer_t` for owned heap allocations; use raw platform pointers only for transient access.

For full details, see:
- [Main project README](../../README.md)
- [Core data types documentation](../../core/readme.md)
- [Memory module documentation](../../core/memory/readme.md)

