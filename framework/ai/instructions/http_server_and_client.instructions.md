---
applyTo: '**'
description: 'Patterns for creating HTTP servers and making HTTP client requests in the MLA framework'
---

# HTTP Server and Client

The HTTP module (`framework/core/http/`) provides a lightweight HTTP server and client. Requests and responses use `mla_stream_input_t` / `mla_stream_output_t` for body content and `mla_array_list_t` of `mla_http_header_t` for headers.

## Key Types

| Type | Header | Purpose |
|---|---|---|
| `mla_http_server_t` | `mla_http_server.h` | HTTP server with request handler registry |
| `mla_http_server_handler_item_t` | `mla_http_server.h` | A registered request handler (method + URL matcher + executor) |
| `mla_http_client_t` | `mla_http_client.h` | HTTP client with timeout settings |
| `mla_http_client_response_t` | `mla_http_client.h` | Client response wrapper with status/error info |
| `mla_http_request_t` | `mla_http_request.h` | HTTP request (method, URL, headers, body) |
| `mla_http_response_t` | `mla_http_response.h` | HTTP response (status code, headers, body) |
| `mla_http_header_t` | `mla_http_header.h` | A single HTTP header (name + values) |

## HTTP Method Constants

```cpp
mla_http_method_get     // "GET"
mla_http_method_post    // "POST"
mla_http_method_put     // "PUT"
mla_http_method_delete  // "DELETE"
mla_http_method_options // "OPTIONS"
```

## Status Code Constants

```cpp
mla_http_status_ok                    // 200
mla_http_status_created               // 201
mla_http_status_no_content            // 204
mla_http_status_moved_permanently     // 301
mla_http_status_bad_request           // 400
mla_http_status_unauthorized          // 401
mla_http_status_forbidden             // 403
mla_http_status_not_found             // 404
mla_http_status_method_not_allowed    // 405
mla_http_status_internal_server_error // 500
```

---

## HTTP Server

### Step 1 — Create the Server

```cpp
#include "../http/mla_http_server.h"
#include "../network/mla_network.h"

mla_network_host_t host = mla_network_host_ip4(mla_string_const("0.0.0.0"), 8080);
mla_http_server_t server = mla_http_server(host);
```

### Step 2 — Write a Request Handler

A handler receives a request and populates a response. It returns `true` on success:

```cpp
static mla_bool_t api_status_handler(
    mla_http_server_t& http_server,
    const mla_http_request_t &request,
    mla_http_response_t &response)
{
    response.statusCode = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    mla_http_headers_add(response.headers,
        mla_string_const("Content-Type"),
        mla_string_const("application/json"));

    // Provide the body via a content stream or a content writer
    mla_string_t body = mla_string_const("{\"status\":\"running\"}");
    response.content = mla_stream_input_from_string(body);
    return true;
}
```

### Step 3 — Register Handlers

```cpp
// Match GET requests whose path starts with "/api/"
mla_http_server_handler_item_t handler =
    mla_http_server_handler_starts_with(
        mla_http_method_get,
        mla_string_const("/api/"),
        api_status_handler);
mla_http_server_register_handler(server, handler);

// Match all POST requests (any path)
mla_http_server_handler_item_t postAll =
    mla_http_server_handler_all(mla_http_method_post, post_handler);
mla_http_server_register_handler(server, postAll);
```

### Step 4 — Start and Stop

```cpp
// Start with 4 worker tasks
mla_http_server_start(server, 4);

// Later …
mla_http_server_stop(server);
mla_http_server_destroy(server);
```

### WebSocket Support

Register a WebSocket handler for a specific path:

```cpp
static mla_bool_t ws_text_handler(
    mla_http_server_websocket_connection_t& connection,
    const mla_string_t& message,
    mla_bool_t isFinalFragment)
{
    // Echo the message back
    mla_http_server_send_websocket_text_message(connection, message, true);
    return true;
}

mla_http_server_websocket_handler_item_t wsHandler =
    mla_http_server_websocket_handler_path_equals(
        mla_string_const("/ws"),
        ws_text_handler,   // text message handler
        nullptr            // binary message handler (optional)
    );
mla_http_server_register_websocket_handler(server, wsHandler);
```

