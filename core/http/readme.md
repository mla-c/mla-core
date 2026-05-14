# HTTP Module

The HTTP module provides a comprehensive set of tools for building HTTP clients and servers. It includes support for HTTP requests and responses, WebSocket connections, and a flexible handler-based routing system for servers.

## Architecture

The HTTP module is divided into two main components:

- **`mla_http_server`**: A high-performance HTTP server that can handle multiple connections and route requests to different handlers based on the request URL and method.
- **`mla_http_client`**: A simple and easy-to-use HTTP client for sending HTTP requests and receiving responses.

### HTTP Server

The HTTP server is built around a handler-based architecture. You can register multiple handlers, each of which is responsible for a specific set of requests. When a request comes in, the server finds the appropriate handler and passes the request to it for processing.

The server also includes built-in support for WebSockets, allowing you to create real-time applications with bidirectional communication.

### HTTP Client

The HTTP client provides a straightforward way to send HTTP requests. You can create a request, set its method, URL, and headers, and then send it to a server. The client will return a response object containing the status code, headers, and content of the response.

## Usage

### Creating an HTTP Server

To create an HTTP server, you need to define one or more request handlers and register them with the server.

```cpp
#include "mla_http_server.h"

// Define a request handler
mla_bool_t hello_world_handler(const mla_http_request_t& request, mla_http_response_t& response) {
    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    response.content = mla_stream_input_from_buffer((mla_byte_t*)mla_string_const("Hello, world!").data, mla_string_const("Hello, world!").length);
    return true;
}

// Create a server and register the handler
mla_network_host_t host = mla_network_host_ip4(mla_string_const("127.0.0.1"), 8080);
mla_http_server_t server = mla_http_server(host);
mla_http_server_handler_item_t handler = mla_http_server_handler_all(mla_http_method_get, hello_world_handler);
mla_http_server_register_handler(server, handler);

// Start the server
mla_http_server_start(server, 4); // Use 4 worker threads
```

### Creating an HTTP Client

To create an HTTP client, you simply need to create a request and send it.

```cpp
#include "mla_http_client.h"

// Create a GET request
mla_http_request_t request = mla_http_get_request(mla_string_const("http://127.0.0.1:8080"));

// Send the request
mla_http_client_response_t response = mla_http_client_send_request(request);

// Check the response
if (response.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
    // Process the response
}
```

### WebSockets

The HTTP server also supports WebSockets. You can register a WebSocket handler to handle WebSocket connections.

```cpp
#include "mla_http_server.h"

// Define a WebSocket message handler
mla_bool_t websocket_echo_handler(mla_http_server_websocket_connection_t& connection, const mla_string_t& message, mla_bool_t isFinalFragment) {
    // Echo the message back to the client
    return mla_http_server_send_websocket_text_message(connection, message, isFinalFragment);
}

// Register the WebSocket handler
mla_http_server_websocket_handler_item_t ws_handler = mla_http_server_websocket_handler_path_equals(
    mla_string_const("/echo"),
    websocket_echo_handler,
    nullptr // No binary message handler
);
mla_http_server_register_websocket_handler(server, ws_handler);
```
