# Task Management Module

The Task Management module provides a cooperative multitasking system, allowing for the creation, scheduling, and management of concurrent tasks. It supports both one-time and repeating tasks, along with priority-based scheduling.

## Architecture

The module is composed of two primary components:

- **`mla_task_t`**: Represents a single task, containing its name, worker function, priority, and state.
- **`mla_task_manager_t`**: A global manager responsible for registering, scheduling, and executing tasks.

## Usage

### Creating a Repeating Task

Repeating tasks execute periodically until they signal completion.

```cpp
#include "mla_task.h"
#include "mla_task_manager.h"

// Define a repeating task worker
mla_task_process_result_state my_repeating_task(mla_callback_userdata userData) {
    // Do work...
    return TASK_STATE_SLEEP; // Or TASK_STATE_DONE
}

// Create and register the task
mla_task_t task = mla_task_repeating(mla_string_const("my_repeating_task"), my_repeating_task);
mla_task_manager_register_task(task);
```

### Creating a One-Time Task

One-time tasks execute once and are then removed.

```cpp
#include "mla_task.h"
#include "mla_task_manager.h"

// Define a one-time task worker
void my_one_time_task(mla_callback_userdata userData) {
    // Do work...
}

// Create and register the task
mla_task_t task = mla_task_one_time(mla_string_const("my_one_time_task"), my_one_time_task);
mla_task_manager_register_task(task);
```

### Processing Tasks

The task manager needs to be periodically invoked to process the task queue.

```cpp
// In your main loop
while (true) {
    mla_task_manager_process_all_tasks();
}
```
