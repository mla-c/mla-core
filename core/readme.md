# mla-c Data Types

The `mla_data_types.h` header file defines a set of core data types and macros that are used throughout the mla-c library. These types and macros provide a consistent and platform-independent way to work with primitive data types, memory, and other low-level operations.

## Data Types

The header defines a set of fixed-width integer types, floating-point types, and character types.

- **`mla_bool_t`**: A boolean type.
- **`mla_int8_t`, `mla_uint8_t`**: 8-bit signed and unsigned integers.
- **`mla_int16_t`, `mla_uint16_t`**: 16-bit signed and unsigned integers.
- **`mla_int32_t`, `mla_uint32_t`**: 32-bit signed and unsigned integers.
- **`mla_int64_t`, `mla_uint64_t`**: 64-bit signed and unsigned integers.
- **`mla_float_t`, `mla_double_t`**: 32-bit and 64-bit floating-point numbers.
- **`mla_char_t`**: A character type, typically `char`.
- **`mla_utf_16_char_t`, `mla_utf_32_char_t`**: 16-bit and 32-bit character types for UTF-16 and UTF-32 strings, respectively.

## Macros

The header also defines a number of useful macros.

### Memory Operations

- **`mla_memcpy`**: A wrapper for `memcpy`.
- **`mla_memset`**: A wrapper for `memset`.
- **`mla_memcmp`**: A wrapper for `memcmp`.
- **`mla_memmove`**: A wrapper for `memmove`.
- **`mla_platform_malloc`**: A wrapper for `malloc` that includes error checking.
- **`mla_platform_free`**: A wrapper for `free`.

### String Operations

- **`mla_strcpy`**: A wrapper for `strcpy`.
- **`mla_strlen`**: A wrapper for `strlen`.
- **`mla_strstr`**: A wrapper for `strstr`.

### Other Macros

- **`mla_min`, `mla_max`**: Macros for finding the minimum and maximum of two values.
- **`__FILENAME_ONLY__`**: A macro that expands to the base name of the current file.
- **`mla_print`**: A macro to print to the Standard Out.

## Low-Level Operations

The `mla_low_level_operations_t` struct defines a set of function pointers for low-level operations such as memory allocation, string manipulation, and I/O. This allows the mla-c library to be easily ported to different platforms by providing a custom implementation of these operations.
