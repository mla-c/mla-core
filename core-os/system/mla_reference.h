//
// Created by christian on 8/7/25.
//

#include "mla_buffer.h"

#ifndef COREOS_MLA_REFERENCE_H
#define COREOS_MLA_REFERENCE_H

template <class T>
mla_buffer_cleanup_mode __mla_class_reference_destroy(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData;

    // Custom cleanup logic for class reference
    if (data) {
        // Perform cleanup actions
        delete static_cast<T*>(data); // Assuming the data is dynamically allocated

        // cleanup is down so you we dont need to cleanup more
        return CLEAN_UP_SKIP;
    }

    return CLEAN_UP_NEEDED;
}

template <class T>
mla_buffer_reference_t mla_class_reference(T* data) {
    return mla_buffer_reference(data, __mla_class_reference_destroy<T>, 0);
}


template <typename  T>
mla_buffer_reference_t mla_struct_reference(T* data) {
    return mla_buffer_reference(data, nullptr, 0);
}

#endif
