---
applyTo: '**'
description: 'Data type and coding style guidelines for the MLA framework'
---

# Data Type and Coding Style Rules

## Core Principles

This project follows strict data type and coding conventions to ensure cross-platform compatibility and consistent behavior across different compilers and architectures.

## Data Type Rules

### 1. Use Only MLA Framework Types

**NEVER** use standard C/C++ types directly. Always use the MLA framework types defined in `mla_data_types.h`:

- ❌ `int`, `long`, `short`, `char`, `unsigned int`, etc.
- ✅ `mla_int32_t`, `mla_int64_t`, `mla_uint32_t`, `mla_char_t`, etc.

### 2. Available MLA Types

**Integer Types:**
- `mla_int8_t`, `mla_uint8_t` (8-bit)
- `mla_int16_t`, `mla_uint16_t` (16-bit)
- `mla_int32_t`, `mla_uint32_t` (32-bit)
- `mla_int64_t`, `mla_uint64_t` (64-bit)

**Floating Point Types:**
- `mla_float_t` (32-bit float)
- `mla_double_t` (64-bit double)

**Character Types:**
- `mla_char_t` (standard char)
- `mla_utf_16_char_t` (UTF-16 character)
- `mla_utf_32_char_t` (UTF-32 character)

**Special Types:**
- `mla_bool_t` (boolean)
- `mla_size_t` (size type, aliased to `mla_uint32_t`)
- `mla_byte_t` (byte type, aliased to `mla_uint8_t`)
- `mla_hash_t` (hash type)
- `mla_platform_pointer_t` (raw void pointer — **data access only**, see below)
- `mla_pointer_t` (managed smart pointer — **preferred for ownership**, see below)
- `mla_void_t` (void)

### 3. Type Limits

Use MLA-defined limits instead of standard library constants:

- ❌ `INT_MAX`, `UINT_MAX`, `SIZE_MAX`
- ✅ `mla_int32_max`, `mla_uint32_max`, `mla_size_max`

Available for all integer types (e.g., `mla_int8_min`, `mla_int8_max`, `mla_uint64_max`)

## Memory Operations

### Use MLA Memory Functions

**NEVER** use standard C library functions directly:

- ❌ `memcpy`, `memset`, `memcmp`, `memmove`
- ✅ `mla_memcpy`, `mla_memset`, `mla_memcmp`, `mla_memmove`

- ❌ `malloc`, `free`
- ✅ `mla_platform_malloc`, `mla_platform_free`

- ❌ `strcpy`, `strlen`, `strstr`
- ✅ `mla_strcpy`, `mla_strlen`, `mla_strstr`

## Pointer Types

### `mla_pointer_t` — Managed Smart Pointer (Preferred)

`mla_pointer_t` is the **preferred way to hold and pass heap-allocated objects** in the MLA framework. It is a reference-counted smart pointer that provides automatic memory cleanup when the last reference is released.

**Key properties:**
- **Automatic cleanup:** When the reference count drops to zero the registered `mla_pointer_cleanup_hook_t` is called, followed by `mla_platform_free`. You never need to call `free` manually.
- **Shared ownership via copy:** Copying a `mla_pointer_t` increments the internal reference counter; the destructor decrements it. Memory is freed only when all copies are gone.
- **Safe assignment:** The assignment operator handles cyclic reference edge cases correctly (e.g. nodes in a linked list).
- **Null state:** Use `mla_pointer_null()` to represent an empty/invalid pointer and `mla_pointer_is_null()` to test for it.

**Usage rules:**
- ✅ **Always** use `mla_pointer_t` when your code **owns** heap-allocated data.
- ✅ Use `mla_malloc(size, cleanup_hook, cleanup_data)` or `mla_malloc_struct(T)` to allocate a managed pointer via the active memory manager.
- ✅ Use `mla_pointer_get_data<T>(ptr)` to obtain a typed raw pointer for reading or writing the payload. This function can return `nullptr` so always check the result before dereferencing.
- ❌ **Never** use `mla_platform_malloc` / `new` for data that should be owned — wrap the result in `mla_pointer_t` instead.
- ❌ **Never** store a raw `mla_platform_pointer_t` as a long-lived owning reference.

```cpp
// Allocate a managed buffer for a custom struct
mla_pointer_t ptr = mla_malloc_struct(mla_my_struct_t);

// Access the data
mla_my_struct_t* data = mla_pointer_get_data<mla_my_struct_t>(ptr);

// Automatic cleanup when ptr goes out of scope — no manual free needed
```

