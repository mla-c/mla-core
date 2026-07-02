//
// Created by chris on 8/1/2025.
//

#ifndef MLA_MLACHARUTILS_H
#define MLA_MLACHARUTILS_H

#include "../mla_data_types.h"

inline mla_char_t mla_char_toLower(const mla_char_t p_Char) {
    if (p_Char >= 'A' && p_Char <= 'Z') {
        return mla_s_cast<mla_char_t>(p_Char + ('a' - 'A'));
    }
    return p_Char;
}

inline mla_char_t mla_char_toUpper(const mla_char_t p_Char) {

    if (p_Char >= 'a' && p_Char <= 'z') {
        return mla_s_cast<mla_char_t>(p_Char - ('a' - 'A'));
    }
    return p_Char;
}

inline mla_bool_t mla_char_is_whitespace(const mla_char_t p_Char) {
    return (p_Char == ' ') || (p_Char == '\t') || (p_Char == '\n') || (p_Char == '\r');
}

inline mla_bool_t mla_char_is_digit(const mla_char_t p_Char) {
    return (p_Char >= '0') && (p_Char <= '9');
}

#endif
