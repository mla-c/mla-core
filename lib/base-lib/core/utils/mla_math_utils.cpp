//
// Created by chris on 2/6/2026.
//

#include "mla_math_utils.h"

// Internal Constants
#define mla_math_internal_two_pi 6.28318530717958647692
#define mla_math_internal_half_pi 1.57079632679489661923
#define mla_math_internal_e 2.71828182845904523536

// Internal Helpers
static inline mla_double_t local_abs(mla_double_t x) {
    return (x < 0.0) ? -x : x;
}

static mla_double_t local_fmod(mla_double_t x, mla_double_t y) {

    if (y == 0.0) {
        return 0.0;
    }

    mla_int64_t n = mla_s_cast<mla_int64_t>(x / y);
    return x -  (mla_s_cast<mla_double_t>(n) * y);
}

mla_double_t mla_math_sin(mla_double_t x) {
    // Range reduction
    x = local_fmod(x, mla_math_internal_two_pi);
    if (x > mla_math_pi) {
        x -= mla_math_internal_two_pi;
    }
    if (x < -mla_math_pi) {
        x += mla_math_internal_two_pi;
    }

    mla_double_t x2 = x * x;
    // Taylor series (Horner's method)
    return (x * (1.0 + (x2 * ((-1.0 / 6.0) + (x2 * ((1.0 / 120.0) + (x2 * ((-1.0 / 5040.0) + (x2 * (1.0 / 362880.0))))))))));
}

mla_double_t mla_math_cos(mla_double_t x) {
    // Range reduction
    x = local_fmod(x, mla_math_internal_two_pi);
    if (x > mla_math_pi) {
        x -= mla_math_internal_two_pi;
    }

    if (x < -mla_math_pi) {
        x += mla_math_internal_two_pi;
    }

    mla_double_t x2 = x * x;
    // Taylor series
    return (1.0 + (x2 * ((-0.5) + (x2 * ((1.0 / 24.0) + (x2 * ((-1.0 / 720.0) + (x2 * (1.0 / 40320.0)))))))));
}

mla_double_t mla_math_tan(mla_double_t x) {
    mla_double_t c = mla_math_cos(x);
    // Simple singularity check
    if (local_abs(c) < 1e-15) {
        return 0.0;
    }

    return mla_math_sin(x) / c;
}

// Internal helper for atan
static mla_double_t local_atan_core(mla_double_t x) {
    mla_double_t x2 = x * x;
    // Minimax polynomial approximation
    return x * (0.99997726 + (x2 * (-0.33262347 + (x2 * (0.19354346 + (x2 * (-0.11643287 + (x2 * (0.05265332 + (x2 * -0.01172120))))))))));
}

// Note: Function signature suggests atan2 behavior (y, x)
mla_double_t mla_math_atan(mla_double_t y, mla_double_t x) {
    if (x == 0.0) {
        if (y > 0.0) {
            return mla_math_internal_half_pi;
        }
        if (y < 0.0) {
            return -mla_math_internal_half_pi;
        }
        return 0.0;
    }

    mla_double_t z = y / x;
    mla_double_t abs_z = local_abs(z);
    mla_double_t result;

    if (abs_z <= 1.0) {
        result = local_atan_core(abs_z);
    } else {
        result = mla_math_internal_half_pi - local_atan_core(1.0 / abs_z);
    }

    if (z < 0.0) {
        result = -result;
    }

    // Adjust quadrants
    if (x < 0.0) {
        if (y >= 0.0) {
            result += mla_math_pi;
        } else {
            result -= mla_math_pi;
        }
    }

    return result;
}

mla_double_t mla_math_asin(mla_double_t x) {
    if (x < -1.0 || x > 1.0) {
        return 0.0;
    }
    return mla_math_atan(x, mla_math_sqrt(1.0 - (x * x)));
}

mla_double_t mla_math_acos(mla_double_t x) {
    if (x < -1.0 || x > 1.0) {
        return 0.0;
    }
    return mla_math_internal_half_pi - mla_math_asin(x);
}

mla_double_t mla_math_sqrt(mla_double_t x) {
    if (x < 0.0) {
        return 0.0;
    }
    if (x == 0.0) {
        return 0.0;
    }

    // Newton-Raphson
    mla_double_t guess = x * 0.5;
    for (mla_int32_t i = 0; i < 10; ++i) {
        guess = 0.5 * (guess + (x / guess));
    }
    return guess;
}

// Internal Logarithm helper
static mla_double_t local_ln(mla_double_t x) {
    if (x <= 0.0) {
        return 0.0;
    }

    mla_int32_t p = 0;
    // Scale x close to 1.0
    while (x > 2.0) {
        x /= mla_math_internal_e;
        p++;
    }
    while (x < 0.5) {
        x *= mla_math_internal_e;
        p--;
    }

    mla_double_t y = (x - 1.0) / (x + 1.0);
    mla_double_t y2 = y * y;
    mla_double_t sum = y;
    mla_double_t term = y;

    for (mla_int32_t i = 3; i < 15; i += 2) {
        term *= y2;
        sum += term / mla_s_cast<mla_double_t>(i);
    }

    return (2.0 * sum) + mla_s_cast<mla_double_t>(p);
}

// Internal Exponential helper
static mla_double_t local_exp(mla_double_t x) {
    mla_double_t sum = 1.0;
    mla_double_t term = 1.0;

    for (mla_int32_t i = 1; i < 25; ++i) {
        term *= x / mla_s_cast<mla_double_t>(i);
        sum += term;
        if (local_abs(term) < 1e-10) {
            break;
        }
    }
    return sum;
}

mla_double_t mla_math_pow(mla_double_t base, mla_double_t exponent) {
    if (base == 0.0) {
        return 0.0;
    }
    if (exponent == 0.0) {
        return 1.0;
    }
    if (exponent == 1.0) {
        return base;
    }

    // Optimization for integer exponents
    mla_int64_t exp_int = mla_s_cast<mla_int64_t>(exponent);
    if (mla_s_cast<mla_double_t>(exp_int) == exponent) {
        mla_double_t res = 1.0;
        mla_double_t curr = base;
        mla_int64_t e = exp_int < 0 ? -exp_int : exp_int;

        while (e > 0) {
            if (e % 2 == 1) {
                res *= curr;
            }
            curr *= curr;
            e /= 2;
        }
        return exp_int < 0 ? 1.0 / res : res;
    }

    // General case: x^y = e^(y * ln(x))
    if (base < 0.0) {
        return 0.0;
    }
    return local_exp(exponent * local_ln(base));
}