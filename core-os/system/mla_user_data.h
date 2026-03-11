//
// Created by chris on 2/9/2026.
//

#ifndef COREOS_MLA_USER_DATA_H
#define COREOS_MLA_USER_DATA_H

#include "mla_buffer.h"
#include "mla_string.h"

typedef mla_uint16_t mla_user_data_id;

struct mla_user_data_t {
    mla_buffer_reference_t dataOwner;
    mla_dynamic_data_t data;
    mla_user_data_id id;

    // Copy Constructor
    mla_user_data_t(const mla_user_data_t& p_Other);

    // Default Constructor
    mla_user_data_t(mla_user_data_id id, mla_buffer_reference_t dataOwner, mla_dynamic_data_t data);

    // Assignment operator
    mla_user_data_t& operator=(const mla_user_data_t& p_Other);
};

mla_user_data_id mla_get_next_user_data_id();

#define mla_user_data_id_init(name) const static mla_user_data_id name = mla_get_next_user_data_id();

mla_user_data_t mla_user_data_empty();
mla_user_data_t mla_user_data_copy(const mla_user_data_t& other);

mla_bool_t mla_user_data_remove(mla_user_data_t& target, mla_user_data_id id);
mla_bool_t mla_user_data_set_pointer_with_ownership_ex(mla_user_data_t& target, mla_user_data_id id, mla_pointer_t data, mla_buffer_cleanup_hook_t cleanup, mla_bool_t mangedExternalResource = true /** for now true because in Area We can not control 100% if the inner suff need to be maintained, like wrapped sockeds **/);
mla_bool_t mla_user_data_set_pointer_without_ownership_ex(mla_user_data_t& target, mla_user_data_id id, mla_pointer_t data);
mla_bool_t mla_user_data_set_int8(mla_user_data_t& target, mla_user_data_id id, mla_int8_t data);
mla_bool_t mla_user_data_set_uint8(mla_user_data_t& target, mla_user_data_id id, mla_uint8_t data);
mla_bool_t mla_user_data_set_int16(mla_user_data_t& target, mla_user_data_id id, mla_int16_t data);
mla_bool_t mla_user_data_set_uint16(mla_user_data_t& target, mla_user_data_id id, mla_user_data_id data);
mla_bool_t mla_user_data_set_int32(mla_user_data_t& target, mla_user_data_id id, mla_int32_t data);
mla_bool_t mla_user_data_set_uint32(mla_user_data_t& target, mla_user_data_id id, mla_uint32_t data);
mla_bool_t mla_user_data_set_int64(mla_user_data_t& target, mla_user_data_id id, mla_int64_t data);
mla_bool_t mla_user_data_set_uint64(mla_user_data_t& target, mla_user_data_id id, mla_uint64_t data);
mla_bool_t mla_user_data_set_float(mla_user_data_t& target, mla_user_data_id id, mla_float_t data);
mla_bool_t mla_user_data_set_double(mla_user_data_t& target, mla_user_data_id id, mla_double_t data);
mla_bool_t mla_user_data_set_bool(mla_user_data_t& target, mla_user_data_id id, mla_bool_t data);
mla_bool_t mla_user_data_set_char(mla_user_data_t& target, mla_user_data_id id, mla_char_t data);
mla_bool_t mla_user_data_set_string(mla_user_data_t& target, mla_user_data_id id, mla_string_t& data);

mla_bool_t mla_user_data_inc_int8(mla_user_data_t& target, mla_user_data_id id, mla_int8_t step);
mla_bool_t mla_user_data_inc_uint8(mla_user_data_t& target, mla_user_data_id id, mla_uint8_t step);
mla_bool_t mla_user_data_inc_int16(mla_user_data_t& target, mla_user_data_id id, mla_int16_t step);
mla_bool_t mla_user_data_inc_uint16(mla_user_data_t& target, mla_user_data_id id, mla_uint16_t step);
mla_bool_t mla_user_data_inc_int32(mla_user_data_t& target, mla_user_data_id id, mla_int32_t step);
mla_bool_t mla_user_data_inc_uint32(mla_user_data_t& target, mla_user_data_id id, mla_uint32_t step);
mla_bool_t mla_user_data_inc_int64(mla_user_data_t& target, mla_user_data_id id, mla_int64_t step);
mla_bool_t mla_user_data_inc_uint64(mla_user_data_t& target, mla_user_data_id id, mla_uint64_t step);
mla_bool_t mla_user_data_inc_float(mla_user_data_t& target, mla_user_data_id id, mla_float_t step);
mla_bool_t mla_user_data_inc_double(mla_user_data_t& target, mla_user_data_id id, mla_double_t step);

