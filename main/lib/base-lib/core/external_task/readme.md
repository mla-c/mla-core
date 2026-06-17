# External Task Module

The External Task module provides a cross-platform abstraction for starting and interacting with external OS processes. It exposes process stdin/stdout as MLA streams and tracks process lifetime through managed resource ownership.

## Architecture

The module consists of a platform-independent API in `mla_external_task.h` and platform-specific implementations that are bound through `g_external_task_management`.

### Key Components

- **`mla_external_task_t`**: Represents one external process instance including command line, managed native resource, stdout input stream, and stdin output stream.
- **`mla_external_task_management_t`**: Low-level platform hooks for process creation, state querying, stop, stdout read, and stdin write.
- **Managed native resource ownership**: Process-specific native state is wrapped in `mla_pointer_t` and cleaned up through `mla_native_resource_to_managed_pointer(...)` cleanup hooks.

## Usage

### Creating and Reading Output

```cpp
#include "mla_external_task.h"

mla_external_task_t task = mla_external_task_create(mla_string_const("printf 'hello'"));

mla_byte_t buffer[5];
mla_size_t read = task.std_out.read(task.std_out, 0, 5, buffer);
```

### Writing Input

```cpp
#include "mla_external_task.h"

mla_external_task_t task = mla_external_task_create(mla_string_const("cat"));

const mla_byte_t* data = mla_r_cast<const mla_byte_t*>("ping\n");
mla_size_t written = task.std_in.write(task.std_in, 0, 5, data);
```

### State and Shutdown

```cpp
#include "mla_external_task.h"

mla_external_task_state state = mla_external_task_get_state(task);
if (state == MLA_EXTERNAL_TASK_STATE_RUNNING) {
    mla_external_task_stop(task);
}
```

## Platform Support

- Linux: `lib/base-lib/platform/linux/mla_global_external_task_linux.h`
- Windows: `lib/base-lib/platform/windows/mla_global_external_task_windows.h`

Platforms without external task support can omit `SOURCE_EXTERNAL_TASK_FILES` and/or leave `g_external_task_management` unbound.
