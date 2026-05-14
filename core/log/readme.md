# Log Module

The Log module provides a flexible and extensible logging framework for your application. It allows you to register multiple loggers, each with its own log level and output writer, and then use a simple set of macros to write log messages.

## Architecture

The Log module is built around a central logger manager that maintains a list of registered loggers. Each logger has a name, a log level, and a writer function. When a log message is written, the logger manager iterates through the registered loggers and, if the message's log level is high enough for a given logger, it calls that logger's writer function to output the message.

### Key Components

- **`mla_logger_t`**: A struct that represents a logger, containing its name, log level, and writer function.
- **`mla_log_register_logger`**: Registers a new logger with the logger manager.
- **`mla_log_*` macros**: A set of macros that provide a simple and convenient way to write log messages.

### Console Logger

The Log module includes a pre-built console logger that writes log messages to the standard output. You can activate and deactivate the console logger and set its log level using the `mla_log_to_console_*` functions.

## Usage

### Activating the Console Logger

To start logging to the console, you first need to activate the console logger.

```cpp
#include "mla_logger_console.h"

// Activate the console logger
mla_log_to_console_activate();

// Set the log level for the console logger
mla_log_to_console_set_level(MLA_LOG_LEVEL_DEBUG);
```

### Logging Macros

The `mla_logging.h` header provides a set of convenient macros for logging messages at different severity levels. These macros automatically capture the filename and function name where they are called, providing valuable context for your log messages.

- **`mla_verbose(msg)`**: Logs a message with the `MLA_LOG_LEVEL_VERBOSE` level. Useful for detailed, fine-grained logging information.
- **`mla_debug(msg)`**: Logs a message with the `MLA_LOG_LEVEL_DEBUG` level. Intended for debugging information that is useful during development.
- **`mla_info(msg)`**: Logs a message with the `MLA_LOG_LEVEL_INFO` level. Used for informational messages that highlight the progress of the application.
- **`mla_warning(msg)`**: Logs a message with the `MLA_LOG_LEVEL_WARNING` level. Indicates a potential problem or a situation that might require attention.
- **`mla_error(msg)`**: Logs a message with the `MLA_LOG_LEVEL_ERROR` level. Used for serious errors that prevent the application from performing a specific operation.
- **`mla_log_msg(level, msg)`**: A generic logging macro that allows you to specify the log level dynamically.

Example:
```cpp
#include "mla_logging.h"

void process_data(data_t* data) {
    if (data == nullptr) {
        mla_error("Input data is null!");
        return;
    }
    mla_info("Processing data...");
    // ...
    mla_debug("Data processing complete.");
}
```

### Creating a Custom Logger

You can create your own custom loggers by defining a writer function and registering it with the logger manager.

```cpp
#include "mla_logger.h"

// Define a custom log writer function
void my_log_writer(const mla_log_level level, mla_string_t& message, mla_string_t& context, const mla_callback_userdata userData) {
    // Write the log message to a file, a network socket, etc.
}

// Create and register the custom logger
mla_logger_t my_logger = {
    .name = mla_string_const("MyLogger"),
    .level = MLA_LOG_LEVEL_INFO,
    .write = my_log_writer,
    .userData = 0
};
mla_log_register_logger(my_logger);
```

## Project Conventions (Latest)

This module follows the current mla-c project-wide conventions:

- **Code style:** Use MLA data types and naming conventions described in the [main README](../../README.md).
- **Heap ownership:** Use `mla_pointer_t` for owned heap allocations; use raw platform pointers only for transient access.

For full details, see:
- [Main project README](../../README.md)
- [Core data types documentation](../../core/readme.md)
- [Memory module documentation](../../core/memory/readme.md)

