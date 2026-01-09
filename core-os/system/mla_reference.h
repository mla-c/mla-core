//
// Created by christian on 8/7/25.
//

#include "mla_buffer.h"

#ifndef COREOS_MLA_REFERENCE_H
#define COREOS_MLA_REFERENCE_H

template <typename  T>
mla_buffer_reference_t mla_struct_reference(T* data) {
    return mla_buffer_reference(data, nullptr, 0);
}

#endif
