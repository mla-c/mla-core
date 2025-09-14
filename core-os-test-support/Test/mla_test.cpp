//
// Created by christian on 9/10/25.
//

#include "mla_test.h"

mla_test_result_t current_test_result;

mla_test_t mla_test(const char *name, const char *category,
                    void (*run)(void),
                    void (*setUp)(void),
                    void (*tearDown)(void)) {
    return {name, category, run, setUp, tearDown};
}

void mla_test_destroy(mla_test_t &test) {

    test.name = nullptr;
    test.category = nullptr;
    test.run = nullptr;
    test.setUp = nullptr;
    test.tearDown = nullptr;

}

typedef mla_test_pointer_t (*test_stat_malloc_hook_t)(mla_test_uint32_t size);
static test_stat_malloc_hook_t mla_test_malloc_hook_original = nullptr;


mla_test_pointer_t mla_test_malloc_stat_hook(mla_test_uint32_t size) {
    current_test_result.allocated_memory += size;
    return mla_test_malloc_hook_original(size);
}

mla_test_bool_t mla_test_run(mla_test_t &test) {

    current_test_result = {true, nullptr, 0};

    if (test.setUp) {
        test.setUp();
    }

#ifdef mla_debug_build
    mla_test_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_test_malloc_stat_hook;
#endif

    test.run();

#ifdef mla_debug_build
    g_low_level_access.malloc = mla_test_malloc_hook_original;
    mla_test_malloc_hook_original = nullptr;
#endif

    if (test.tearDown) {
        test.tearDown();
    }

    if (current_test_result.success) {
        mla_test_printf("✓ Test passed: %s->%s", test.category, test.name);
    } else {
        mla_test_printf("⚠ Test failed: %s->%s - %s", test.category, test.name, current_test_result.message);
    }

#ifdef mla_debug_build
    mla_test_printf(" (Memory allocated: %llu bytes)", current_test_result.allocated_memory);
#endif

    mla_test_printf("\n");

    mla_test_bool_t result = current_test_result.success;

    if (current_test_result.message) {
        delete[] current_test_result.message;
        current_test_result.message = nullptr;
    }

    return result;
}

void mla_check_assert_true(mla_test_bool_t p_Condition, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (!p_Condition) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];

        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: %s", p_Line, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d", p_Line);
        }
        current_test_result.message = l_Result;
    }

}

void mla_check_assert_false(mla_test_bool_t p_Condition, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Condition) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: %s", p_Line, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d", p_Line);
        }
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_equal(mla_test_int8_t p_Actual, mla_test_int8_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %d, Actual: %d. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %d, Actual: %d", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int8_t p_Actual, mla_test_int8_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %d, Actual: %d. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %d, Actual: %d", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_int16_t p_Actual, mla_test_int16_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %d, Actual: %d. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %d, Actual: %d", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int16_t p_Actual, mla_test_int16_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %d, Actual: %d. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %d, Actual: %d", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_int32_t p_Actual, mla_test_int32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %ld, Actual: %ld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %ld, Actual: %ld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int32_t p_Actual, mla_test_int32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %ld, Actual: %ld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %ld, Actual: %ld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_uint32_t p_Actual, mla_test_uint32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %ld, Actual: %ld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %ld, Actual: %ld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_uint32_t p_Actual, mla_test_uint32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %ld, Actual: %ld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %ld, Actual: %ld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_int64_t p_Actual, mla_test_int64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %lld, Actual: %lld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %lld, Actual: %lld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int64_t p_Actual, mla_test_int64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %lld, Actual: %lld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %lld, Actual: %lld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(mla_test_uint64_t p_Actual, mla_test_uint64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %lld, Actual: %lld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %lld, Actual: %lld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_uint64_t p_Actual, mla_test_uint64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %lld, Actual: %lld. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal to Actual. Expected: %lld, Actual: %lld", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(mla_test_float_t p_Actual, mla_test_float_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %f, Actual: %f. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %f, Actual: %f", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_float_t p_Actual, mla_test_float_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal: %f, Actual: %f. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal: %f, Actual: %f", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_double_t p_Actual, mla_test_double_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %lf, Actual: %lf. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: %lf, Actual: %lf", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_double_t p_Actual, mla_test_double_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal: %lf, Actual: %lf. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal: %lf, Actual: %lf", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_char_t *p_Actual, mla_test_char_t *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (strcmp(p_Actual, p_Expected) != 0) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: '%s', Actual: '%s'. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected: '%s', Actual: '%s'", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_char_t *p_Actual, mla_test_char_t *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (strcmp(p_Actual, p_Expected) == 0) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal: '%s', Actual: '%s'. %s", p_Line, p_Expected, p_Actual, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal: '%s', Actual: '%s'", p_Line, p_Expected, p_Actual);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(void *p_Actual, void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected equal pointer, but got different pointer: %p. %s", p_Line, p_Expected, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected equal pointer, but got different pointer: %p", p_Line, p_Expected);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(void *p_Actual, void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal pointer, but got same pointer: %p. %s", p_Line, p_Expected, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal pointer, but got same pointer: %p", p_Line, p_Expected);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(const void *p_Actual, const void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected equal pointer, but got different pointer: %p. %s", p_Line, p_Expected, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected equal pointer, but got different pointer: %p", p_Line, p_Expected);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(const void *p_Actual, const void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];
        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal pointer, but got same pointer: %p. %s", p_Line, p_Expected, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not equal pointer, but got same pointer: %p", p_Line, p_Expected);
        }
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_null(void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer != nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];

        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected null pointer, but got: %p. %s", p_Line, p_Pointer, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected null pointer, but got: %p", p_Line, p_Pointer);
        }
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_not_null(void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer == nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];

        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not null pointer, but got null. %s", p_Line, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not null pointer, but got null", p_Line);
        }
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_null(const void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer != nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];

        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected null pointer, but got: %p. %s", p_Line, p_Pointer, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected null pointer, but got: %p", p_Line, p_Pointer);
        }
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_not_null(const void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer == nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = new mla_test_char_t[4096];

        if (p_Message) {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not null pointer, but got null. %s", p_Line, p_Message);
        } else {
            snprintf(l_Result, 4096, "Assertion failed at line %d: Expected not null pointer, but got null", p_Line);
        }
        current_test_result.message = l_Result;
    }
}
