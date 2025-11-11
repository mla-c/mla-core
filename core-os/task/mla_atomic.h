//
// Created by chris on 11/11/2025.
//

#ifndef COREOS_MLA_ATOMIC_H
#define COREOS_MLA_ATOMIC_H

#include "../mla_data_types.h"

mla_atomic_int32_t mla_atomic_int32(mla_int32_t initialValue);

mla_int32_t mla_atomic_increment(mla_atomic_int32_t& value);
mla_int32_t mla_atomic_decrement(mla_atomic_int32_t& value);
mla_int32_t mla_atomic_add(mla_atomic_int32_t& value, mla_int32_t addend);
mla_int32_t mla_atomic_subtract(mla_atomic_int32_t& value, mla_int32_t subtrahend);
mla_int32_t mla_atomic_exchange(mla_atomic_int32_t& value, mla_int32_t newValue);
mla_bool_t mla_atomic_compare_exchange(mla_atomic_int32_t& value, mla_int32_t expectedValue, mla_int32_t newValue);

#endif