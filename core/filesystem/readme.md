# Filesystem Module

The Filesystem module provides a unified interface for interacting with different file systems. It supports mounting multiple file systems at different paths and provides a consistent API for file and directory operations.

## Architecture

The Filesystem module has two main components:

- **`mla_file_system_t`**: A virtual file system interface that defines a set of function pointers for file and directory operations. You can implement this interface to create your own custom file system.
- **`mla_fs_*` functions**: A set of global functions that provide a high-level API for file and directory operations. These functions automatically resolve the correct mounted file system based on the provided path.

### Mounting a File System

To use a file system, you must first mount it at a specific path using `mla_file_system_initialize`.

```cpp
#include "mla_file_system.h"

// Create a file system implementation
mla_file_system_t my_fs = {
    .file_exists = my_file_exists_func,
    .open_file = my_open_file_func,
    // ... other function pointers
};

// Mount the file system at the root path
mla_file_system_initialize(mla_string("/"), my_fs);
```

## Usage

### File Operations

The Filesystem module provides a comprehensive set of functions for file operations.

#### Checking for a File's Existence

```cpp
if (mla_fs_file_exists(mla_string("/path/to/file.txt"))) {
    // File exists
}
```

#### Creating and Writing to a File

```cpp
mla_file_system_stream_t stream;
if (mla_fs_open_file(mla_string("/path/to/new_file.txt"), MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE, stream)) {
    const char* data = "Hello, world!";
    stream.write(stream, 0, strlen(data), (const mla_byte_t*)data);
}
```

#### Reading from a File

```cpp
mla_file_system_stream_t stream;
if (mla_fs_open_file(mla_string("/path/to/file.txt"), MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ, stream)) {
    mla_size_t length = stream.length(stream);
    mla_byte_t* buffer = (mla_byte_t*)malloc(length);
    stream.read(stream, 0, length, buffer);
    free(buffer);
}
```

#### Deleting a File

```cpp
mla_fs_delete_file(mla_string("/path/to/file.txt"));
```

### Directory Operations

The Filesystem module also provides functions for directory operations.

#### Creating a Directory

```cpp
mla_fs_create_directory(mla_string("/path/to/new_directory/"));
```

#### Checking for a Directory's Existence

```cpp
if (mla_fs_directory_exists(mla_string("/path/to/directory/"))) {
    // Directory exists
}
```

#### Listing Directory Contents

```cpp
mla_array_list_t<mla_string_t, mla_string_initializer> entries;
if (mla_fs_list_directory(mla_string("/path/to/directory/"), entries)) {
    for (mla_size_t i = 0; i < mla_array_list_size(entries); ++i) {
        // Process entry
    }
}
```

#### Deleting a Directory

```cpp
mla_fs_delete_directory(mla_string("/path/to/directory/"));
```

### Path Manipulation

The Filesystem module provides several helper functions for path manipulation.

```cpp
mla_string_t parent = mla_fs_get_parent_directory(mla_string("/path/to/file.txt")); // returns "/path/to/"
mla_string_t filename = mla_fs_get_file_name(mla_string("/path/to/file.txt")); // returns "file.txt"
mla_string_t extension = mla_fs_get_file_extension(mla_string("/path/to/file.txt")); // returns "txt"
mla_string_t combined = mla_fs_combine_paths(mla_string("/path"), mla_string("to/file.txt")); // returns "/path/to/file.txt"
```
