# URL Module

The URL module provides functionality for parsing and manipulating URLs. It allows you to break down a URL string into its constituent parts, such as the scheme, host, port, path, query parameters, and fragment.

## Architecture

The main data structure is `mla_url_t`, which represents a parsed URL. The module provides functions to parse a URL string into an `mla_url_t` structure and to convert an `mla_url_t` structure back into a string.

## Usage

### Parsing a URL

To parse a URL, you can use the `mla_url_parse` function.

```cpp
#include "mla_url.h"

mla_string_t url_string = mla_string_const("http://www.example.com:8080/path?key=value#fragment");
mla_url_t url;
if (mla_url_parse(url_string, url)) {
    // Access the parsed components
    // url.scheme, url.host, url.port, url.path, url.query, url.fragment
}
```

### Converting a URL to a String

To convert a URL structure back to a string, you can use the `mla_url_to_string` function.

```cpp
#include "mla_url.h"

mla_url_t url = mla_url_empty();
url.scheme = mla_string_const("https");
url.host = mla_string_const("example.com");
url.path = mla_string_const("/resource");

mla_string_t url_string = mla_url_to_string(url);
// url_string will be "https://example.com/resource"
```

## Project Conventions (Latest)

This module follows the current mla-c project-wide conventions:

- **Code style:** Use MLA data types and naming conventions described in the [main README](../../README.md).
- **Heap ownership:** Use `mla_pointer_t` for owned heap allocations; use raw platform pointers only for transient access.

For full details, see:
- [Main project README](../../README.md)
- [Core data types documentation](../../core/readme.md)
- [Memory module documentation](../../core/memory/readme.md)

