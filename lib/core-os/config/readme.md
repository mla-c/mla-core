# Config Module

The Config module provides a flexible and persistent way to manage application configuration. It allows you to define custom data structures for your configuration, which can be easily read from and written to a persistent storage.

## Architecture

The Config module is built on top of the Serializer module and consists of three main parts:

- **`mla_config_definition_t`**: This structure defines a configuration, including its name and serialization/deserialization functions. You create a definition for each configuration structure you want to manage.

- **`mla_config_manager_read`**: This function reads a configuration from persistent storage. It takes a configuration definition and a pointer to the data structure to populate.

- **`mla_config_manager_write`**: This function writes a configuration to persistent storage. It takes a configuration definition and a pointer to the data structure to save.

To use the Config module, you need to provide serialize and deserialize functions for your configuration data structures. These functions are responsible for converting your data to and from a serialized format.

## Usage

### Defining a Configuration Structure

First, define a `struct` for your configuration and implement the `serialize` and `deserialize` functions.

```cpp
#include "mla_config.h"
#include "mla_serializer.h"

struct test_config_struct {
    mla_int32_t intValue;
    mla_bool_t boolValue;
    mla_string_t strValue;

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t config, const mla_string_t& property_name) {
        test_config_struct* obj = static_cast<test_config_struct*>(config);
        if (mla_string_equals_const(property_name, "intValue")) {
            mla_deserializer_read_int32(deserializer, obj->intValue);
        } else if (mla_string_equals_const(property_name, "boolValue")) {
            mla_deserializer_read_bool(deserializer, obj->boolValue);
        } else if (mla_string_equals_const(property_name, "strValue")) {
            mla_deserializer_read_string(deserializer, obj->strValue);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
        return MLA_DESERIALIZER_READ_HANDLED;
    }

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t config) {
        const test_config_struct* obj = static_cast<const test_config_struct*>(config);
        mla_serializer_write_int32(serializer, mla_string_const("intValue"), obj->intValue);
        mla_serializer_write_bool(serializer, mla_string_const("boolValue"), obj->boolValue);
        mla_serializer_write_string(serializer, mla_string_const("strValue"), obj->strValue);
        return true;
    }
};
```

### Creating a Configuration Definition

Next, create a configuration definition that links your structure with its serialization functions.

```cpp
mla_serialize_definition_t test_config_struct_serialize_def() {
    return mla_serialize_definition<test_config_struct>();
}

mla_config_definition_t def = mla_config_definition<test_config_struct>(
    mla_string_const("test_config"), test_config_struct_serialize_def());
```

### Writing and Reading Configuration

Now you can use `mla_config_manager_write` and `mla_config_manager_read` to save and load your configuration.

```cpp
// Create and populate a config structure
test_config_struct original = {42, true, mla_string("ConfigTest")};

// Write the config
mla_config_manager_write(def, &original);

// Create an empty structure to load the config into
test_config_struct loaded = {0, false, mla_string_empty()};

// Read the config
mla_config_manager_read(def, &loaded);
```
