# UI Module

The UI module provides a web-based user interface framework built on top of the HTTP server. It enables the creation of modern web UIs that can be embedded into applications, with support for serving static assets and handling HTTP-based UI interactions.

## Architecture

The UI module integrates with the HTTP module to provide:

- **`mla_ui_http_server`**: HTTP server initialization and configuration specifically for UI applications
- **`mla_ui_rpc`**: RPC integration for UI communication (reserved for future use)
- **`mla_ui_web_embedded`**: Embedded web resources for serving UI assets

The UI module is designed to work seamlessly with the HTTP and RPC modules, enabling the creation of full-featured web-based interfaces for embedded systems and applications.

## Usage

### Initializing the UI HTTP Server

To create a web-based UI, you need to initialize the UI HTTP server with an existing HTTP server instance.

```cpp
#include "mla_ui_http_server.h"
#include "mla_http_server.h"

// Create an HTTP server
mla_network_host_t host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 8080);
mla_http_server_t server = mla_http_server(host);

// Initialize the UI HTTP server
if (mla_ui_http_server_initialize(server)) {
    // Start the server
    if (mla_http_server_start(server, 4)) {
        // Server is now running and serving the UI
    }
}
```

### Accessing the UI

Once the UI HTTP server is initialized and started, you can access the web interface by navigating to the server's address in a web browser:

```
http://127.0.0.1:8080/
```

### Integration with RPC

The UI module is designed to work with the RPC module for handling UI actions and data exchange. UI events can trigger RPC calls to the backend, and RPC responses can update the UI state.

```cpp
// The UI module can expose RPC endpoints for UI interactions
// See mla_ui_rpc.h for RPC-specific UI functionality (future implementation)
```

### Embedded Web Resources

The UI module can embed web resources (HTML, CSS, JavaScript) directly into the application binary, allowing for deployment without external file dependencies. This is particularly useful for embedded systems with limited filesystem access.

## Example

Here's a complete example of creating a simple web UI server:

```cpp
#include "mla_ui_http_server.h"
#include "mla_http_server.h"
#include "mla_network.h"

int main() {
    // Create the server host
    mla_network_host_t host = mla_network_host_ip4(
        mla_string_const("0.0.0.0"), 
        8080
    );
    
    // Create the HTTP server
    mla_http_server_t server = mla_http_server(host);
    
    // Initialize UI functionality
    if (!mla_ui_http_server_initialize(server)) {
        // Handle initialization error
        return 1;
    }
    
    // Start the server with 4 worker threads
    if (!mla_http_server_start(server, 4)) {
        // Handle server start error
        return 1;
    }
    
    // Server is now running
    // Keep the application alive
    while (true) {
        // Main application loop
        mla_task_manager_process_all_tasks();
    }
    
    return 0;
}
```

## Use Cases

The UI module is ideal for:
- **Embedded device configuration**: Web-based configuration interfaces for IoT devices
- **System monitoring**: Real-time dashboards and monitoring UIs
- **Remote control**: Web interfaces for controlling hardware or software systems
- **Diagnostic tools**: Web-based debugging and diagnostic interfaces
- **User applications**: Full-featured web applications embedded in desktop or mobile apps

## Project Conventions (Latest)

This module follows the current mla-c project-wide conventions:

- **Code style:** Use MLA data types and naming conventions described in the [main README](../../../README.md).
- **Heap ownership:** Use `mla_pointer_t` for owned heap allocations; use raw platform pointers only for transient access.

For full details, see:
- [Main project README](../../../README.md)
- [Core data types documentation](../../../core/readme.md)
- [Memory module documentation](../../../core/memory/readme.md)

