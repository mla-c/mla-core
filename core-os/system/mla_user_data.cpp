//
// Created by chris on 2/9/2026.
//

#include "mla_user_data.h"

#include "mla_array_list.h"
#include "mla_string_concat.h"
#include "../mla_data_types.h"
#include "../log/mla_logging.h"

#define mla_user_data_name_empty ""
#define mla_user_data_name_list "udatas"

struct mla_user_data_list_t {
    mla_array_list_t<mla_user_data_t, mla_user_data_initializer> datas;
};

struct mla_user_data_list_initializer {
    static mla_user_data_list_t init() {
        return {mla_array_list_empty<mla_user_data_t, mla_user_data_initializer>()};
    }
};

mla_user_data_t mla_user_data_empty() {
    return {
        mla_user_data_name_empty,
        mla_buffer_reference_noOwner(),
        mla_dynamic_data_empty()
    };
}

mla_bool_t __mla_user_data_name_equal(const mla_char_t a[mla_user_data_name_size],
                                      const mla_char_t b[mla_user_data_name_size]) {

    // Compare the names character by character and check for the null terminator to determine if they are equal
    for (mla_size_t i = 0; i < mla_user_data_name_size; ++i) {

        if (a[i] != b[i]) {
            return false; // Characters differ, names are not equal
        }

        if (a[i] == '\0') {
            return true; // Both names are equal up to the null terminator
        }
    }

    return false;
}

mla_bool_t __mla_user_data_manage_external_resource(mla_user_data_list_t* list) {
    for (mla_size_t i = 0; i < mla_array_list_size(list->datas); ++i) {

        mla_user_data_t& item = mla_array_list_get_unsafe(list->datas, i);

        if (!mla_buffer_reference_is_noOwner(item.dataOwner)) {
            return true;
        }
    }

    return false;
}

mla_user_data_t* __mla_user_data_get_for_update(mla_user_data_t &target, const mla_char_t name[mla_user_data_name_size]) {

    if (__mla_user_data_name_equal(target.name, mla_user_data_name_empty)) {
        return &target;
    }

    if (__mla_user_data_name_equal(target.name, mla_user_data_name_list)) {

        mla_user_data_list_t* list = (mla_user_data_list_t*)target.data.asPointer;
        if (list == nullptr) {
            mla_error(mla_string_concat("User data list is null for name: ", name));
            return nullptr;
        }

        for (mla_size_t i = 0; i < mla_array_list_size(list->datas); ++i) {

            mla_user_data_t* item = mla_array_list_get_ref_unsafe(list->datas, i);
            if (__mla_user_data_name_equal(item->name, name)) {
                return item;
            }
        }

        mla_user_data_t newItem = mla_user_data_empty();
        if (!mla_array_list_add(list->datas, newItem)) {
            mla_error(mla_string_concat("Failed to add user data to list for name: ", name));
            return nullptr;
        }

        return mla_array_list_get_ref_unsafe(list->datas, mla_array_list_size(list->datas) - 1);

    }

    if (__mla_user_data_name_equal(target.name, name)) {

        // is the same name, we can overwrite it
        return &target;
    }

    // Convert to list
    mla_user_data_list_t* list = (mla_user_data_list_t*)mla_malloc(sizeof(mla_user_data_list_t));

    if (list == nullptr) {
        return nullptr;
    }

    mla_memset(list, 0, sizeof(mla_user_data_list_t));
    list->datas = mla_array_list<mla_user_data_t, mla_user_data_initializer>(1);

    // Add the existing item to the list
    mla_user_data_t existingItem = target;
    if (!mla_array_list_add(list->datas, existingItem)) {
        mla_error(mla_string_concat("Failed to add existing user data to list for name: ", target.name));
        mla_free(list);
        return nullptr;
    }

    mla_user_data_t newItem = mla_user_data_empty();
    if (!mla_array_list_add(list->datas, newItem)) {
        mla_error(mla_string_concat("Failed to add user data to list for name: ", name));
        mla_free(list);
        return nullptr;
    }

    // Determine if we need to manage external resources based on the existing items in the list
    mla_bool_t managedExternalResources = __mla_user_data_manage_external_resource(list);

    // Update the target to be the list
    mla_memcpy(target.name, mla_user_data_name_list, mla_user_data_name_size);
    target.dataOwner = mla_buffer_reference_create(list, managedExternalResources, mla_buffer_default_cleanup<mla_user_data_list_t, mla_user_data_list_initializer>, mla_dynamic_data_empty());
    target.data.asPointer = list;

    return mla_array_list_get_ref_unsafe(list->datas, mla_array_list_size(list->datas) - 1);

}


void __mla_user_data_update_manage_external_resource(mla_user_data_t &target, mla_user_data_t* newData) {

    if (mla_buffer_reference_is_noOwner(newData->dataOwner)) {
        return; // No ownership, no need to update
    }

    if (!mla_buffer_reference_is_noOwner(target.dataOwner)) {
        return; // Already managing external resources, no need to update
    }

    if (!__mla_user_data_name_equal(target.name, mla_user_data_name_list)) {
        return;
    }

    mla_user_data_list_t* list = (mla_user_data_list_t*)target.data.asPointer;
    if (list == nullptr) {
        return; // List is null, cannot update
    }

    target.dataOwner = mla_buffer_reference_create(list, true, mla_buffer_default_cleanup<mla_user_data_list_t, mla_user_data_list_initializer>, mla_dynamic_data_empty());

}

