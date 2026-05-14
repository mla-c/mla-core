# Serializer Module

The Serializer module provides a flexible framework for serializing and deserializing data to and from various formats. It supports JSON, Binary, and XML serialization out of the box.

## Architecture

The module is centered around the `mla_serializer_t` and `mla_deserializer_t` structures, which define a generic interface for serialization and deserialization. Concrete implementations for JSON (`mla_json_serializer.h`), binary (`mla_binary_serializer.h`), and XML (`mla_xml_serializer.h`) formats are provided.

The serialization process is driven by a `mla_serialize_definition_t` structure, which contains functions for reading and writing a specific data type.

## Supported Formats

### JSON (`mla_json_serializer.h`)
- Human-readable text format
- Standard JSON structure with objects and arrays
- Bytes are encoded as base64 strings

### Binary (`mla_binary_serializer.h`)
- Compact binary format
- Little-endian encoding
- Most efficient for size and performance

### XML (`mla_xml_serializer.h`)
- XML format with a specific structure
- Simple types (bool, numbers, strings) are serialized as **attributes**
- Complex types (structs, lists) are serialized as **tags** (elements)
- Deserializer supports reading values from both attributes and tags
- Example output:
  ```xml
  <item int_val="42" bool_val="true" str_val="Hello">
    <innerStruct int_val="10" bool_val="false" />
    <list>
      <item value="1" />
      <item value="2" />
      <item value="3" />
    </list>
  </item>
  ```

## Usage

### Defining a Serializable Struct

To make a struct serializable, you need to define `serialize` and `deserialize` functions for it.

```cpp
#include "mla_serializer.h"

struct my_data_t {
    mla_int32_t value;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_platform_pointer_t obj) {
        const my_data_t* data = (const my_data_t*)obj;
        serializer.write_start_struct(serializer);
        mla_serializer_write_int32(serializer, mla_string_const("value"), data->value);
        serializer.write_end_struct(serializer);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_platform_pointer_t obj, const mla_string_t& property_name) {
        my_data_t* data = (my_data_t*)obj;
        if (mla_string_equals(property_name, mla_string_const("value"))) {
            mla_deserializer_read_int32(deserializer, data->value);
        }
        return MLA_DESERIALIZER_READ_SKIPPED;
    }
};
```

### Serializing to JSON

```cpp
#include "mla_json_serializer.h"
#include "mla_stream.h"

my_data_t data = { 42 };
mla_stream_output_t stream = mla_stream_output_from_buffer(...);
mla_serializer_t serializer = mla_json_serializer(stream);

mla_serialize_definition_t def = mla_serialize_definition<my_data_t>();
def.write_function(serializer, &data);
```

### Deserializing from JSON

```cpp
#include "mla_json_serializer.h"
#include "mla_stream.h"

mla_stream_input_t stream = mla_stream_input_from_buffer(...);
mla_deserializer_t deserializer = mla_json_deserializer(stream);

my_data_t data;
mla_serialize_definition_t def = mla_serialize_definition<my_data_t>();
mla_deserializer_read_struct(deserializer, &data, def.read_function);
```

### Serializing to XML

```cpp
#include "mla_xml_serializer.h"
#include "mla_stream.h"

my_data_t data = { 42 };
mla_stream_output_t stream = mla_stream_output_from_buffer(...);
mla_serializer_t serializer = mla_xml_serializer(stream);

mla_serialize_definition_t def = mla_serialize_definition<my_data_t>();
def.write_function(serializer, &data);
```

### Deserializing from XML

```cpp
#include "mla_xml_serializer.h"
#include "mla_stream.h"

mla_stream_input_t stream = mla_stream_input_from_buffer(...);
mla_deserializer_t deserializer = mla_xml_deserializer(stream);

my_data_t data;
mla_serialize_definition_t def = mla_serialize_definition<my_data_t>();
mla_deserializer_read_struct(deserializer, &data, def.read_function);
```

## XML Serialization Details

The XML serializer uses a specific format optimized for the serialization framework:

- **Simple values** (bool, numbers, strings, bytes) are written as XML **attributes**
- **Complex values** (structs, lists) are written as XML **tags**
- Structs are represented as `<item>` tags
- Lists are represented as `<list>` tags
- Bytes are base64-encoded
- XML entities are properly escaped (`<`, `>`, `&`, `"`, `'`)

### Example XML Output

```xml
<item boolValue="true" int8Value="-8" int16Value="-16" int32Value="-32" 
      int64Value="-64" uint8Value="8" uint16Value="16" uint32Value="32" 
      uint64Value="64" floatValue="3.14" doubleValue="3.141593" 
      stringValue="Test String">
  <innerStruct int32Value="0" boolValue="false" />
  <intList>
    <item int32Value="1" />
    <item int32Value="2" />
    <item int32Value="3" />
  </intList>
</item>
```

## Project Conventions (Latest)

This module follows the current mla-c project-wide conventions:

- **Code style:** Use MLA data types and naming conventions described in the [main README](../../README.md).
- **Heap ownership:** Use `mla_pointer_t` for owned heap allocations; use raw platform pointers only for transient access.

For full details, see:
- [Main project README](../../README.md)
- [Core data types documentation](../../core/readme.md)
- [Memory module documentation](../../core/memory/readme.md)

