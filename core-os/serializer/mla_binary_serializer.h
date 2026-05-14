//
// Created by chris on 9/15/2025.
//

#ifndef MLA_BINARY_SERIALIZER_H
#define MLA_BINARY_SERIALIZER_H

#include "mla_serializer.h"

mla_serializer_t mla_binary_serializer(const mla_stream_output_t& output);
mla_deserializer_t mla_binary_deserializer(const mla_stream_input_t& input);


#endif