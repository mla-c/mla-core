# Reflection Module

The Reflection module provides runtime type information and metadata capabilities for C++ structs. It enables introspection of struct fields, types, and properties, which is essential for serialization, RPC, and dynamic data handling.

## Architecture

The Reflection module consists of the following key components:

- **`mla_reflection_type_t`**: An enumeration that defines all supported reflection types, including primitives (bool, integers, floats), strings, bytes, structs, and lists.
- **`mla_reflection_struct_field_t`**: Represents a single field within a struct, including its name, offset, type, and optional nested struct metadata.
- **`mla_reflection_struct_metadata_t`**: Contains complete metadata for a struct, including its name, size, and collection of fields.
- **`mla_reflection_register_struct`**: Registers struct metadata with the reflection system, making it available for runtime introspection.

## Usage

### Defining Struct Metadata

To make a struct reflectable, you need to define a static `metadata()` function that returns the struct's metadata.

```cpp
#include "mla_reflection.h"

struct my_data_t {
    mla_int32_t id;
    mla_string_t name;
    mla_float_t value;

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_data_t);
        
        mla_reflection_struct_metadata_add_field(meta, 
            mla_reflection_struct_field(
                mla_string_const("id"),
                offsetof(my_data_t, id),
                MLA_REFLECTION_TYPE_INT32,
                MLA_REFLECTION_TYPE_INVALID,
                nullptr
            )
        );
        
        mla_reflection_struct_metadata_add_field(meta,
            mla_reflection_struct_field(
                mla_string_const("name"),
                offsetof(my_data_t, name),
                MLA_REFLECTION_TYPE_STRING,
                MLA_REFLECTION_TYPE_INVALID,
                nullptr
            )
        );
        
        mla_reflection_struct_metadata_add_field(meta,
            mla_reflection_struct_field(
                mla_string_const("value"),
                offsetof(my_data_t, value),
                MLA_REFLECTION_TYPE_FLOAT,
                MLA_REFLECTION_TYPE_INVALID,
                nullptr
            )
        );
        
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};
```

### Registering a Struct

Once you've defined the metadata, you need to register the struct with the reflection system.

```cpp
// Register the struct manually
mla_reflection_register_struct<my_data_t>();

// Or use the auto-registration macro
mla_reflection_auto_register_struct(my_data_t);
```

### Querying Struct Metadata

After registration, you can query metadata at runtime.

```cpp
mla_reflection_struct_metadata_t metadata;
if (mla_reflection_get_struct_metadata(mla_string_const("my_data_t"), metadata)) {
    // Access struct name, size, and fields
    for (mla_size_t i = 0; i < mla_array_list_size(metadata.fields); ++i) {
        mla_reflection_struct_field_t field = mla_array_list_get(metadata.fields, i);
        // Process field information
    }
}
```

### Nested Structs

The reflection system supports nested structs through struct metadata providers.

```cpp
struct nested_data_t {
    my_data_t inner;
    mla_int32_t outer_value;
    
    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(nested_data_t);
        
        mla_reflection_struct_metadata_add_field(meta,
            mla_reflection_struct_field(
                mla_string_const("inner"),
                offsetof(nested_data_t, inner),
                MLA_REFLECTION_TYPE_STRUCT,
                MLA_REFLECTION_TYPE_INVALID,
                my_data_t::metadata  // Provide nested struct metadata
            )
        );
        
        // Add other fields...
        
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};
```

## Integration with RPC

The Reflection module integrates with the RPC module to enable automatic serialization and remote procedure calls. See `mla_reflection_rpc.h` for RPC-specific reflection utilities.
