# Lifecycle Module

The Lifecycle module provides a simple yet powerful mechanism for managing boot-time events. It allows you to register callbacks that will be executed in a specific order during the application's startup sequence.

## Architecture

The Lifecycle module is based on a priority system. Each registered event is assigned a priority, and events with lower priority values are executed before events with higher priority values. The module defines a set of predefined priority levels for common system components, making it easy to ensure that your callbacks are executed at the correct time.

### Key Functions

- **`mla_lifecycle_boot_event_register`**: Registers a boot event callback with a specific priority.
- **`mla_lifecycle_fire_boot_events`**: Executes all registered boot event callbacks in order of priority.
- **`mla_lifecycle_boot_event_static_register`**: A macro that provides a convenient way to register a boot event callback from anywhere in your code. This is the recommended way to register boot events.

## Usage

To use the Lifecycle module, you simply need to register your boot event callbacks using the `mla_lifecycle_boot_event_static_register` macro.

### Example

Here's an example of how to register a boot event callback that initializes a custom module:

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