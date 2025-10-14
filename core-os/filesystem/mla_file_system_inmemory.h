//
// Created by chris on 10/14/2025.
//

#ifndef COREOS_MLA_FILE_SYSTEM_IN_MEMORY_H
#define COREOS_MLA_FILE_SYSTEM_IN_MEMORY_H

#include "mla_file_system.h"

mla_file_system_t mla_file_system_create_in_memory(mla_size_t capacity);
mla_file_system_t mla_file_system_create_in_memory_from_buffer(mla_byte_t* buffer, mla_size_t capacity);



#endif