---

### `mla_platform_pointer_t` — Raw Void Pointer (Data Access Only)

`mla_platform_pointer_t` is a plain `void*` and carries **no ownership semantics**. It is used exclusively for **short-lived, read/write access** to data whose lifetime is managed elsewhere (i.e. by a `mla_pointer_t`).

**Usage rules:**
- ✅ Use `mla_platform_pointer_t` only as a transient handle to access or manipulate memory (e.g. as a parameter to low-level memory functions such as `mla_memcpy`, `mla_memset`).
- ✅ Acceptable as an output of `mla_pointer_get_platform_pointer()` when working with memory-manager internals.
- ❌ **Never** store a `mla_platform_pointer_t` as a field in a struct or as a long-lived variable to represent ownership of heap memory.
- ❌ **Never** call `mla_platform_free` on a pointer that was allocated through `mla_malloc` / `mla_malloc_struct` — let `mla_pointer_t` handle the lifetime.

```cpp
// ✅ Correct — transient access for a memory operation
mla_platform_pointer_t raw = mla_pointer_get_platform_pointer(ptr);
mla_memset(raw, 0, sizeof(mla_my_struct_t));

// ❌ Incorrect — storing raw pointer as owner
mla_platform_pointer_t myData = mla_platform_malloc(sizeof(mla_my_struct_t)); // no cleanup tracking
```

---

### `mla_pointer_memory_manager_t` — Memory Manager Interface

`mla_pointer_memory_manager_t` is a **vtable-style interface struct** that defines the lifecycle strategy for `mla_pointer_t` objects. It decouples allocation, reference counting, and deallocation from the pointer type itself, allowing different memory management strategies to be plugged in at runtime.

**Structure:**

```cpp
struct mla_pointer_memory_manager_t {
    // Allocate a new managed block of `size` bytes.
    // Returns a mla_pointer_t with the first reference already held.
    mla_pointer_t (*malloc)(...);

    // Returns true if the payload represents a null/empty pointer.
    mla_bool_t (*is_null)(...);

    // Returns the raw data pointer (skipping any internal header).
    mla_platform_pointer_t (*get_platform_pointer)(...);

    // Increment the reference count (called on copy).
    void (*incReferences)(...);

    // Decrement the reference count; frees memory when it reaches zero.
    void (*decReferences)(...);

    // Returns the current reference count, or -1 if not supported.
    mla_int32_t (*get_ref_count)(...);
};
```

**Built-in implementations:**

| Global instance | Description |
|---|---|
| `g_default_pointer_memory_manager` | Standard reference-counted allocator. Allocates a header (`mla_pointer_header_t`) followed by the user data, tracks the ref count atomically, and calls the cleanup hook on release. **Use for all normally allocated objects.** |
| `g_noop_pointer_memory_manager` | No-operation manager used to wrap externally-owned resources (e.g. platform handles). Reference counting and free are no-ops. Useful when bridging with external APIs via `mla_platform_pointer_to_managed_pointer()`. |

**Active memory manager:**

The global `g_pointer_memory_manager_instance.current` holds the currently active manager. The `mla_malloc(...)` macro always allocates through this instance, so the active manager can be swapped (e.g. in tests) to intercept or redirect allocations without changing call sites.

**Usage rules:**
- ✅ Use `mla_malloc(size, cleanup_hook, cleanup_data)` or `mla_malloc_struct(T)` in normal code — they route through the active manager automatically.
- ✅ Use `mla_malloc_with_manager(manager, size, ...)` when you need to allocate with a specific manager explicitly.
- ❌ Do **not** call `mla_pointer_memory_manager_t` function pointers directly from consumer code — always go through `mla_pointer_t` and the allocation macros.

---

## Coding Style (C-Style with C++ Features)

### 1. Naming Conventions

- **Structs:** `mla_<name>_t` (e.g., `mla_string_t`, `mla_buffer_t`)
- **Functions:** `mla_<module>_<action>` (e.g., `mla_string_copy`, `mla_string_equals`)
- **Variables:** `p_<Name>` for parameters, `camelCase` for local variables
- **Constants/Macros:** `MLA_<NAME>_<CONSTANT>` or `mla_<name>_constant`

### 2. Function Signatures

Functions should follow this pattern:

```cpp
mla_return_type mla_module_function_name(const mla_type_t &p_Parameter1, mla_type_t &p_Output);
```

## Key points:
- Use references (&) for struct parameters (C++ feature)
- Use const for read-only parameters
- Prefix parameters with p_
- Return types use MLA types

