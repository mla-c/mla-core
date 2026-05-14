//
// Created by copilot on 12/30/2025.
//

#ifndef MLA_C_MLA_XML_SERIALIZER_H
#define MLA_C_MLA_XML_SERIALIZER_H

#include "mla_serializer.h"

mla_serializer_t mla_xml_serializer(const mla_stream_output_t& output);
mla_deserializer_t mla_xml_deserializer(const mla_stream_input_t& input);

#endif