mla_bool_t mla_user_data_dec_int8(mla_user_data_t& target, mla_user_data_id id, mla_int8_t step);
mla_bool_t mla_user_data_dec_uint8(mla_user_data_t& target, mla_user_data_id id, mla_uint8_t step);
mla_bool_t mla_user_data_dec_int16(mla_user_data_t& target, mla_user_data_id id, mla_int16_t step);
mla_bool_t mla_user_data_dec_uint16(mla_user_data_t& target, mla_user_data_id id, mla_uint16_t step);
mla_bool_t mla_user_data_dec_int32(mla_user_data_t& target, mla_user_data_id id, mla_int32_t step);
mla_bool_t mla_user_data_dec_uint32(mla_user_data_t& target, mla_user_data_id id, mla_uint32_t step);
mla_bool_t mla_user_data_dec_int64(mla_user_data_t& target, mla_user_data_id id, mla_int64_t step);
mla_bool_t mla_user_data_dec_uint64(mla_user_data_t& target, mla_user_data_id id, mla_uint64_t step);
mla_bool_t mla_user_data_dec_float(mla_user_data_t& target, mla_user_data_id id, mla_float_t step);
mla_bool_t mla_user_data_dec_double(mla_user_data_t& target, mla_user_data_id id, mla_double_t step);

typedef void(*mla_user_data_set_native_resource_hook_t)(const mla_dynamic_data_t& userData);

mla_bool_t mla_user_data_set_native_resource(mla_user_data_t& target, mla_user_data_id id, mla_dynamic_data_t data, mla_user_data_set_native_resource_hook_t);

template <typename T, typename TInit = mla_default_init_ref(T)>
mla_bool_t mla_user_data_set_pointer_with_ownership(mla_user_data_t& target, mla_user_data_id id, T* data) {

    mla_buffer_cleanup_hook_t cleanupHook = mla_buffer_default_cleanup<T, TInit>;
    return mla_user_data_set_pointer_with_ownership_ex(target, id, reinterpret_cast<mla_pointer_t>(data), cleanupHook);

}

template <typename T>
mla_bool_t mla_user_data_set_pointer_without_ownership(mla_user_data_t& target, mla_user_data_id id, T* data) {

    return mla_user_data_set_pointer_without_ownership_ex(target, id, reinterpret_cast<mla_pointer_t>(data));
}

template <typename T>
mla_bool_t mla_user_data_set_pointer_without_ownership(mla_user_data_t& target, mla_user_data_id id, const T* data) {

    return mla_user_data_set_pointer_without_ownership_ex(target, id, reinterpret_cast<mla_pointer_t>(const_cast<T*>(data)));
}

template <typename T>
mla_bool_t mla_user_data_set_callback(mla_user_data_t& target, mla_user_data_id id, T data) {
    return mla_user_data_set_pointer_without_ownership_ex(target, id, reinterpret_cast<mla_pointer_t>(data));
}

mla_bool_t mla_user_data_equal(const mla_user_data_t& a, const mla_user_data_t& b);
mla_user_data_t* mla_user_data_get(const mla_user_data_t& data, mla_user_data_id id);

