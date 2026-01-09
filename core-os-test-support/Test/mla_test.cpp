//
// Created by christian on 9/10/25.
//

#include "mla_test.h"
#include "../mla_test_utils.h"

#if (!defined(mla_test_memory) || (mla_test_memory == 1))
#include "../../core-os/mla_data_types.h"
#endif

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

    if (current_test_result.block_memory_allocations)
        return nullptr;

    current_test_result.allocated_memory += size;
    return mla_test_malloc_hook_original(size);
}

typedef void (*test_on_malloc_failure)(mla_test_uint32_t size, const mla_test_char_t* filename, const mla_test_char_t* function_name);
static test_on_malloc_failure mla_test_on_malloc_failure_hook_original = nullptr;

void mla_test_on_malloc_failure_hook(mla_test_uint32_t size, const mla_test_char_t* filename, const mla_test_char_t* function_name) {
    // supress error
    (void)size;
    (void)filename;
    (void)function_name;
}

mla_test_bool_t mla_test_run(mla_test_t &test) {

    current_test_result = {true, nullptr, 0, false};

    if (test.setUp) {
        test.setUp();
    }

#if (!defined(mla_test_memory) || (mla_test_memory == 1))
    mla_test_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_test_malloc_stat_hook;
#endif

    test.run();

#if (!defined(mla_test_memory) || (mla_test_memory == 1))
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

#if (!defined(mla_test_memory) || (mla_test_memory == 1))
    mla_test_printf(" (Memory allocated: %llu bytes)", current_test_result.allocated_memory);
#endif

    mla_test_printf("\n");

    mla_test_bool_t result = current_test_result.success;

    if (current_test_result.message) {
        mla_test_free(current_test_result.message);
        current_test_result.message = nullptr;
    }


#if (!defined(mla_test_memory) || (mla_test_memory == 1))

    // Run again but blocking memory allocations

    current_test_result.block_memory_allocations = true;

    if (test.setUp) {
        test.setUp();
    }

    mla_test_malloc_hook_original = g_low_level_access.malloc;
    g_low_level_access.malloc = mla_test_malloc_stat_hook;

    mla_test_on_malloc_failure_hook_original = g_low_level_access.on_malloc_failure;
    g_low_level_access.on_malloc_failure = mla_test_on_malloc_failure_hook;

    test.run();

    g_low_level_access.on_malloc_failure = mla_test_on_malloc_failure_hook_original;
    mla_test_on_malloc_failure_hook_original = nullptr;

    g_low_level_access.malloc = mla_test_malloc_hook_original;
    mla_test_malloc_hook_original = nullptr;

    if (test.tearDown) {
        test.tearDown();
    }

    if (current_test_result.message) {
        mla_test_free(current_test_result.message);
        current_test_result.message = nullptr;
    }

#endif

    return result;
}

void mla_check_assert_fail(const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    current_test_result.success = false;
    mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
    mla_test_uint32_t offset = 0;

    // Build: "Assertion failed at line "
    offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
    // Add line number
    offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);

    if (p_Message) {
        // Add ": "
        offset += mla_test_strcat(l_Result, 4096, offset, ": ");
        // Add message
        offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
    }

    l_Result[offset] = '\0';
    current_test_result.message = l_Result;

}

void mla_check_assert_true(mla_test_bool_t p_Condition, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (!p_Condition) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);

        if (p_Message) {
            // Add ": "
            offset += mla_test_strcat(l_Result, 4096, offset, ": ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = l_Result;
    }

}