WebSocket connection management:

```cpp
// Find connections by path prefix
auto conns = mla_http_server_find_websocket_connections(server, mla_string_const("/ws"));

// Send to a specific connection by ID
mla_http_server_send_websocket_text_message(server, connectionId, msg, true);

// Close a connection
mla_http_server_close_websocket_connection(connection, 1000, mla_string_const("Normal closure"));
```

---

## HTTP Client

### Sending a GET Request

```cpp
#include "../http/mla_http_client.h"

mla_http_request_t request = mla_http_get_request(
    mla_string_const("http://example.com/api/status"));
mla_http_headers_add(request.headers,
    mla_string_const("Accept"),
    mla_string_const("application/json"));

mla_http_client_response_t result = mla_http_client_send_request(request);

if (result.status == MLA_HTTP_CLIENT_RESPONSE_STATUS_OK) {
    mla_http_response_t& resp = result.response;
    // resp.statusCode, resp.headers, resp.content are available
} else {
    mla_error("HTTP request failed");
}

mla_http_client_response_destroy(result);
```

### Sending a POST Request with Body

```cpp
mla_http_request_t request = mla_http_post_request(
    mla_string_const("http://example.com/api/data"));
mla_http_headers_add(request.headers,
    mla_string_const("Content-Type"),
    mla_string_const("application/json"));

mla_string_t body = mla_string_const("{\"key\":\"value\"}");
request.content = mla_stream_input_from_string(body);

mla_http_client_response_t result = mla_http_client_send_request(request);
// … check result.status …
mla_http_client_response_destroy(result);
```

### Custom Client (Timeout)

```cpp
mla_http_client_t client = mla_http_client();
client.timeout_ms = 10000; // 10 seconds

mla_http_client_response_t result = mla_http_client_send_request(client, request);
```

### Client Response Status Codes

| Enum | Meaning |
|---|---|
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_OK` | Request succeeded |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL` | Unsupported protocol |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST` | DNS resolution failed |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED` | TCP connection failed |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT` | Request timed out |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_INVALID_RESPONSE` | Could not parse response |
| `MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN` | Other error |

---

## Working with Headers

```cpp
// Add a header
mla_http_headers_add(request.headers,
    mla_string_const("Authorization"),
    mla_string_const("Bearer abc123"));

// Read a header value
mla_string_t contentType = mla_http_headers_get_value(
    response.headers,
    mla_string_const("Content-Type"));

// Check for a specific header value
mla_bool_t hasJson = mla_http_headers_has_header_value(
    response.headers,
    mla_string_const("Content-Type"),
    mla_string_const("application/json"));
```

## Rules

- Always call `mla_http_client_response_destroy` after processing a client response.
- Always call `mla_http_server_destroy` when the server is no longer needed.
- Request handlers must **not** store pointers to the request or response after returning — they are stack-allocated.
- Use `mla_http_server_handler_starts_with` for prefix-based URL matching and `mla_http_server_handler_all` for catch-all handlers.
- WebSocket message handlers must return `true` to keep the connection open and `false` to close it.
- Use `mla_stream_input_from_string` to set a string body on a request or response.

## Incorrect Usage

```cpp
// ❌ Using standard library HTTP utilities
#include <curl/curl.h> // never — use mla_http_client

// ❌ Forgetting to destroy the client response
mla_http_client_response_t result = mla_http_client_send_request(request);
// leaked — always call mla_http_client_response_destroy(result)

// ❌ Not checking the response status
mla_http_client_response_t result = mla_http_client_send_request(request);
auto body = result.response.content; // may be invalid if status != OK

// ❌ Using printf for response output
printf("status: %d\n", response.statusCode); // use mla_info or stream writes
```
