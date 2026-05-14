# System Module

The System module provides foundational data structures used across CoreOS.

## Headers in This Module

- `mla_string.h` - String type and string helpers
- `mla_string_concat.h` - String concatenation utilities
- `mla_buffer.h` - Buffer management
- `mla_bytes.h` - Byte sequence helpers
- `mla_number.h` - Numeric parsing/conversion helpers
- `mla_stream.h` - Input/output stream abstractions
- `mla_array_list.h` - Dynamic array list
- `mla_link_list.h` - Linked-list container
- `mla_hash_map.h` - Hash map container
- `mla_id.h` - ID generation helpers
- `mla_user_data.h` - Generic user data storage

## Source Files in This Module

- String: `mla_string.cpp`, `mla_string_concat.cpp`, `mla_string_convert.cpp`, `mla_string_utf.cpp`
- Streams: `mla_stream.cpp`, `mla_stream_helper.cpp`, `mla_stream_wrapper.cpp`, `mla_stream_deflate.cpp`
- Core structures: `mla_buffer.cpp`, `mla_bytes.cpp`, `mla_number.cpp`, `mla_id.cpp`, `mla_user_data.cpp`

## Related Tests

See `core-os-test/`:
- `mla_string_test.h`
- `mla_buffer_test.h`
- `mla_bytes_test.h`
- `mla_number_test.h`
- `mla_stream_test.h`
- `mla_stream_deflate_test.h`
- `mla_id_test.h`
- `mla_user_data_test.h`
- `native_string_test.h`
- `native_list_test.h`
