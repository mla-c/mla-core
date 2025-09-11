//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_STRING_CONCAT_H
#define COREOS_MLA_STRING_CONCAT_H

#include "mla_string.h"

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2);

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2);

mla_string_t mla_string_concat(const mla_char_t* p_String1, mla_int32_t number);

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2);

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3);

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3);

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3,
                  const mla_string_t &p_String4);

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4 );


#endif
