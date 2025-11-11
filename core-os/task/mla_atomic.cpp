//
// Created by chris on 11/11/2025.
//

#include "mla_atomic.h"
#include "mla_task_manager_data_types.h"

mla_atomic_int32_t mla_atomic_int32(mla_int32_t initialValue) {
    return {initialValue};
}

mla_int32_t mla_atomic_increment(mla_atomic_int32_t& value) {
    return g_task_low_level_access.atomic_int32_increment(value);
}

mla_int32_t mla_atomic_decrement(mla_atomic_int32_t& value) {
    return g_task_low_level_access.atomic_int32_decrement(value);
}

mla_int32_t mla_atomic_add(mla_atomic_int32_t& value, mla_int32_t addend) {
    return g_task_low_level_access.atomic_int32_add(value, addend);
}

mla_int32_t mla_atomic_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend) {
    return g_task_low_level_access.atomic_int32_subtract(value, subtrahend);
}

mla_int32_t mla_atomic_exchange(mla_atomic_int32_t& value, mla_int32_t newValue) {
    return g_task_low_level_access.atomic_int32_exchange(value, newValue);
}

mla_bool_t mla_atomic_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue) {
    return g_task_low_level_access.atomic_int32_compare_exchange(value, expectedValue, newValue);
}