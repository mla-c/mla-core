//
// Created by chris on 2/6/2026.
//

#ifndef MLA_MATH_UTILS_H
#define MLA_MATH_UTILS_H

#include "../mla_data_types.h"

#define mla_math_pi 3.14159265358979323846

mla_double_t mla_math_sin(mla_double_t x);
mla_double_t mla_math_cos(mla_double_t x);
mla_double_t mla_math_tan(mla_double_t x);
mla_double_t mla_math_asin(mla_double_t x);
mla_double_t mla_math_acos(mla_double_t x);
mla_double_t mla_math_atan(mla_double_t y, mla_double_t x);
mla_double_t mla_math_sqrt(mla_double_t x);
mla_double_t mla_math_pow(mla_double_t base, mla_double_t exponent);


#endif