mla_pointer_t mla_user_data_get_mla_pointer(const mla_user_data_t& userData, mla_user_data_id id);
mla_int8_t mla_user_data_get_int8(const mla_user_data_t& userData, mla_user_data_id id, mla_int8_t defaultValue = 0);
mla_uint8_t mla_user_data_get_uint8(const mla_user_data_t& userData, mla_user_data_id id, mla_uint8_t defaultValue = 0);
mla_int16_t mla_user_data_get_int16(const mla_user_data_t& userData, mla_user_data_id id, mla_int16_t defaultValue = 0);
mla_uint16_t mla_user_data_get_uint16(const mla_user_data_t& userData, mla_user_data_id id, mla_uint16_t defaultValue = 0);
mla_int32_t mla_user_data_get_int32(const mla_user_data_t& userData, mla_user_data_id id, mla_int32_t defaultValue = 0);
mla_uint32_t mla_user_data_get_uint32(const mla_user_data_t& userData, mla_user_data_id id, mla_uint32_t defaultValue = 0);
mla_int64_t mla_user_data_get_int64(const mla_user_data_t& userData, mla_user_data_id id, mla_int64_t defaultValue = 0);
mla_uint64_t mla_user_data_get_uint64(const mla_user_data_t& userData, mla_user_data_id id, mla_uint64_t defaultValue = 0);
mla_float_t mla_user_data_get_float(const mla_user_data_t& userData, mla_user_data_id id, mla_float_t defaultValue = 0);
mla_double_t mla_user_data_get_double(const mla_user_data_t& userData, mla_user_data_id id, mla_double_t defaultValue = 0);
mla_bool_t mla_user_data_get_bool(const mla_user_data_t& userData, mla_user_data_id id, mla_bool_t defaultValue = false);
mla_char_t mla_user_data_get_char(const mla_user_data_t& userData, mla_user_data_id id, mla_char_t defaultValue = 0);
mla_string_t mla_user_data_get_string(const mla_user_data_t& userData, mla_user_data_id id, mla_string_t defaultValue = mla_string_empty());

mla_int8_t mla_user_data_get_and_replace_int8(const mla_user_data_t& userData, mla_user_data_id id, mla_int8_t newValue, mla_int8_t defaultValue = 0);
mla_uint8_t mla_user_data_get_and_replace_uint8(const mla_user_data_t& userData, mla_user_data_id id, mla_uint8_t newValue, mla_uint8_t defaultValue = 0);
mla_int16_t mla_user_data_get_and_replace_int16(const mla_user_data_t& userData, mla_user_data_id id, mla_int16_t newValue, mla_int16_t defaultValue = 0);
mla_uint16_t mla_user_data_get_and_replace_uint16(const mla_user_data_t& userData, mla_user_data_id id, mla_uint16_t newValue, mla_uint16_t defaultValue = 0);
mla_int32_t mla_user_data_get_and_replace_int32(const mla_user_data_t& userData, mla_user_data_id id, mla_int32_t newValue, mla_int32_t defaultValue = 0);
mla_uint32_t mla_user_data_get_and_replace_uint32(const mla_user_data_t& userData, mla_user_data_id id, mla_uint32_t newValue, mla_uint32_t defaultValue = 0);
mla_int64_t mla_user_data_get_and_replace_int64(const mla_user_data_t& userData, mla_user_data_id id, mla_int64_t newValue, mla_int64_t defaultValue = 0);
mla_uint64_t mla_user_data_get_and_replace_uint64(const mla_user_data_t& userData, mla_user_data_id id, mla_uint64_t newValue, mla_uint64_t defaultValue = 0);
mla_float_t mla_user_data_get_and_replace_float(const mla_user_data_t& userData, mla_user_data_id id, mla_float_t newValue, mla_float_t defaultValue = 0);
mla_double_t mla_user_data_get_and_replace_double(const mla_user_data_t& userData, mla_user_data_id id, mla_double_t newValue, mla_double_t defaultValue = 0);
mla_bool_t mla_user_data_get_and_replace_bool(const mla_user_data_t& userData, mla_user_data_id id, mla_bool_t newValue, mla_bool_t defaultValue = false);
mla_char_t mla_user_data_get_and_replace_char(const mla_user_data_t& userData, mla_user_data_id id, mla_char_t newValue, mla_char_t defaultValue = 0);
mla_string_t mla_user_data_get_and_replace_string(const mla_user_data_t& userData, mla_user_data_id id, mla_string_t newValue, mla_string_t defaultValue = mla_string_empty());

mla_dynamic_data_t mla_user_data_get_native_resource(const mla_user_data_t& userData, mla_user_data_id id, mla_dynamic_data_t defaultValue = mla_dynamic_data_empty());


template <typename T>
T* mla_user_data_get_pointer(const mla_user_data_t& userData, mla_user_data_id id) {
    return (T*)mla_user_data_get_mla_pointer(userData, id);
}

template <typename T>
T mla_user_data_get_callback(const mla_user_data_t& userData, mla_user_data_id id) {
    return (T)mla_user_data_get_mla_pointer(userData, id);
}


struct mla_user_data_initializer {

    static mla_user_data_t init() {
        return mla_user_data_empty();
    }
};

#endif