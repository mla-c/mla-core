//
// Created by chris on 4/14/2026.
//

#ifndef COREOS_MLA_POINTER_TEST_H
#define COREOS_MLA_POINTER_TEST_H

#include "../core-os/memory/mla_memory_hook.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

struct my_pointer_test_native_auto_test_t {
    mla_test_int8_t data[16];
    mla_pointer_t childData;

    static my_pointer_test_native_auto_test_t init() {
        return {{0}, mla_pointer_null()};
    }
};

static mla_bool_t is_pointer_test_destroyed = false;

mla_bool_t my_pointer_test_malloc_hook(mla_size_t size, mla_platform_pointer_t* out_ptr) {
    (void)size;
    (void)out_ptr;
    return false;
}

mla_bool_t my_pointer_test_free_hook(mla_platform_pointer_t ptr) {
    (void)ptr;
    is_pointer_test_destroyed = true;
    return false;
}

void SimplePointerReleaseTest() {

    is_pointer_test_destroyed = false;
    mla_memory_hook_t hook = mla_memory_hook_install(my_pointer_test_malloc_hook, my_pointer_test_free_hook);

    mla_pointer_t container = mla_malloc_struct(my_pointer_test_native_auto_test_t);

    assert_equal(mla_pointer_ref_count(container), (mla_int32_t)1, "Reference count should be 1 after creation");

    assert_false(is_pointer_test_destroyed, "Buffer should not be destroyed yet");

    {
        mla_pointer_t container2 = container; // Copy constructor
        assert_equal(mla_pointer_ref_count(container2), (mla_int32_t)2, "Reference count should be 2 after copy");
        assert_equal(mla_pointer_ref_count(container), (mla_int32_t)2, "Reference count should be 2 after copy");
        assert_false(is_pointer_test_destroyed, "Buffer should not be destroyed yet");
    }

    assert_equal(mla_pointer_ref_count(container), (mla_int32_t)1, "Reference count should be 1 after copy goes out of scope");
    container = mla_pointer_null(); // Clear the container
    assert_true(is_pointer_test_destroyed, "Buffer should be destroyed after clearing the container");

    mla_memory_hook_uninstall(hook); // Uninstall the memory hook

}

void SimplePointerChildReleaseTest() {

    mla_pointer_t container = mla_malloc_struct(my_pointer_test_native_auto_test_t);
    assert_equal(mla_pointer_ref_count(container), (mla_int32_t)1, "Reference count should be 1 after creation");

    mla_pointer_t child_container = mla_malloc_struct(my_pointer_test_native_auto_test_t);
    assert_equal(mla_pointer_ref_count(child_container), (mla_int32_t)1, "Reference count should be 1 after creation");

    my_pointer_test_native_auto_test_t* ptr_data = mla_pointer_get_data<my_pointer_test_native_auto_test_t>(container);

    if (ptr_data != nullptr) {
        ptr_data->childData = child_container;
    } else {
        assert_fail("Pointer data should not be null");
    }

    assert_equal(mla_pointer_ref_count(child_container), (mla_int32_t)2, "Reference count should be 2 after being assigned to childData");

     {
        mla_pointer_t container2 = container; // Copy constructor
        assert_equal(mla_pointer_ref_count(child_container), (mla_int32_t)2, "Reference count should be 2 after copy");
    }


    container = mla_pointer_null(); // Clear the container

    assert_equal(mla_pointer_ref_count(child_container), (mla_int32_t)1, "Reference count should be 1 after parent container is cleared");

}

void RegisterPointerTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimplePointerRelease", test_category, SimplePointerReleaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SimplePointerChildRelease", test_category, SimplePointerChildReleaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
