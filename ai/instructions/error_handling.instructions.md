---
applyTo: '**'
description: 'Error handling conventions for the MLA framework (no exceptions)'
---

# Error Handling

The MLA framework compiles with `-fno-exceptions` on all platforms. **Never** use `throw`, `try`, or `catch`. Instead, errors are communicated through return values and out-parameters.

## Core Patterns

### 1. Boolean Return Value (Most Common)

Functions return `mla_bool_t` to indicate success (`true`) or failure (`false`):

```cpp
mla_bool_t mla_network_connection_connect(
    mla_network_connection_t &connection,
    const mla_network_host_t &host,
    mla_connection_type_t type,
    mla_size_t timeout_ms);

// Usage
if (!mla_network_connection_connect(conn, host, mla_connection_type_tcp, 5000)) {
    mla_error("Connection failed");
    return false; // propagate the failure
}
```

### 2. Nullptr Return for Pointer-Returning Functions

Functions that return pointers use `nullptr` to signal failure:

```cpp
mla_buffer_t* buffer = mla_buffer(data, size, cleanup);
if (buffer == nullptr) {
    mla_error("Buffer allocation failed");
    return false;
}
```

### 3. Negative Index for Search Functions

Functions that search for an item return `-1` when the item is not found:

```cpp
mla_int32_t idx = mla_string_index_of(str, mla_string_const("needle"));
if (idx < 0) {
    // not found
}

mla_int32_t pos = mla_array_list_index_of(list, item);
if (pos < 0) {
    // item not in list
}
```

### 4. Out-Parameters with Boolean Success

Complex results are returned through reference parameters; the `mla_bool_t` return indicates whether the out-parameter was populated:

```cpp
mla_url_t url = mla_url_empty();
if (!mla_url_parse(mla_string_const("https://example.com"), url)) {
    mla_error("Invalid URL");
    return false;
}
// url is now populated

mla_int32_t value = 0;
if (!mla_hash_map_get(map, key, value)) {
    // key not found — value is unchanged
}
```

### 5. Default Values on Getter Failure

User-data getters accept a default value that is returned when the key is missing:

```cpp
mla_int32_t port = mla_user_data_get_int32(userData, portId, 8080); // default 8080
mla_string_t name = mla_user_data_get_string(userData, nameId, mla_string_const("unknown"));
```

### 6. Enum Return for Multi-State Results

Some operations use enums to distinguish between multiple outcomes:

```cpp
mla_hash_map_push_result result = mla_hash_map_push(map, key, value);
if (result == MLA_HASH_MAP_PUSH_ERROR) {
    mla_error("Failed to insert into hash map");
}
// MLA_HASH_MAP_PUSH_ADDED or MLA_HASH_MAP_PUSH_REPLACED on success
```

## Error Propagation

Propagate errors by returning `false` (or the appropriate failure sentinel) up the call chain:

```cpp
mla_bool_t load_config(mla_config_definition_t& def, MyConfig& config) {
    if (!mla_config_manager_read(def, &config)) {
        mla_warning("Failed to read config — using defaults");
        return false;
    }
    return true;
}
```

## Logging Errors

Always log an error message at the point where the failure is first detected, using the logging macros:

```cpp
if (!mla_fs_file_exists(path)) {
    mla_error("File not found");
    return false;
}
```

Use the appropriate log level:
- `mla_error` — operation failed, cannot continue
- `mla_warning` — unexpected but recoverable situation
- `mla_debug` / `mla_info` — informational, not an error

## Rules

- **Never** use `throw`, `try`, `catch`, or any C++ exception mechanism.
- **Never** use `exit()`, `abort()`, or `assert()` for recoverable errors.
- Always check return values of functions that can fail.
- Propagate failures up the call stack by returning `false` / `nullptr` / `-1`.
- Log errors at the point of detection before returning.
- Use default values for optional lookups rather than aborting.
- Guard pointer dereferences with null checks.

## Incorrect Usage

```cpp
// ❌ Using exceptions
try {
    riskyOperation();
} catch (const std::exception& e) {
    // not available — -fno-exceptions
}

// ❌ Ignoring return values
mla_network_connection_connect(conn, host, mla_connection_type_tcp, 5000);
// did not check success — may use a disconnected connection

// ❌ Using assert for runtime errors
assert(buffer != nullptr); // use an if-check and return false instead

// ❌ Calling exit() on failure
if (!mla_fs_file_exists(path)) {
    exit(1); // never exit — return false instead
}
```
