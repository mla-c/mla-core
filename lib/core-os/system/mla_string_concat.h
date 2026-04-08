//
// Created by christian on 8/10/25.
//

#ifndef COREOS_MLA_STRING_CONCAT_H
#define COREOS_MLA_STRING_CONCAT_H

#include "mla_string.h"

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2);
mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2);
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2);
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_char_t* p_String2);

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3);
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3);
mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2, const mla_string_t &p_String3);

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_char_t* p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4 );
mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3, const mla_string_t &p_String4 );
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4 );
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4 );
mla_string_t mla_string_concat(const mla_string_t& p_String1, const mla_char_t* p_String2, const mla_string_t&  p_String3, const mla_char_t* p_String4 );

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5 );
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6 );
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6 );

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6, const mla_char_t* p_String7 );

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6, const mla_char_t* p_String7 , const mla_string_t& p_String8);

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t& p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4, const mla_char_t* p_String5, const mla_char_t* p_String6, const mla_char_t* p_String7, const mla_char_t* p_String8, const mla_char_t* p_String9);
mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t& p_String2, const mla_char_t* p_String3, const mla_string_t& p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6, const mla_char_t* p_String7,  const mla_string_t& p_String8, const mla_char_t* p_String9);


#endif
