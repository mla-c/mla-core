# System Module

The System module provides fundamental data structures and utilities that form the foundation of the CoreOS library. It includes essential types for strings, buffers, streams, collections, and other core functionality used throughout the framework.

## Architecture

The System module consists of several key components:

- **String Management**: UTF-8 string handling with support for multiple memory layouts and encodings
- **Buffer Management**: Memory buffer abstraction with ownership tracking
- **Stream Abstraction**: Input/output stream interfaces for data transfer
- **Collections**: Array lists, hash maps, and linked lists for data organization
- **Type Utilities**: Numeric conversions, byte arrays, and reference counting

## Key Components

### String (`mla_string.h`)

Provides a flexible string type that supports multiple memory layouts and UTF-8 encoding.

```cpp
#include "mla_string.h"

// Create a string from a literal
mla_string_t str = mla_string_const("Hello, world!");

// Create an empty string
mla_string_t empty = mla_string_empty();

// Compare strings
if (mla_string_equals(str1, str2)) {
    // Strings are equal
}

// Get substring
mla_string_t substr = mla_string_substring(str, 0, 5);

// Convert to C-string
mla_c_string_t cstr = mla_string_to_c_string(str);
```

### Buffer (`mla_buffer.h`)

Manages memory buffers with automatic lifetime tracking.

```cpp
#include "mla_buffer.h"

// Create a buffer
mla_buffer_t buffer = mla_buffer_create(1024);

// Access buffer data
mla_byte_t* data = mla_buffer_get_data(buffer);

// Buffers use reference counting for automatic cleanup
```

### Stream (`mla_stream.h`)

Provides abstract input and output stream interfaces for flexible data I/O.

```cpp
#include "mla_stream.h"

// Create a stream from a buffer
mla_stream_input_t input = mla_stream_input_from_buffer(buffer, size);
mla_stream_output_t output = mla_stream_output_to_buffer(buffer, size);

// Standard I/O streams
mla_stream_input_t stdin_stream = mla_stream_input_stdin();
mla_stream_output_t stdout_stream = mla_stream_output_stdout();

// No-op streams (for testing or placeholders)
mla_stream_input_t noop_in = mla_stream_noop_input();
mla_stream_output_t noop_out = mla_stream_noop_output();

// Copy data between streams
mla_stream_copy(input, output);
```

### Array List (`mla_array_list.h`)

A dynamic array implementation with templated types.

```cpp
#include "mla_array_list.h"

// Create an array list
mla_array_list_t<mla_int32_t, mla_int32_initializer> list;

// Add items
mla_array_list_add(list, 42);

// Get items
mla_int32_t value = mla_array_list_get(list, 0);

// Get size
mla_size_t size = mla_array_list_size(list);
```

### Hash Map (`mla_hash_map.h`)

A hash table implementation for key-value storage.

```cpp
#include "mla_hash_map.h"

// Create a hash map
mla_hash_map_t<mla_string_t, mla_int32_t, mla_string_hash_t, 
               mla_string_initializer, mla_int32_initializer> map;

// Insert values
mla_hash_map_set(map, mla_string_const("key"), 42);

// Get values
mla_int32_t value;
if (mla_hash_map_get(map, mla_string_const("key"), value)) {
    // Value found
}

// Check if key exists
if (mla_hash_map_contains(map, mla_string_const("key"))) {
    // Key exists
}
```

### Link List (`mla_link_list.h`)

A doubly-linked list implementation.

```cpp
#include "mla_link_list.h"

// Create a linked list
mla_link_list_t<mla_int32_t, mla_int32_initializer> list;

// Add items
mla_link_list_add_back(list, 42);
mla_link_list_add_front(list, 10);

// Iterate through items
mla_link_list_node_t<mla_int32_t>* node = mla_link_list_front(list);
while (node != nullptr) {
    mla_int32_t value = node->data;
    node = node->next;
}
```

### Bytes (`mla_bytes.h`)

Utilities for working with byte arrays.

```cpp
#include "mla_bytes.h"

// Create bytes from string
mla_bytes_t bytes = mla_bytes_from_string(str);

// Compare bytes
if (mla_bytes_equals(bytes1, bytes2)) {
    // Bytes are equal
}
```

### Number Utilities (`mla_number.h`)

Functions for parsing and converting numeric values.

```cpp
#include "mla_number.h"

// Parse integers
mla_int32_t value;
if (mla_string_parse_int32(str, value)) {
    // Parsed successfully
}

// Parse floats
mla_float_t float_value;
if (mla_string_parse_float(str, float_value)) {
    // Parsed successfully
}
```

### ID Generation (`mla_id.h`)

Utilities for generating unique identifiers.

```cpp
#include "mla_id.h"

// Generate a unique ID
mla_id_t id = mla_id_generate();
```

### Reference Counting (`mla_reference.h`)

Provides reference counting for automatic memory management.

```cpp
#include "mla_reference.h"

// Reference counting is used internally by buffers and other types
// to manage memory lifecycle automatically
```

## Memory Layouts

The System module supports multiple memory layouts for strings:
- **C-style strings**: Null-terminated character arrays
- **Buffer-based strings**: Non-null-terminated buffers with explicit length
- **Substrings**: Views into existing strings without copying data

This flexibility allows for efficient string handling in various scenarios while maintaining a consistent API.
