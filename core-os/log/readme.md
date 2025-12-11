# Log Module

The Log module provides a flexible and extensible logging framework for your application. It allows you to register multiple loggers, each with its own log level and output writer, and then use a simple set of macros to write log messages.

## Architecture

The Log module is built around a central logger manager that maintains a list of registered loggers. Each logger has a name, a log level, and a writer function. When a log message is written, the logger manager iterates through the registered loggers and, if the message's log level is high enough for a given logger, it calls that logger's writer function to output the message.

### Key Components

- **`mla_logger_t`**: A struct that represents a logger, containing its name, log level, and writer function.
- **`mla_log_register_logger`**: Registers a new logger with the logger manager.
- **`mla_log_*` macros**: A set of macros (`mla_verbose`, `mla_debug`, `mla_info`, `mla_warning`, `mla_error`) that provide a simple and convenient way to write log messages.

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

### Writing Log Messages

Once a logger is active, you can use the logging macros to write messages.

```cpp
#include "mla_logging.h"

// Write an info message
mla_info("This is an informational message.");

// Write a debug message
mla_debug("This is a debug message.");
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