mla_bool_t mla_user_data_set_pointer_with_ownership_ex(mla_user_data_t &target, const mla_char_t name[mla_user_data_name_size],
                                     mla_pointer_t data, mla_buffer_cleanup_hook_t cleanup_hook,
                                     mla_bool_t mangedExternalResource) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_create(data, mangedExternalResource, cleanup_hook,mla_dynamic_data_empty());
    user_data->data.asPointer = data;

    __mla_user_data_update_manage_external_resource(target, user_data);

    return true;
}

mla_bool_t mla_user_data_set_pointer_without_ownership_ex(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_pointer_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asPointer = data;
    return true;

}

mla_bool_t mla_user_data_set_int8(mla_user_data_t &target, const mla_char_t name[mla_user_data_name_size],
                                  mla_int8_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asInt8 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint8(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint8_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asUint8 = data;
    return true;

}

mla_bool_t mla_user_data_set_int16(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int16_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asInt16 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint16(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint16_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asUint16 = data;
    return true;

}

mla_bool_t mla_user_data_set_int32(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int32_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asInt32 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint32(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint32_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asUint32 = data;
    return true;

}

mla_bool_t mla_user_data_set_int64(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int64_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asInt64 = data;
    return true;

}

mla_bool_t mla_user_data_set_uint64(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint64_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asUint64 = data;
    return true;

}

mla_bool_t mla_user_data_set_float(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_float_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asFloat = data;
    return true;

}

mla_bool_t mla_user_data_set_double(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_double_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asDouble = data;
    return true;

}

mla_bool_t mla_user_data_set_bool(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_bool_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asBool = data;
    return true;

}

mla_bool_t mla_user_data_set_char(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_char_t data) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_noOwner();
    user_data->data.asChar = data;
    return true;

}

mla_bool_t mla_user_data_set_native_resource(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_dynamic_data_t data, mla_buffer_cleanup_hook_t cleanup) {

    mla_user_data_t* user_data = __mla_user_data_get_for_update(target, name);

    if (user_data == nullptr) {
        return false;
    }

    mla_memcpy(user_data->name, name, mla_user_data_name_size);
    user_data->dataOwner = mla_buffer_reference_create(data.asPointer, true, cleanup, data);
    user_data->data = data;

    __mla_user_data_update_manage_external_resource(target, user_data);

    return true;
}


mla_bool_t mla_user_data_equal(const mla_user_data_t& a, const mla_user_data_t& b) {

    if (__mla_user_data_name_equal(a.name, b.name)) {
        return false;
    }

    if (mla_memcmp(&a.data, &b.data, sizeof(mla_dynamic_data_t)) != 0) {
        return false;
    }

    return true;

}

mla_user_data_t* mla_user_data_get(mla_user_data_t& data, const mla_char_t name[mla_user_data_name_size]) {

    if (__mla_user_data_name_equal(data.name, name)) {
        return &data;
    }

    if (__mla_user_data_name_equal(data.name, mla_user_data_name_list)) {

        mla_user_data_list_t* list = (mla_user_data_list_t*)data.data.asPointer;
        if (list == nullptr) {
            return nullptr;
        }

        for (mla_size_t i = 0; i < mla_array_list_size(list->datas); ++i) {

            mla_user_data_t* item = mla_array_list_get_ref_unsafe(list->datas, i);
            if (__mla_user_data_name_equal(item->name, name)) {
                return item;
            }
        }
    }

    return nullptr;
}

mla_pointer_t mla_user_data_get_mla_pointer(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size]) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return nullptr;
    }
    return found->data.asPointer;
}

mla_int8_t mla_user_data_get_int8(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int8_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt8;
}

mla_uint8_t mla_user_data_get_uint8(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint8_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint8;
}

mla_int16_t mla_user_data_get_int16(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int16_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt16;
}

mla_uint16_t mla_user_data_get_uint16(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint16_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint16;
}

mla_int32_t mla_user_data_get_int32(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int32_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt32;
}

mla_uint32_t mla_user_data_get_uint32(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint32_t defaultValue) {;

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint32;
}

mla_int64_t mla_user_data_get_int64(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int64_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asInt64;
}

mla_uint64_t mla_user_data_get_uint64(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint64_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asUint64;
}

mla_float_t mla_user_data_get_float(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_float_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asFloat;
}

mla_double_t mla_user_data_get_double(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_double_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asDouble;
}

mla_bool_t mla_user_data_get_bool(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_bool_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asBool;
}

mla_char_t mla_user_data_get_char(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_char_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data.asChar;
}

mla_dynamic_data_t mla_user_data_get_native_resource(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_dynamic_data_t defaultValue) {

    const mla_user_data_t* found = mla_user_data_get((mla_user_data_t&)userData, name);
    if (found == nullptr) {
        return defaultValue;
    }
    return found->data;
}
