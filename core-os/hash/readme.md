# Hash Module

The Hash module provides a collection of hashing functions for generating unique identifiers for data. It includes a SHA-1 implementation for cryptographic hashing and a set of non-cryptographic hash functions for use in data structures like hash tables.

## Architecture

The Hash module consists of two main parts:

- **`mla_sha1`**: A function that computes the SHA-1 hash of a given input. It takes a `mla_bytes_t` as input and returns a 20-byte SHA-1 hash.

- **`mla_int32_hash_t` and `mla_int16_hash_t`**: These are structs that provide non-cryptographic hash functions for 32-bit and 16-bit integers, respectively. They are designed to be used with hash maps and other hash-based data structures.

## Usage

### SHA-1 Hashing

To compute the SHA-1 hash of a string or a block of data, use the `mla_sha1` function.

```cpp
#include "mla_sha1.h"
#include "mla_string.h"

// Create a string to hash
mla_string_t str = mla_string_const("hello, world!");

// Compute the SHA-1 hash
mla_bytes_t hash = mla_sha1(mla_bytes_from_string(str));

// The 'hash' variable now contains the 20-byte SHA-1 hash
```

### Integer Hashing

The integer hash functions are typically used internally by hash maps, but you can also use them directly if needed.

```cpp
#include "mla_hash.h"

// Hash a 32-bit integer
mla_int32_t my_int = 12345;
mla_size_t hash_value = mla_int32_hash_t::hash(my_int);
```