void mla_check_assert_false(mla_test_bool_t p_Condition, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Condition) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);

        if (p_Message) {
            // Add ": "
            offset += mla_test_strcat(l_Result, 4096, offset, ": ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_equal(mla_test_bool_t p_Actual, mla_test_bool_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_bool_t p_Actual, mla_test_bool_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_char_t p_Actual, mla_test_char_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_char_t p_Actual, mla_test_char_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(mla_test_int8_t p_Actual, mla_test_int8_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int8_t p_Actual, mla_test_int8_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_uint8_t p_Actual, mla_test_uint8_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_uint8_t p_Actual, mla_test_uint8_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_int16_t p_Actual, mla_test_int16_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int16_t p_Actual, mla_test_int16_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_uint16_t p_Actual, mla_test_uint16_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_uint16_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_uint16_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_uint16_t p_Actual, mla_test_uint16_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_uint16_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_uint16_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_int32_t p_Actual, mla_test_int32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int32_t p_Actual, mla_test_int32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_uint32_t p_Actual, mla_test_uint32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_uint32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_uint32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_uint32_t p_Actual, mla_test_uint32_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_uint32_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_uint32_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_int64_t p_Actual, mla_test_int64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_int64_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int64_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_int64_t p_Actual, mla_test_int64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_int64_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_int64_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(mla_test_uint64_t p_Actual, mla_test_uint64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_uint64_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_uint64_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_uint64_t p_Actual, mla_test_uint64_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal to Actual. Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal to Actual. Expected: ");
        // Add expected value
        offset += mla_uint64_to_string(l_Result + offset, 4096 - offset, p_Expected);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_uint64_to_string(l_Result + offset, 4096 - offset, p_Actual);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(mla_test_float_t p_Actual, mla_test_float_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_float_to_string(l_Result + offset, 4096 - offset, p_Expected, 6);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_float_to_string(l_Result + offset, 4096 - offset, p_Actual, 6);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_float_t p_Actual, mla_test_float_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    mla_test_float_t diff = p_Actual - p_Expected;
    if ((diff < 0 ? -diff : diff) > 0.000001) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal: ");
        // Add expected value
        offset += mla_float_to_string(l_Result + offset, 4096 - offset, p_Expected, 6);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_float_to_string(l_Result + offset, 4096 - offset, p_Actual, 6);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_double_t p_Actual, mla_test_double_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    mla_test_double_t diff = p_Actual - p_Expected;
    if ((diff < 0 ? -diff : diff) > 0.000001) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: ");
        // Add expected value
        offset += mla_double_to_string(l_Result + offset, 4096 - offset, p_Expected, 6);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_double_to_string(l_Result + offset, 4096 - offset, p_Actual, 6);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_double_t p_Actual, mla_test_double_t p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal: ");
        // Add expected value
        offset += mla_double_to_string(l_Result + offset, 4096 - offset, p_Expected, 6);
        // Add ", Actual: "
        offset += mla_test_strcat(l_Result, 4096, offset, ", Actual: ");
        // Add actual value
        offset += mla_double_to_string(l_Result + offset, 4096 - offset, p_Actual, 6);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(mla_test_char_t *p_Actual, mla_test_char_t *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (mla_test_strcmp(p_Actual, p_Expected) != 0) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected: '"
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected: '");
        // Add expected value
        offset += mla_test_strcat(l_Result, 4096, offset, p_Expected);
        // Add "', Actual: '"
        offset += mla_test_strcat(l_Result, 4096, offset, "', Actual: '");
        // Add actual value
        offset += mla_test_strcat(l_Result, 4096, offset, p_Actual);
        // Add "'"
        offset += mla_test_strcat(l_Result, 4096, offset, "'");

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(mla_test_char_t *p_Actual, mla_test_char_t *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (mla_test_strcmp(p_Actual, p_Expected) == 0) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal: '"
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal: '");
        // Add expected value
        offset += mla_test_strcat(l_Result, 4096, offset, p_Expected);
        // Add "', Actual: '"
        offset += mla_test_strcat(l_Result, 4096, offset, "', Actual: '");
        // Add actual value
        offset += mla_test_strcat(l_Result, 4096, offset, p_Actual);
        // Add "'"
        offset += mla_test_strcat(l_Result, 4096, offset, "'");

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_equal(void *p_Actual, void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected equal pointer, but got different pointer: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected equal pointer, but got different pointer: ");
        // Add expected pointer
        offset += mla_pointer_to_string(l_Result + offset, 4096 - offset, p_Expected);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(void *p_Actual, void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal pointer, but got same pointer: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal pointer, but got same pointer: ");
        // Add expected pointer
        offset += mla_pointer_to_string(l_Result + offset, 4096 - offset, p_Expected);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}


void mla_check_assert_equal(const void *p_Actual, const void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual != p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected equal pointer, but got different pointer: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected equal pointer, but got different pointer: ");
        // Add expected pointer
        offset += mla_pointer_to_string(l_Result + offset, 4096 - offset, (void*)p_Expected);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_not_equal(const void *p_Actual, const void *p_Expected, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Actual == p_Expected) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not equal pointer, but got same pointer: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not equal pointer, but got same pointer: ");
        // Add expected pointer
        offset += mla_pointer_to_string(l_Result + offset, 4096 - offset, (void*)p_Expected);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = (mla_test_char_t *)l_Result;
    }
}

void mla_check_assert_null(void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer != nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected null pointer, but got: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected null pointer, but got: ");
        // Add pointer
        offset += mla_pointer_to_string(l_Result + offset, 4096 - offset, p_Pointer);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_not_null(void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer == nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not null pointer, but got null"
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not null pointer, but got null");

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_null(const void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer != nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected null pointer, but got: "
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected null pointer, but got: ");
        // Add pointer
        offset += mla_pointer_to_string(l_Result + offset, 4096 - offset, (void*)p_Pointer);

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = l_Result;
    }
}

void mla_check_assert_not_null(const void *p_Pointer, const mla_test_char_t *p_Message, mla_test_int16_t p_Line) {

    if (!current_test_result.success)
        return;

    if (p_Pointer == nullptr) {
        current_test_result.success = false;
        mla_test_char_t* l_Result = (mla_test_char_t*)mla_test_malloc(sizeof(mla_test_char_t) * 4096);
        mla_test_uint32_t offset = 0;

        // Build: "Assertion failed at line "
        offset += mla_test_strcat(l_Result, 4096, offset, "Assertion failed at line ");
        // Add line number
        offset += mla_int16_to_string(l_Result + offset, 4096 - offset, p_Line);
        // Add ": Expected not null pointer, but got null"
        offset += mla_test_strcat(l_Result, 4096, offset, ": Expected not null pointer, but got null");

        if (p_Message) {
            // Add ". "
            offset += mla_test_strcat(l_Result, 4096, offset, ". ");
            // Add message
            offset += mla_test_strcat(l_Result, 4096, offset, p_Message);
        }

        l_Result[offset] = '\0';
        current_test_result.message = l_Result;
    }
}