### 3. Struct Definitions

```cpp
struct mla_example_t {
    mla_size_t length;
    const mla_char_t *data;
    mla_bool_t isValid;
};
```

## Key points:
- Use struct keyword explicitly
- Suffix with _t
- Use MLA data types for all fields
- Operator overloading is allowed (C++ feature)

### 4. Initializers

Use initializer structs for default initialization in templates:
```cpp
struct mla_string_initializer {
    static mla_string_t init() {
        return mla_string_empty();
    }
};
```

### 5. Helper Macros and Templates

Template functions are allowed for compile-time operations:
```cpp
template<mla_size_t N>
mla_string_t mla_string_const(const mla_char_t (&literal)[N]) {
    return mla_string(literal, N-1);
}
```


### Avoid Standard Library
   ❌ #include <string>, #include <vector>, #include <stdio.h>
   ✅ Use MLA framework equivalents (mla_string_t, mla_array_list_t)
   
### Boolean Values
   - Use mla_bool_t type
   - Return values: true or false (C++ keywords are acceptable)

### Examples
   ✅ Correct

```cpp
mla_string_t mla_string_copy(const mla_string_t &p_String) {
    mla_size_t length = mla_string_length(p_String);
    mla_char_t* buffer = mla_create_char_array(length);
    mla_memcpy(buffer, mla_string_data(p_String), length);
    return mla_string_from_buffer_with_ownership(buffer, length);
}
```

❌ Incorrect

```cpp
string copy_string(const string &str) {
    size_t len = str.length();
    char* buf = new char[len];
    memcpy(buf, str.c_str(), len);
    return string(buf);
}
```

## Global Configuration System

The framework uses a centralized configuration system to manage system-wide and test-specific parameters.

### 1. Configuration Headers
- **Framework Config:** `core-os/mla_global_config.h` (included via `mla_data_types.h`)
- **Test Config:** `core-os-test-support/mla_test_global_config.h` (included via `mla_test_data_types.h`)

### 2. Naming Conventions
- **Feature Flags (Booleans):** Use the prefix `mla_global_feature_flag_` (e.g., `mla_global_feature_flag_logging_use_native`).
- **Configuration Values (Numeric/String):** Use the prefix `mla_global_config_` (e.g., `mla_global_config_string_sso_max_length`).
- **Test-Support Specific Feature Flags:** Use the prefix `mla_test_global_feature_flag_` (e.g., `mla_test_global_feature_flag_test_memory`).
- **Test-Support Specific Config Values:** Use the prefix `mla_test_global_config_` (e.g., `mla_test_global_config_benchmark_iterations`).

### 3. Usage Guidelines
- All configuration macros **MUST** be wrapped in `#ifndef` guards to allow for compile-time overrides via compiler flags (e.g., `-Dmla_global_config_default_http_timeout_ms=5000`).
- Configuration files should contain Doxygen-style documentation for each parameter.
- Only parameters that are intended to be configurable should be placed in these files. Immutable constants (like mathematical PI) should remain in their respective module headers.

### Summary
- Always use MLA data types from mla_data_types.h
- Never use standard C/C++ types or library functions directly
- Follow C-style naming with C++ references and templates
- Use struct keyword and _t suffix for types
- Prefix functions with mla_<module>_
- Use MLA memory operations and low-level from the framework like mla_platform_malloc, mla_memcpy, etc.
- Never include standard library headers; use MLA equivalents instead.
- Never use new or delete; use mla_platform_malloc and mla_platform_free instead.
- **Always use `mla_pointer_t` for owning heap-allocated data** — it provides automatic reference-counted cleanup via `mla_malloc` / `mla_malloc_struct`.
- **Use `mla_platform_pointer_t` only for short-lived, non-owning data access** (e.g. passing to `mla_memcpy`). Never store it as a long-lived owner.
- **Never call `mla_platform_free` on memory allocated through `mla_malloc` / `mla_malloc_struct`** — the `mla_pointer_memory_manager_t` handles deallocation automatically.
- **NEVER** access internal struct fields (e.g., `bytes.data`, `string.heap.data`) directly in external consumer code or tests if a corresponding API function is provided by the header file (e.g., `mla_bytes_get_data_readonly`, `mla_string_data`). External code must always use the provided API for data access and manipulation to ensure encapsulation. Internal framework implementation files (e.g., implementations like `mla_string*` and `mla_bytes*` functions) are permitted to access internal fields like `.heap.data` or `.heap.length` as needed for core operations.