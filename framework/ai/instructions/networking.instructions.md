---
applyTo: '**'
description: 'Patterns for TCP/UDP networking in the MLA framework'
---

# Networking

The network module (`framework/core/network/mla_network.h`) provides a platform-abstracted socket API for TCP and UDP communication. It exposes IP address handling, DNS resolution, client connections, and server listeners — all using MLA streams for I/O.

## Key Types

| Type | Purpose |
|---|---|
| `mla_network_ip_address_t` | IP address (v4 or v6) |
| `mla_network_host_t` | IP address + port |
| `mla_network_connection_t` | An active connection with input/output streams |
| `mla_network_listener_t` | A bound listener that accepts incoming connections |
| `mla_connection_type_t` | Enum: `mla_connection_type_tcp` or `mla_connection_type_udp` |

## IP Addresses

```cpp
#include "../network/mla_network.h"

// IPv4
mla_network_ip_address_t ip4 = mla_network_ip_address_ip4(mla_string_const("192.168.1.10"));

// IPv6
mla_network_ip_address_t ip6 = mla_network_ip_address_ip6(mla_string_const("::1"));

// Invalid / sentinel
mla_network_ip_address_t invalid = mla_network_ip_address_invalid();
```

## Hosts (IP + Port)

```cpp
mla_network_host_t server = mla_network_host_ip4(mla_string_const("192.168.1.10"), 8080);
mla_network_host_t local6 = mla_network_host_ip6(mla_string_const("::1"), 3000);
mla_network_host_t invalid = mla_network_host_invalid();
```

## DNS Resolution

```cpp
mla_network_host_t host = mla_network_host_invalid();
if (mla_network_host_resolve(host, mla_string_const("example.com"), 443)) {
    // host.address now contains the resolved IP
    // host.port is 443
} else {
    mla_error("DNS resolution failed");
}
```

## Discovering Local IP Addresses

```cpp
auto localIPs = mla_network_get_local_ip_addresses();
for (mla_size_t i = 0; i < mla_array_list_size(localIPs); i++) {
    auto& ip = mla_array_list_get_unsafe(localIPs, i);
    mla_info(ip.address); // e.g., "192.168.1.5"
}
```

## TCP Client Connection

```cpp
mla_network_host_t server = mla_network_host_ip4(mla_string_const("192.168.1.10"), 8080);
mla_network_connection_t conn = mla_network_connection_disconnected();

if (mla_network_connection_connect(conn, server, mla_connection_type_tcp, 5000)) {
    // Connected — use conn.inputStream and conn.outputStream

    // Write data
    mla_stream_output_write_string(conn.outputStream, mla_string_const("Hello\n"));

    // Read data
    mla_byte_t buf[256];
    mla_size_t bytesRead = conn.inputStream.read(conn.inputStream, 0, sizeof(buf), buf);

    // Disconnect when done
    mla_network_connection_disconnect(conn);
} else {
    mla_error("Connection failed");
}
```

### Connection Struct

```cpp
struct mla_network_connection_t {
    mla_network_host_t host;          // Remote host
    mla_stream_input_t inputStream;   // Read from the remote end
    mla_stream_output_t outputStream; // Write to the remote end
};
```

### Connection Functions

| Function | Purpose |
|---|---|
| `mla_network_connection_disconnected()` | Create a disconnected sentinel |
| `mla_network_connection_connect(conn, host, type, timeout_ms)` | Connect to a remote host |
| `mla_network_connection_disconnect(conn)` | Close the connection |
| `mla_network_connection_is_connected(conn)` | Check if still connected |

## TCP Server Listener

```cpp
mla_network_host_t bindHost = mla_network_host_ip4(mla_string_const("0.0.0.0"), 9090);
mla_network_listener_t listener = mla_network_listener_invalid();

if (mla_network_listener_bind_and_listen(listener, bindHost, mla_connection_type_tcp)) {
    mla_info("Listening on port 9090");

    // Accept loop (typically runs inside a native task)
    mla_network_connection_t client = mla_network_connection_disconnected();
    while (mla_network_listener_accept_connection(listener, client)) {
        // client is now connected — process request
        // … read from client.inputStream, write to client.outputStream …
        mla_network_connection_disconnect(client);
    }

    mla_network_listener_close(listener);
} else {
    mla_error("Failed to bind");
}
```

### Listener Functions

| Function | Purpose |
|---|---|
| `mla_network_listener_invalid()` | Create an invalid sentinel |
| `mla_network_listener_bind_and_listen(listener, host, type)` | Bind and start listening |
| `mla_network_listener_accept_connection(listener, conn)` | Accept the next incoming connection (blocking) |
| `mla_network_listener_close(listener)` | Stop listening and release the socket |

## Using Connections Inside Tasks

Server accept loops block, so they must run in a **native task**:

```cpp
#include "../task/mla_task.h"
#include "../task/mla_task_manager.h"

static mla_task_process_result_state server_task(mla_user_data_t& p_UserData) {
    mla_network_listener_t* listener =
        mla_user_data_get_pointer<mla_network_listener_t>(p_UserData, listenerId);

    mla_network_connection_t client = mla_network_connection_disconnected();
    if (mla_network_listener_accept_connection(*listener, client)) {
        // handle client …
        mla_network_connection_disconnect(client);
    }
    return TASK_PROCESS_RESULT_CONTINUE;
}

void start_server() {
    // … bind listener …
    mla_user_data_t ud = mla_user_data_empty();
    mla_user_data_set_pointer_without_ownership(ud, listenerId, &listener);

    mla_task_t task = mla_task_native(mla_string_const("tcp.server"), server_task, ud);
    task.priority = TASK_PRIO_HIGH;
    task.stack_size = TASK_STACK_SIZE_LARGE;
    mla_task_manager_register_task(task);
}
```

## Rules

- **Never** use POSIX `socket()`, `connect()`, `bind()`, `accept()`, or WinSock APIs directly — use the `mla_network_*` functions.
- Always check return values of `connect`, `bind_and_listen`, and `accept_connection`.
- Blocking accept/receive calls must run inside `mla_task_native` tasks on platforms with a simulated task loop.
- Always call `mla_network_connection_disconnect` when a connection is no longer needed.
- Always call `mla_network_listener_close` when the server is shutting down.

## Incorrect Usage

```cpp
// ❌ Using raw POSIX sockets
int fd = socket(AF_INET, SOCK_STREAM, 0); // use mla_network_connection_connect

// ❌ Blocking in a simulated repeating task
static mla_task_process_result_state bad_task(mla_user_data_t& ud) {
    mla_network_listener_accept_connection(listener, conn); // blocks the entire loop
    return TASK_PROCESS_RESULT_CONTINUE;
}
// Use mla_task_native instead

// ❌ Forgetting to disconnect
mla_network_connection_connect(conn, host, mla_connection_type_tcp, 5000);
// … use conn … but never call mla_network_connection_disconnect(conn)

// ❌ Using a connection after disconnect
mla_network_connection_disconnect(conn);
conn.outputStream.write(conn.outputStream, 0, 5, data); // undefined
```
