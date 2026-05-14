# Network Module

The Network module provides a cross-platform interface for TCP and UDP networking. It includes functionality for resolving hostnames, creating client connections, and setting up network listeners (servers).

## Architecture

The Network module is designed to be abstract and extensible. It defines a set of low-level operations in the `mla_network_low_level_operations_t` struct, which can be implemented to support different networking backends. The high-level API then uses these low-level operations to provide a consistent and easy-to-use interface.

### Key Components

- **`mla_network_host_t`**: Represents a network host, including its IP address and port.
- **`mla_network_connection_t`**: Represents a client connection to a network host. It includes input and output streams for sending and receiving data.
- **`mla_network_listener_t`**: Represents a network listener that can accept incoming connections.

## Usage

### Creating a Client Connection

To create a client connection, you first need to resolve the hostname of the server you want to connect to, and then create a connection to that host.

```cpp
#include "mla_network.h"

// Resolve the hostname
mla_network_host_t host;
if (mla_network_host_resolve(host, mla_string_const("example.com"), 80)) {
    // Create a TCP connection
    mla_network_connection_t connection;
    if (mla_network_connection_connect(connection, host, mla_connection_type_tcp, 5000)) {
        // Connection successful, you can now use the connection's input and output streams
        // ...

        // Disconnect when you're done
        mla_network_connection_disconnect(connection);
    }
}
```

### Creating a Server

To create a server, you need to create a network listener and bind it to a specific host and port.

```cpp
#include "mla_network.h"

// Define a function to handle incoming connections
mla_bool_t my_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t& connection) {
    // Handle the new connection
    // ...
    return true;
}

// Create a host for the server to listen on
mla_network_host_t host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 8080);

// Create and bind the listener
mla_network_listener_t listener;
listener.accept_connection = my_accept_connection;
if (mla_network_listener_bind_and_listen(listener, host, mla_connection_type_tcp)) {
    // Listener is now active and will call my_accept_connection for each new connection
    // ...

    // Close the listener when you're done
    mla_network_listener_close(listener);
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

