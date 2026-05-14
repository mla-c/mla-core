# Lifecycle Module

The Lifecycle module provides a simple yet powerful mechanism for managing boot-time events. It allows you to register callbacks that will be executed in a specific order during the application's startup sequence.

## Architecture

The Lifecycle module is based on a priority system. Each registered event is assigned a priority, and events with lower priority values are executed before events with higher priority values. The module defines a set of predefined priority levels for common system components, making it easy to ensure that your callbacks are executed at the correct time.

### Key Functions

- **`mla_lifecycle_boot_event_register`**: Registers a boot event callback with a specific priority.
- **`mla_lifecycle_fire_boot_events`**: Executes all registered boot event callbacks in order of priority.

## Usage

### Registering Boot Events

The recommended way to register a boot event is to use the `mla_lifecycle_boot_event_static_register` macro. This macro creates a static object that automatically registers your callback at program startup.

### `mla_lifecycle_boot_event_static_register(priority, callback_name)`

- **`priority`**: The priority of the boot event. The module provides a set of predefined priorities (e.g., `mla_lifecycle_boot_event_priority_low_level_setup`, `mla_lifecycle_boot_event_priority_application_setup`) to help you order your events correctly.
- **`callback_name`**: The name of the function to be called when the boot event is fired.

Example:
```cpp
#include "mla_lifecycle_events.h"

// Define your custom module's initialization function
void my_module_init() {
    // Initialization code for your module
}

// Register the initialization function as a boot event
mla_lifecycle_boot_event_static_register(
    mla_lifecycle_boot_event_priority_application_setup,
    my_module_init
);
```

In this example, the `my_module_init` function will be called during the application setup phase, as defined by the `mla_lifecycle_boot_event_priority_application_setup` priority.

### Firing Boot Events

The `mla_lifecycle_fire_boot_events` function is typically called once at the beginning of your application's `main` function to execute all registered boot events.

```cpp
#include "mla_lifecycle_events.h"

int main() {
    // Fire all registered boot events
    mla_lifecycle_fire_boot_events();

    // Continue with your application's main loop
    // ...
}
```

## Project Conventions (Latest)

This module follows the current mla-c project-wide conventions:

- **Code style:** Use MLA data types and naming conventions described in the [main README](../../README.md).
- **Heap ownership:** Use `mla_pointer_t` for owned heap allocations; use raw platform pointers only for transient access.

For full details, see:
- [Main project README](../../README.md)
- [Core data types documentation](../../core/readme.md)
- [Memory module documentation](../../core/memory/readme.md)

