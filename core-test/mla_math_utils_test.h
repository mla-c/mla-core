//
// Created by chris on 2/6/2026.
//

#ifndef MLA_MATH_UTILS_TEST_H
#define MLA_MATH_UTILS_TEST_H

#include "../framework/core/utils/mla_math_utils.h"
#include "../framework/core-test-support/mla_test_executor.h"

inline void MathSinTest() {
    // Test known values
    assert_true(mla_math_sin(0.0) < 0.00001 && mla_math_sin(0.0) > -0.00001, "sin(0) should be ~0");

    mla_double_t sin_pi_2 = mla_math_sin(mla_math_pi / 2.0);
    assert_true(sin_pi_2 > 0.99999 && sin_pi_2 < 1.00001, "sin(π/2) should be ~1");

    mla_double_t sin_pi = mla_math_sin(mla_math_pi);
    assert_true(sin_pi < 0.01 && sin_pi > -0.01, "sin(π) should be ~0");

    mla_double_t sin_3pi_2 = mla_math_sin(3.0 * mla_math_pi / 2.0);
    assert_true(sin_3pi_2 < -0.99999 && sin_3pi_2 > -1.00001, "sin(3π/2) should be ~-1");
}

inline void MathCosTest() {
    mla_double_t cos_0 = mla_math_cos(0.0);
    assert_true(cos_0 > 0.99999 && cos_0 < 1.00001, "cos(0) should be ~1");

    mla_double_t cos_pi_2 = mla_math_cos(mla_math_pi / 2.0);
    assert_true(cos_pi_2 < 0.01 && cos_pi_2 > -0.01, "cos(π/2) should be ~0");

    mla_double_t cos_pi = mla_math_cos(mla_math_pi);
    assert_true(cos_pi < -0.9 && cos_pi > -1.1, "cos(π) should be ~-1");
}

inline void MathTanTest() {
    mla_double_t tan_0 = mla_math_tan(0.0);
    assert_true(tan_0 < 0.00001 && tan_0 > -0.00001, "tan(0) should be ~0");

    mla_double_t tan_pi_4 = mla_math_tan(mla_math_pi / 4.0);
    assert_true(tan_pi_4 > 0.99999 && tan_pi_4 < 1.00001, "tan(π/4) should be ~1");
}

inline void MathAsinTest() {
    mla_double_t asin_0 = mla_math_asin(0.0);
    assert_true(asin_0 < 0.00001 && asin_0 > -0.00001, "asin(0) should be ~0");

    mla_double_t asin_1 = mla_math_asin(1.0);
    assert_true(asin_1 > (mla_math_pi / 2.0 - 0.00001) && asin_1 < (mla_math_pi / 2.0 + 0.00001), "asin(1) should be ~π/2");

    mla_double_t asin_neg1 = mla_math_asin(-1.0);
    assert_true(asin_neg1 < (-mla_math_pi / 2.0 + 0.00001) && asin_neg1 > (-mla_math_pi / 2.0 - 0.00001), "asin(-1) should be ~-π/2");
}

inline void MathAcosTest() {
    mla_double_t acos_1 = mla_math_acos(1.0);
    assert_true(acos_1 < 0.00001 && acos_1 > -0.00001, "acos(1) should be ~0");

    mla_double_t acos_0 = mla_math_acos(0.0);
    assert_true(acos_0 > (mla_math_pi / 2.0 - 0.00001) && acos_0 < (mla_math_pi / 2.0 + 0.00001), "acos(0) should be ~π/2");

    mla_double_t acos_neg1 = mla_math_acos(-1.0);
    assert_true(acos_neg1 > (mla_math_pi - 0.00001) && acos_neg1 < (mla_math_pi + 0.00001), "acos(-1) should be ~π");
}

inline void MathAtanTest() {
    mla_double_t atan_0_1 = mla_math_atan(0.0, 1.0);
    assert_true(atan_0_1 < 0.00001 && atan_0_1 > -0.00001, "atan2(0, 1) should be ~0");

    mla_double_t atan_1_1 = mla_math_atan(1.0, 1.0);
    assert_true(atan_1_1 > (mla_math_pi / 4.0 - 0.00001) && atan_1_1 < (mla_math_pi / 4.0 + 0.00001), "atan2(1, 1) should be ~π/4");

    mla_double_t atan_1_0 = mla_math_atan(1.0, 0.0);
    assert_true(atan_1_0 > (mla_math_pi / 2.0 - 0.00001) && atan_1_0 < (mla_math_pi / 2.0 + 0.00001), "atan2(1, 0) should be ~π/2");
}

inline void MathSqrtTest() {
    assert_true(mla_math_sqrt(0.0) < 0.00001 && mla_math_sqrt(0.0) > -0.00001, "sqrt(0) should be ~0");
    assert_true(mla_math_sqrt(1.0) > 0.99999 && mla_math_sqrt(1.0) < 1.00001, "sqrt(1) should be ~1");
    assert_true(mla_math_sqrt(4.0) > 1.99999 && mla_math_sqrt(4.0) < 2.00001, "sqrt(4) should be ~2");
    assert_true(mla_math_sqrt(9.0) > 2.99999 && mla_math_sqrt(9.0) < 3.00001, "sqrt(9) should be ~3");
    assert_true(mla_math_sqrt(16.0) > 3.99999 && mla_math_sqrt(16.0) < 4.00001, "sqrt(16) should be ~4");
}

inline void MathPowTest() {
    assert_true(mla_math_pow(2.0, 0.0) > 0.99999 && mla_math_pow(2.0, 0.0) < 1.00001, "2^0 should be ~1");
    assert_true(mla_math_pow(2.0, 1.0) > 1.99999 && mla_math_pow(2.0, 1.0) < 2.00001, "2^1 should be ~2");
    assert_true(mla_math_pow(2.0, 3.0) > 7.99999 && mla_math_pow(2.0, 3.0) < 8.00001, "2^3 should be ~8");
    assert_true(mla_math_pow(3.0, 2.0) > 8.99999 && mla_math_pow(3.0, 2.0) < 9.00001, "3^2 should be ~9");
    assert_true(mla_math_pow(5.0, 2.0) > 24.99999 && mla_math_pow(5.0, 2.0) < 25.00001, "5^2 should be ~25");
}

inline void MathPythagoreanIdentityTest() {
    mla_double_t angles[] = {0.0, mla_math_pi / 6.0, mla_math_pi / 4.0, mla_math_pi / 3.0, mla_math_pi / 2.0};

    for (mla_size_t i = 0; i < 5; i++) {
        mla_double_t sin_val = mla_math_sin(angles[i]);
        mla_double_t cos_val = mla_math_cos(angles[i]);
        mla_double_t identity = sin_val * sin_val + cos_val * cos_val;
        assert_true(identity > 0.99999 && identity < 1.00001, "Pythagorean identity sin²+cos²=1 should hold");
    }
}

void RegisterMathUtilsTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("Sin", test_category, MathSinTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Cos", test_category, MathCosTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Tan", test_category, MathTanTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Asin", test_category, MathAsinTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Acos", test_category, MathAcosTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Atan", test_category, MathAtanTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Sqrt", test_category, MathSqrtTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Pow", test_category, MathPowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("PythagoreanIdentity", test_category, MathPythagoreanIdentityTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}





#endif
