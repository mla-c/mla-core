# Utilities Module

The Utilities module provides a collection of miscellaneous helper functions and data structures that are used throughout the CoreOS library.

## Components

### Character Utilities (`mla_char_utils.h`)

Provides a set of functions for character manipulation, such as converting between uppercase and lowercase, and checking for whitespace.

```cpp
#include "mla_char_utils.h"

mla_char_t lower = mla_char_toLower('A'); // 'a'
mla_char_t upper = mla_char_toUpper('b'); // 'B'
mla_bool_t is_space = mla_char_is_whitespace(' '); // true
```

### Endian Utilities (`mla_endian_utils.h`)

Provides functions for converting between big-endian and little-endian byte orders.

```cpp
#include "mla_endian_utils.h"

mla_uint16_t value = 0x1234;
mla_uint16_t swapped = mla_endian_swap16(value); // 0x3412
```
