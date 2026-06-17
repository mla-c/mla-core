# Example Application

The `core-exmaple-app/` directory contains a reference implementation that demonstrates how to use the mla-c library modules together in a complete application. It showcases CLI integration, background tasks, web UI, and native window rendering.

## Architecture

The example application is composed of the following header files:

- **`main_app.h`**: Application entry point that initializes logging, CLI, web UI, window UI, and background tasks.
- **`main_app_cli.h`**: Command-line interface setup with stdin/stdout processing and task manager CLI module integration.
- **`main_app_web_ui.h`**: HTTP server setup on port 8081 with RPC endpoints and web-based UI surfaces.
- **`main_app_main_window.h`**: UI controls showcase demonstrating labels, buttons, text inputs, and event handling.
- **`main_app_background_task.h`**: Periodic background task example running every 500ms with user data management.
- **`main_app_window_ui.h`**: Native display surface setup with control surface connector in single-threaded rendering mode.

## Features Demonstrated

### CLI Integration

The example registers a CLI application with a root module containing a task manager submodule. It creates a task that reads and processes stdin input:

```cpp
// CLI setup with task manager integration
mla_cli_app_t cliApp = mla_cli_app(mla_string_const("example"));
mla_cli_app_add_module(cliApp, task_manager_module);
```

### Web UI Server

An HTTP server is started on `0.0.0.0:8081`, providing:
- RPC HTTP endpoints for remote procedure calls
- Web-based UI surfaces accessible via browser
- Automatic listing of available local IP addresses with Web UI URLs

### Native Window UI

A native window demonstrates the full UI control set:
- **Labels**: All text kinds (primary, secondary, error, success, link, disabled, warning, info, custom)
- **Buttons**: All styles (primary, secondary, tertiary, link) with disabled states and click events
- **Text Inputs**: Standard and password styles with text selection

### Background Tasks

A repeating task runs every 500ms, demonstrating task scheduling, user data management, and periodic logging.

## Running the Example

The example application requires a platform header to be included before `main_app.h`. For example, on Linux:

```cpp
#include "lib/base-lib/platform/linux/mla_global_platform_linux.h"
#include "main_app.h"

int main() {
    return run();
}
```

Once running, the web UI is accessible at `http://localhost:8081` in a browser.

## Module Dependencies

The example application uses the following mla-c modules:
- **Log** - Console logging initialization
- **CLI** - Command-line interface framework
- **Task** - Background task scheduling
- **HTTP** - HTTP server for web UI
- **RPC** - Remote procedure call endpoints
- **UI** - Controls, surfaces, and web rendering
- **Lifecycle** - Boot event management
