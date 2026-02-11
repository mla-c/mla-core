//
// Created by chris on 2/9/2026.
//

#ifndef COREOS_MLA_USER_DATA_H
#define COREOS_MLA_USER_DATA_H

#include "mla_buffer.h"

#define mla_user_data_name_size 8

struct mla_user_data_t {
    mla_char_t name[mla_user_data_name_size];
    mla_buffer_reference_t dataOwner;
    mla_dynamic_data_t data;
};

mla_user_data_t mla_user_data_empty();
mla_bool_t mla_user_data_set_pointer_with_ownership_ex(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_pointer_t data, mla_buffer_cleanup_hook_t cleanup, mla_bool_t mangedExternalResource = false);
mla_bool_t mla_user_data_set_pointer_without_ownership_ex(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_pointer_t data);
mla_bool_t mla_user_data_set_int8(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int8_t data);
mla_bool_t mla_user_data_set_uint8(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint8_t data);
mla_bool_t mla_user_data_set_int16(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int16_t data);
mla_bool_t mla_user_data_set_uint16(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint16_t data);
mla_bool_t mla_user_data_set_int32(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int32_t data);
mla_bool_t mla_user_data_set_uint32(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint32_t data);
mla_bool_t mla_user_data_set_int64(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_int64_t data);
mla_bool_t mla_user_data_set_uint64(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_uint64_t data);
mla_bool_t mla_user_data_set_float(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_float_t data);
mla_bool_t mla_user_data_set_double(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_double_t data);
mla_bool_t mla_user_data_set_bool(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_bool_t data);
mla_bool_t mla_user_data_set_char(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_char_t data);
mla_bool_t mla_user_data_set_native_resource(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], mla_dynamic_data_t data, mla_buffer_cleanup_hook_t cleanup);

template <typename T, typename TInit = mla_default_init_ref(T)>
mla_bool_t mla_user_data_set_pointer_with_ownership(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], T* data) {

    mla_buffer_cleanup_hook_t cleanupHook = mla_buffer_default_cleanup<T, TInit>;
    return mla_user_data_set_pointer_with_ownership_ex(target, name, reinterpret_cast<mla_pointer_t>(data), cleanupHook);

}

template <typename T>
mla_bool_t mla_user_data_set_pointer_without_ownership(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], T* data) {

    return mla_user_data_set_pointer_without_ownership_ex(target, name, reinterpret_cast<mla_pointer_t>(data));
}

template <typename T>
mla_bool_t mla_user_data_set_pointer_without_ownership(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], const T* data) {

    return mla_user_data_set_pointer_without_ownership_ex(target, name, reinterpret_cast<mla_pointer_t>(const_cast<T*>(data)));
}

template <typename T>
mla_bool_t mla_user_data_set_callback(mla_user_data_t& target, const mla_char_t name[mla_user_data_name_size], T data) {
    return mla_user_data_set_pointer_without_ownership_ex(target, name, reinterpret_cast<mla_pointer_t>(data));
}

mla_bool_t mla_user_data_equal(const mla_user_data_t& a, const mla_user_data_t& b);
mla_user_data_t* mla_user_data_get(const mla_user_data_t& data, const mla_char_t name[mla_user_data_name_size]);

mla_pointer_t mla_user_data_get_mla_pointer(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size]);
mla_int8_t mla_user_data_get_int8(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int8_t defaultValue = 0);
mla_uint8_t mla_user_data_get_uint8(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint8_t defaultValue = 0);
mla_int16_t mla_user_data_get_int16(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int16_t defaultValue = 0);
mla_uint16_t mla_user_data_get_uint16(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint16_t defaultValue = 0);
mla_int32_t mla_user_data_get_int32(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int32_t defaultValue = 0);
mla_uint32_t mla_user_data_get_uint32(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint32_t defaultValue = 0);
mla_int32_t mla_user_data_get_int64(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_int64_t defaultValue = 0);
mla_uint64_t mla_user_data_get_uint64(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_uint64_t defaultValue = 0);
mla_float_t mla_user_data_get_float(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_float_t defaultValue = 0);
mla_double_t mla_user_data_get_double(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_double_t defaultValue = 0);
mla_bool_t mla_user_data_get_bool(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_bool_t defaultValue = false);
mla_char_t mla_user_data_get_char(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_char_t defaultValue = 0);
mla_dynamic_data_t mla_user_data_get_native_resource(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size], mla_dynamic_data_t defaultValue = mla_dynamic_data_empty());


template <typename T>
T* mla_user_data_get_pointer(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size]) {
    return (T*)mla_user_data_get_mla_pointer(userData, name);
}

template <typename T>
T mla_user_data_get_callback(const mla_user_data_t& userData, const mla_char_t name[mla_user_data_name_size]) {
    return (T)mla_user_data_get_mla_pointer(userData, name);
}


struct mla_user_data_initializer {

    static mla_user_data_t init() {
        return mla_user_data_empty();
    }
};

#endif