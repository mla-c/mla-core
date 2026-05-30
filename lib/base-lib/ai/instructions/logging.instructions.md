---
applyTo: '**'
description: 'Patterns for using the logging framework in the MLA framework'
---

# Logging

The logging module (`core/log/`) provides a structured, level-based logging system. Log messages are routed to one or more registered **loggers**, each with its own output destination and minimum log level.

## Log Levels

| Macro | Enum | Value | When to Use |
|---|---|---|---|
| `mla_verbose(msg)` | `MLA_LOG_LEVEL_VERBOSE` | 0 | Extremely detailed tracing |
| `mla_debug(msg)` | `MLA_LOG_LEVEL_DEBUG` | 1 | Development / diagnostic information |
| `mla_info(msg)` | `MLA_LOG_LEVEL_INFO` | 2 | Normal operational messages |
| `mla_warning(msg)` | `MLA_LOG_LEVEL_WARNING` | 3 | Unexpected but recoverable situations |
| `mla_error(msg)` | `MLA_LOG_LEVEL_ERROR` | 4 | Operation failures |

A logger only receives messages at or above its configured level. For example, a logger set to `MLA_LOG_LEVEL_INFO` will receive info, warning, and error messages but not verbose or debug.

## Quick Start — Log a Message

```cpp
#include "../log/mla_logging.h"

void my_function() {
    mla_info("Module initialised");
    mla_debug("Processing started");
    mla_warning("Unexpected input — using defaults");
    mla_error("Connection lost");
}
```

The macros automatically inject the source file name and function name as context.

## Console Logger (Built-In)

The simplest way to see log output on stdout:

```cpp
#include "../log/mla_logger_console.h"

// Activate at startup (typically in a boot event)
mla_log_to_console_activate();
mla_log_to_console_set_level(MLA_LOG_LEVEL_DEBUG);

// … later …
mla_info("This appears on the console");

// Deactivate if no longer needed
mla_log_to_console_deactivate();
```

### Console Logger Functions

| Function | Purpose |
|---|---|
| `mla_log_to_console_activate()` | Enable console logging |
| `mla_log_to_console_deactivate()` | Disable console logging |
| `mla_log_to_console_is_active()` | Check if console logging is active |
| `mla_log_to_console_set_level(level)` | Set the console logger's minimum level |
| `mla_log_to_console_get_level()` | Get the console logger's current level |

## Custom Logger

### Step 1 — Write a Logger Callback

The callback receives the log level, the message, a context string (file + function), and user data:

```cpp
void my_log_writer(
    const mla_log_level level,
    mla_string_t& message,
    mla_string_t& context,
    mla_user_data_t& userData)
{
    // context contains "filename::function"
    // Write to a file, network socket, ring buffer, etc.
}
```

### Step 2 — Register the Logger

```cpp
#include "../log/mla_logger.h"

mla_logger_t logger;
logger.name = mla_string_const("MyFileLogger");
logger.level = MLA_LOG_LEVEL_INFO;
logger.need_full_managed_strings = false;
logger.write = my_log_writer;
logger.userData = mla_user_data_empty(); // attach custom state if needed

mla_log_register_logger(logger);
```

### Step 3 — Manage the Logger at Runtime

```cpp
// Change level dynamically
mla_log_set_logger_level(mla_string_const("MyFileLogger"), MLA_LOG_LEVEL_DEBUG);

// Query current level
mla_log_level lvl = mla_log_get_logger_level(mla_string_const("MyFileLogger"));

// Remove the logger
mla_log_unregister_logger(mla_string_const("MyFileLogger"));
```

## RPC Logger

The RPC logger (`mla_logger_rpc.h`) caches recent log messages and exposes them over RPC for remote monitoring. It provides the following RPC procedures:

| Procedure | Signature | Purpose |
|---|---|---|
| `"log/setLoglevel"` | `mla_logger_rpc_log_level_t → void` | Set the log level remotely |
| `"log/getLoglevel"` | `void → mla_logger_rpc_log_level_t` | Get the current log level |
| `"log/getMessages"` | `void → mla_logger_rpc_log_messages_t` | Retrieve cached log messages |

## Checking if a Level is Active

Before constructing an expensive log message, check whether any logger will receive it:

```cpp
if (mla_log_is_active(MLA_LOG_LEVEL_DEBUG)) {
    mla_string_t msg = build_expensive_debug_string();
    mla_debug(msg);
    mla_string_destroy(msg);
}
```

## Level-Specific Functions (Alternative to Macros)

When the file/function context is not needed, call the functions directly:

```cpp
mla_log_info(mla_string_const("Started"), mla_string_const("my_module"));
mla_log_error(mla_string_const("Failed"), mla_string_const("my_module"));
```

## Utility

```cpp
const mla_char_t* levelStr = mla_log_level_to_string(MLA_LOG_LEVEL_WARNING); // "WARNING"
```

## Rules

- Use the logging macros (`mla_info`, `mla_debug`, etc.) in module code — they automatically add file/function context.
- **Never** use `printf`, `std::cout`, or other standard I/O for logging — always use the MLA logging framework.
- Each logger name must be **unique** across the application.
- Register loggers during boot events (e.g., `mla_lifecycle_boot_event_priority_application_setup`).
- Set `need_full_managed_strings` to `true` only if the logger callback stores the message for later use — it forces a heap copy.
- Check `mla_log_is_active` before building expensive log messages.

## Incorrect Usage

```cpp
// ❌ Using printf / std::cout
printf("Error: %s\n", msg);           // use mla_error(msg)
std::cout << "Info: " << msg << "\n"; // use mla_info(msg)

// ❌ Registering a logger with a duplicate name
mla_log_register_logger(loggerA); // name = "Console"
mla_log_register_logger(loggerB); // name = "Console" — duplicate!

// ❌ Not checking mla_log_is_active for expensive messages
mla_string_t huge = build_large_report(); // always built even if no logger cares
mla_debug(huge);
```
