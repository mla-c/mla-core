//
// Created by christian on 9/18/25.
//

#ifndef MLA_MLA_JSON_SERIALIZER_H
#define MLA_MLA_JSON_SERIALIZER_H

#include "mla_serializer.h"

mla_serializer_t mla_json_serializer(const mla_stream_output_t& output);
mla_deserializer_t mla_json_deserializer(const mla_stream_input_t& input);

#endif
