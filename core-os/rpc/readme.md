# RPC Module

The RPC (Remote Procedure Call) module provides a powerful and flexible way to execute procedures on remote systems. It includes a type-safe interface for defining and calling procedures, and it can be easily extended to support different transport protocols.

## Architecture

The RPC module is designed with a layered architecture:

- **Core RPC Layer**: This layer provides the fundamental building blocks for defining, registering, and executing RPC procedures. It's transport-agnostic, meaning it can be used with any communication protocol.
- **HTTP Transport Layer**: This layer provides an implementation of the RPC module over HTTP. It includes an HTTP server that can handle RPC requests and an HTTP client that can send them.

### Key Concepts

- **Procedure**: A procedure is a function that can be called remotely. Each procedure has a name, an input data type, and an output data type.
- **Remote Endpoint**: A remote endpoint represents a connection to a remote system that can execute RPC procedures.
- **Serialization**: The RPC module uses the Serializer module to convert data to and from a format that can be transmitted over the network.

## Usage

### Defining an RPC Procedure

To define an RPC procedure, you need to create a handler function and then register it using the `mla_rpc_auto_register_procedure` macro.

```cpp
#include "mla_rpc.h"

// Define input and output data structures
struct my_input_t {
    mla_int32_t a;
    mla_int32_t b;
    // ... serialization/deserialization functions
};

struct my_output_t {
    mla_int32_t sum;
    // ... serialization/deserialization functions
};

// Define the procedure handler
mla_bool_t my_procedure_handler(const my_input_t* input, my_output_t* output) {
    output->sum = input->a + input->b;
    return true;
}

// Register the procedure
mla_rpc_auto_register_procedure(
    "math/sum",
    my_input_t,
    my_output_t,
    my_procedure_handler
);
```

### Exposing Procedures over HTTP

To expose your RPC procedures over HTTP, you need to initialize the RPC HTTP server.

```cpp
#include "mla_rpc_http_server.h"
#include "mla_http_server.h"

// Create an HTTP server
mla_http_server_t server = mla_http_server(my_host);

// Initialize the RPC HTTP server
mla_rpc_http_server_initialize(server);

// Start the HTTP server
mla_http_server_start(server, 4);
```

### Calling a Remote Procedure

To call a remote procedure, you first need to register an HTTP remote endpoint, and then you can call the procedure using `mla_rpc_execute_procedure_remote`.

```cpp
#include "mla_rpc_http_client.h"

// Register the remote endpoint
mla_rpc_remote_endpoint_t endpoint = mla_rpc_http_register_endpoint(
    mla_string_const("http://remote-server/rpc/"),
    mla_http_rpc_content_type_binary
);

// Prepare the input and output data
my_input_t input = { 3, 4 };
my_output_t output = { 0 };

// Execute the remote procedure
mla_rpc_execute_procedure_remote(
    mla_string_const("math/sum"),
    &input,
    &output
);
```