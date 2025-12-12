# Serializer Module

The Serializer module provides a flexible framework for serializing and deserializing data to and from various formats. It supports both JSON and binary serialization out of the box.

## Architecture

The module is centered around the `mla_serializer_t` and `mla_deserializer_t` structures, which define a generic interface for serialization and deserialization. Concrete implementations for JSON (`mla_json_serializer.h`) and binary (`mla_binary_serializer.h`) formats are provided.

The serialization process is driven by a `mla_serialize_definition_t` structure, which contains functions for reading and writing a specific data type.

## Usage

### Defining a Serializable Struct

To make a struct serializable, you need to define `serialize` and `deserialize` functions for it.

```cpp
#include "mla_serializer.h"

struct my_data_t {
    mla_int32_t value;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
        const my_data_t* data = (const my_data_t*)obj;
        serializer.write_start_struct(serializer);
        mla_serializer_write_int32(serializer, mla_string_const("value"), data->value);
        serializer.write_end_struct(serializer);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {
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
