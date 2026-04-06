# Memory Module

The Memory module provides a powerful mechanism for hooking into the memory allocation and deallocation process. It allows you to install custom hooks that will be called whenever memory is allocated or freed, enabling you to implement custom memory management schemes, track memory usage, or detect memory leaks.

## Architecture

The Memory module is based on a global list of memory hooks. You can install up to `mla_global_config_max_memory_hooks` hooks, and they will be called in the order they were installed for `malloc` operations and in the reverse order for `free` operations.

### Key Functions

- **`mla_memory_hook_install`**: Installs a new memory hook. It takes function pointers for your custom `malloc` and `free` hooks and returns a handle to the installed hook.
- **`mla_memory_hook_uninstall`**: Uninstalls a previously installed memory hook.

### Hook Functions

Your custom hook functions must match the following signatures:

- `mla_memory_malloc_hook_t`: `mla_bool_t (*)(mla_size_t size, mla_pointer_t* out_ptr)`
- `mla_memory_free_hook_t`: `mla_bool_t (*)(mla_pointer_t ptr)`
- `mla_memory_is_gcc_pointer_hook_t`: `MLA_MEMORY_POINTER_TYPE (*)(const mla_pointer_t ptr)`

If a `malloc` hook returns `true`, it indicates that the hook has handled the memory allocation and the original `malloc` function will not be called. If it returns `false`, the original `malloc` will be called. Similarly, if a `free` hook returns `true`, the original `free` function will not be called.

## Usage

### Installing a Memory Hook

To install a memory hook, you need to define your custom `malloc` and `free` hook functions and then call `mla_memory_hook_install`.

```cpp
#include "mla_memory_hook.h"

// Custom malloc hook
mla_bool_t my_malloc_hook(mla_size_t size, mla_pointer_t* out_ptr) {
    // Your custom allocation logic here
    return false; // Return false to chain to the original malloc
}

// Custom free hook
mla_bool_t my_free_hook(mla_pointer_t ptr) {
    // Your custom deallocation logic here
    return false; // Return false to chain to the original free
}

// Install the hook
mla_memory_hook_t my_hook = mla_memory_hook_install(my_malloc_hook, my_free_hook, nullptr);
```

### Uninstalling a Memory Hook

To uninstall a memory hook, simply call `mla_memory_hook_uninstall` with the handle that was returned by `mla_memory_hook_install`.

```cpp
// Uninstall the hook
mla_memory_hook_uninstall(my_hook);
```