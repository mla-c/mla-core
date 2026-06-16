//
// Created by chris on 4/21/2026.
//

#include "mla_data_types.h"

mla_pointer_t mla_platform_pointer_to_managed_pointer(mla_platform_const_pointer_t resource) {

    if (resource == nullptr) {
        return mla_pointer_null();
    }

    return {
        mla_dynamic_data_from_pointer(mla_c_cast<mla_platform_pointer_t>(resource)),
        nullptr
    };
}