---
applyTo: '**'
description: 'Patterns for using I/O streams and stream wrappers in the MLA framework'
---

# Streams

The stream module (`core-os/system/mla_stream.h`) provides a callback-based I/O abstraction. Every stream is a lightweight struct containing function pointers for reading or writing, plus a `mla_user_data_t` for state. Streams are the backbone of serialisation, HTTP, file I/O, and compression.

## Key Types

| Type | Purpose |
|---|---|
| `mla_stream_input_t` | Readable byte stream (read + remaining_bytes callbacks) |
| `mla_stream_output_t` | Writable byte stream (write + available_bytes callbacks) |
| `mla_memory_stream_t` | In-memory stream with both input and output |

## Stream Structs

```cpp
struct mla_stream_input_t {
    mla_user_data_t userdata;
    mla_size_t (*read)(mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer);
    mla_size_t (*remaining_bytes)(mla_stream_input_t& input); // may return mla_size_max if unknown
};

struct mla_stream_output_t {
    mla_user_data_t userdata;
    mla_size_t (*write)(mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
    mla_size_t (*available_bytes)(mla_stream_output_t& output); // may return mla_size_max if unknown
};
```

## Creating Streams

### No-op Streams

Useful as default / sentinel values:

```cpp
mla_stream_input_t  noopIn  = mla_stream_noop_input();
mla_stream_output_t noopOut = mla_stream_noop_output();
```

### Console Streams (stdin / stdout)

```cpp
mla_stream_input_t  in  = mla_stream_input_stdin();
mla_stream_output_t out = mla_stream_output_stdout();
```

### Buffer Streams

```cpp
mla_byte_t buf[256];

// Non-owning — caller manages buffer lifetime
mla_stream_input_t  bufIn  = mla_stream_input_from_buffer(buf, sizeof(buf));
mla_stream_output_t bufOut = mla_stream_output_to_buffer(buf, sizeof(buf));

// Owning — stream allocates and manages its own buffer
mla_stream_input_t  ownedIn  = mla_stream_input_from_buffer(1024);
mla_stream_output_t ownedOut = mla_stream_output_to_buffer(1024);
```

### String Input Stream

```cpp
mla_string_t json = mla_string_const("{\"key\":\"value\"}");
mla_stream_input_t strIn = mla_stream_input_from_string(json);
```

### Memory Stream (Read + Write)

```cpp
mla_memory_stream_t mem = mla_memory_stream(4096); // initial size, auto-grows

// Write through mem.output, read back through mem.input
mla_stream_output_write_string(mem.output, mla_string_const("Hello"));

mla_size_t pos  = mla_memory_stream_get_position(mem);
mla_size_t size = mla_memory_stream_get_size(mem);
mla_memory_stream_set_position(mem, 0);
mla_memory_stream_reset(mem);
```

## Copying Between Streams

```cpp
mla_bool_t ok = mla_stream_copy(inputStream, outputStream);
```

Copies all remaining bytes from `inputStream` to `outputStream`.

## Writing a String to an Output Stream

```cpp
mla_stream_output_write_string(outputStream, mla_string_const("Hello\n"));
```

## Stream Wrappers

Wrappers decorate an existing stream with additional behaviour. The original stream must remain valid for the lifetime of the wrapper.

### Timeout Wrapper (Input)

Adds a per-read timeout to a blocking input stream:

```cpp
mla_stream_input_t timed = mla_stream_input_timeout_wrapper(rawInput, 5000); // 5 s timeout
```

Or perform a single read with a timeout:

```cpp
mla_size_t bytesRead = mla_stream_input_read_with_timeout(input, offset, length, buffer, 5000);
```

### Limited Wrapper (Input)

Restricts the number of bytes that can be read:

```cpp
mla_stream_input_t limited = mla_stream_input_limited_wrapper(input, 1024); // max 1 KB
```

### Buffered Wrappers

Add an intermediate buffer to reduce the number of underlying read/write calls.

**Input buffering** — reduces small, frequent reads:

```cpp
mla_stream_input_t buffIn = mla_stream_input_buffered_wrapper(input, 4096);
```

**Output buffering** — batches small writes into larger chunks:

```cpp
mla_stream_output_t buffOut = mla_stream_output_buffered_wrapper(output, 4096);
// … write data to buffOut …
// Flush the output buffer when finished writing
mla_stream_output_flush_buffered_wrapper(buffOut);
```

### Deflate Compression / Decompression

```cpp
// Compress data written to the output stream
mla_stream_output_t compressed = mla_stream_output_deflate_compress_wrapper(rawOutput);
// … write data to compressed …
mla_stream_output_deflate_finish(compressed); // flush & finalise

// Decompress data read from the input stream
mla_stream_input_t decompressed = mla_stream_input_deflate_decompress_wrapper(rawInput);
// … read decompressed bytes …
```

### Interceptor Wrappers (Advanced)

Intercept read or write calls for custom processing (e.g., hashing, logging):

```cpp
// Input interceptor
static mla_size_t my_read_interceptor(
    mla_stream_input_t& wrapper,
    mla_stream_input_t& input,
    mla_size_t offset, mla_size_t length, const mla_byte_t* buffer)
{
    // buffer already contains bytes read from the underlying input
    // inspect / transform / log them here
    return length; // return bytes consumed
}

mla_stream_input_t intercepted = mla_stream_input_interceptor_wrapper(
    input,
    my_read_interceptor,
    nullptr // optional remaining_bytes interceptor
);

// Output interceptor follows the same pattern with mla_stream_output_interceptor_wrapper
```

### Size Calculation Stream

Discards all data but tracks how many bytes were written — useful for pre-computing content lengths:

```cpp
mla_stream_output_t sizeCalc = mla_stream_output_size_calculation();
// … write data to sizeCalc …
mla_size_t totalBytes = mla_stream_output_size_calculation_get_size(sizeCalc);
```

## Common Patterns

### Serialize to JSON String

```cpp
mla_memory_stream_t mem = mla_memory_stream(256);
mla_serializer_t serializer = mla_json_serializer(mem.output);
auto def = mla_serialize_definition<MyStruct>();
mla_serialize(serializer, def, &myData);
// Read the JSON back from mem.input
```

### Read HTTP Response Body

```cpp
// response.content is an mla_stream_input_t
mla_byte_t buf[512];
mla_size_t bytesRead = response.content.read(response.content, 0, sizeof(buf), buf);
```

## Rules

- Streams are **not thread-safe** by default. Guard concurrent access with a mutex.
- The underlying resource (buffer, file descriptor, socket) must outlive the stream and all its wrappers.
- Always call `mla_stream_output_deflate_finish` after writing all data to a deflate wrapper.
- Always call `mla_stream_output_flush_buffered_wrapper` before closing a buffered output.
- `remaining_bytes` / `available_bytes` may return `mla_size_max` when the total size is unknown (e.g., network streams).

## Incorrect Usage

```cpp
// ❌ Using FILE* or fread / fwrite directly
FILE* f = fopen("data.bin", "rb"); // use mla_fs_open_file instead

// ❌ Forgetting to finish deflate compression
mla_stream_output_t comp = mla_stream_output_deflate_compress_wrapper(out);
// wrote data but never called mla_stream_output_deflate_finish(comp)

// ❌ Destroying the underlying buffer before the wrapper
mla_byte_t* buf = (mla_byte_t*)mla_platform_malloc(1024);
mla_stream_input_t in = mla_stream_input_from_buffer(buf, 1024);
mla_platform_free(buf); // in now points to freed memory!

// ❌ Using printf for output instead of stream writes
printf("result: %d\n", value); // use mla_stream_output_write_string
```
