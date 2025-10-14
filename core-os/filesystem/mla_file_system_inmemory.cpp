//
// Created by chris on 10/14/2025.
//

#include "mla_file_system_inmemory.h"
#include "../mla_data_types.h"

mla_file_system_t mla_file_system_create_in_memory(mla_size_t capacity) {

    mla_byte_t* buffer = static_cast<mla_byte_t*>(mla_malloc(capacity * sizeof(mla_byte_t)));#

    if (buffer == nullptr) {
        return mla_file_system_empty();
    }

    return mla_file_system_create_in_memory_from_buffer(buffer, capacity);

}

mla_file_system_t mla_file_system_create_in_memory_from_buffer(mla_byte_t* buffer, mla_size_t capacity) {


}