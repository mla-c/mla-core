# CLI Module

The CLI module provides a framework for building command-line interfaces (CLIs). It allows you to create a hierarchical structure of modules and commands, process user input, and execute actions based on commands.

## Architecture

The CLI module consists of three main components:

- **`mla_cli_app`**: The main application object that manages the CLI's state, including the active modules and input processing.
- **`mla_cli_module`**: A container for commands and sub-modules, allowing you to organize your CLI into a tree-like structure.
- **`mla_cli_command`**: Represents a specific action that the user can perform. Each command can have a name, a description, a set of parameters, and an execution function.

## Usage

### Creating a CLI Application

To create a CLI application, you need to initialize an `mla_cli_app_t` with a root module.

```cpp
#include "mla_cli_app.h"

// Create a root module
mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

// Create a CLI app
mla_cli_app_t app = mla_cli_app_init(root, mla_stream_noop_output());
```

### Adding Modules and Commands

You can add sub-modules and commands to any module.

```cpp
// Create a sub-module
mla_cli_module_t subModule = mla_cli_module(mla_string_const("SubModule"));

// Add the sub-module to the root module
mla_cli_module_add_sub_module(root, subModule);

// Create a command
mla_cli_command_t myCommand = mla_cli_command(mla_string_const("mycommand"));
myCommand.execute = [](const mla_cli_command_t&, const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>&, const mla_cli_command_execute_outstream_t& out) {
    out.write(out.userdata, mla_string_const("Executing mycommand"));
    return true;
};

// Add the command to the sub-module
mla_cli_module_add_command(subModule, myCommand);
```

### Processing User Input

The `mla_cli_app_update_and_process_input` function processes user input and updates the application state.

```cpp
// Simulate user input
mla_string_t buffer = mla_string("SubModule\nmycommand\n");

// Process the input
mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length), mla_stream_noop_output());
```

### Example

Here's a complete example of how to create a simple CLI with a nested module and a command:

```cpp
#include "mla_cli_app.h"
#include "mla_stream.h"

// Command execution function
mla_bool_t test_command_execute(const mla_cli_command_t&, const mla_hash_map_t<mla_string_t, mla_string_t, mla_string_hash_t, mla_string_initializer, mla_string_initializer>&, const mla_cli_command_execute_outstream_t& out) {
    out.write(out.userdata, mla_string_const("Command executed successfully"));
    return true;
}

void main() {
    // Create a root module
    mla_cli_module_t root = mla_cli_module(mla_string_const("Root"));

    // Create a sub-module
    mla_cli_module_t subModule = mla_cli_module(mla_string_const("SubModule"));
    mla_cli_module_add_sub_module(root, subModule);

    // Create a command and add it to the sub-module
    mla_cli_command_t testCommand = mla_cli_command(mla_string_const("testcmd"));
    testCommand.execute = test_command_execute;
    mla_cli_module_add_command(subModule, testCommand);

    // Initialize the CLI app
    mla_cli_app_t app = mla_cli_app_init(root, mla_stream_noop_output());

    // Simulate user input to navigate to the sub-module and execute the command
    mla_string_t buffer = mla_string("SubModule\ntestcmd\n");
    mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length), mla_stream_noop_output());
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

