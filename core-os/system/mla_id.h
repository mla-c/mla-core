//
// Created by chris on 11/3/2025.
//

#ifndef MLA_ID_H
#define MLA_ID_H

#include "mla_string.h"

/*
 * Generates a new UUID (Universally Unique Identifier) string.
 * The UUID is generated according to RFC 4122 standards.
 */
mla_string_t mla_generate_uuid();

/*
 * Generates a runtime unique identifier string.
 * This identifier is unique for the duration of the program execution.
 */
mla_string_t mla_generate_runtime_id();


/*
 * Generates a random 32-bit unsigned integer.
 */
mla_uint32_t mla_random_uint32();


#endif