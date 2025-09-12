//
// Created by chris on 8/3/2025.
//

#ifndef COREOS_ARRAY_LIST_TEST_H
#define COREOS_ARRAY_LIST_TEST_H

#include "../core-os/system/mla_string.h"
#include "../core-os/system/mla_array_list.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"
#include "mla_list_contains_const.h"

struct my_array_list_container_t {
    mla_array_list_t<int> arr;
};


struct my_array_list_test_struct {
    mla_int32_t test1;
    mla_int32_t test2;

    mla_bool_t operator==(const my_array_list_test_struct &other) const {
        return (this->test1 == other.test1 && this->test2 == other.test2);
    }
};

/**
 * You should never build a struc with const. The const are never copied.
 * This here is only an ugly trick for testing
 */
struct my_array_list_with_const_test_struct {
    const mla_int32_t test1 = 0;
    mla_int32_t test2 = 0;

    my_array_list_with_const_test_struct(mla_int32_t t1, mla_int32_t t2) : test1(t1), test2(t2) {}
    my_array_list_with_const_test_struct() = default;
    my_array_list_with_const_test_struct& operator=(const my_array_list_with_const_test_struct &other) {

        if (this != &other) {
            // Very hack trick to assign a const member variable
            mla_int32_t* nonConstTest1 = const_cast<mla_int32_t*>(&test1);
            *nonConstTest1 = other.test1; // test1 is const, so we cannot assign it
            this->test2 = other.test2; // test1 is const, so we cannot assign it
        }
        return *this;
    }

    mla_bool_t operator==(const my_array_list_with_const_test_struct &other) const {
        return (this->test1 == other.test1 && this->test2 == other.test2);
    }

    my_array_list_with_const_test_struct(const my_array_list_with_const_test_struct& other) {
        // Either implement copying logic here or just call your assignment operator
        *this = other;
    }
};


void ArrayListContainsMlaStringTest() {

    mla_array_list_t<mla_string_t, mla_string_initializer> mla_arr = mla_array_list<mla_string_t, mla_string_initializer>();

    mla_array_list_add(mla_arr, mla_string("Hello"));
    mla_array_list_add(mla_arr, mla_string("World"));
    mla_array_list_add(mla_arr, mla_string("!"));

    mla_bool_t found = mla_array_list_contains<mla_string_t>(mla_arr, mla_string("World"));
    assert_true(found, "List should contain 'World'");

    found = mla_array_list_contains<mla_string_t>(mla_arr, mla_string("NotInList"));
    assert_false(found, "List should not contain 'NotInList'");


}

void ArrayListContainsTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>();
    mla_array_list_add(mla_arr, 1);
    mla_array_list_add(mla_arr, 2);
    mla_array_list_add(mla_arr, 3);

    mla_bool_t found = mla_array_list_contains<int>(mla_arr, 2);
    assert_true(found, "List should contain '2'");
    found = mla_array_list_contains<int>(mla_arr, 4);
    assert_false(found, "List should not contain '4'");

    int value;
    assert_true(mla_array_list_get(mla_arr, 0, value), "Should get first item");
    assert_equal(value, 1, "First item should be '1'");
    assert_true(mla_array_list_get(mla_arr, 1, value), "Should get second item");
    assert_equal(value, 2, "Second item should be '2'");

    assert_false(mla_array_list_get(mla_arr, 3, value), "Should not get item at index 3 (out of bounds)");
    assert_equal(value, 0, "Value should be unchanged when getting out of bounds");

    assert_equal(mla_array_list_get_unsafe(mla_arr, 0), 1, "Unsafe get should return '1'");
    assert_equal(mla_array_list_get_unsafe(mla_arr, 1), 2, "Unsafe get should return '2'");

}

void ArrayListRemoveTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>();
    mla_array_list_add(mla_arr, 1);
    mla_array_list_add(mla_arr, 2);
    mla_array_list_add(mla_arr, 3);
    assert_equal(mla_arr.size, 3, "List size should be 3 after adding 3 items");

    mla_bool_t removed = mla_array_list_remove<int>(mla_arr, 1);
    assert_true(removed, "List should remove '2'");
    assert_false(mla_array_list_contains<int>(mla_arr, 2), "List should not contain '2' after removal");
    assert_true(mla_array_list_contains<int>(mla_arr, 1), "List should still contain '1'");
    assert_true(mla_array_list_contains<int>(mla_arr, 3), "List should still contain '3'");

    assert_equal(mla_arr.size, 2, "List size should be 2 after removal");

}

void ArrayListClearTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>();
    mla_array_list_add(mla_arr, 1);
    mla_array_list_add(mla_arr, 2);
    mla_array_list_add(mla_arr, 3);

    mla_array_list_clear(mla_arr);
    assert_equal(mla_arr.size, 0, "List size should be 0 after clearing");
    assert_false(mla_array_list_contains<int>(mla_arr, 1), "List should not contain '1' after clearing");
    assert_false(mla_array_list_contains<int>(mla_arr, 2), "List should not contain '2' after clearing");
    assert_false(mla_array_list_contains<int>(mla_arr, 3), "List should not contain '3' after clearing");
}

void ArrayListIndexOfTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>();
    mla_array_list_add(mla_arr, 1);
    mla_array_list_add(mla_arr, 2);
    mla_array_list_add(mla_arr, 3);

    mla_int32_t index = mla_array_list_index_of<int>(mla_arr, 2);
    assert_equal(index, (mla_int32_t)1, "Index of '2' should be 1");
    index = mla_array_list_index_of<int>(mla_arr, 4);
    assert_equal(index, (mla_int32_t)-1, "Index of '4' should be -1 (not found)");

}

void ArrayListAddMuchItemsTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>();
    for (int i = 0; i < 1000; ++i) {
        mla_array_list_add(mla_arr, i);
    }

    assert_equal(mla_arr.size, 1000, "List size should be 1000 after adding 1000 items");

    for (int i = 0; i < 1000; ++i) {
        assert_true(mla_array_list_contains<int>(mla_arr, i), "List should contain item");
    }
}

void ArrayListAddMuchItemsNoGrowTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>(1000);
    int* internal_buffer = mla_arr.items;

    for (int i = 0; i < 1000; ++i) {
        mla_array_list_add(mla_arr, i);
    }

    assert_equal(mla_arr.size, 1000, "List size should be 1000 after adding 1000 items");
    assert_equal(internal_buffer, mla_arr.items, "Internal buffer should not change after adding 1000 items");

    for (int i = 0; i < 1000; ++i) {
        assert_true(mla_array_list_contains<int>(mla_arr, i), "List should contain item");
    }
}

void ArrayListGrowTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>(10);
    for (int i = 0; i < 20; ++i) {
        mla_array_list_add(mla_arr, i);
    }

    assert_equal(mla_arr.size, 20, "List size should be 20 after adding 20 elements");
    for (int i = 0; i < 20; ++i) {
        assert_true(mla_array_list_contains<int>(mla_arr, i), "List should contain element");
    }

    assert_equal(mla_array_list_capacity(mla_arr), 20, "List capacity should be 20 after growing");
}

void ArrayListShrinkTest() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>(50);
    for (int i = 0; i < 20; ++i) {
        mla_array_list_add(mla_arr, i);
    }

    mla_array_list_shrink_to_fit(mla_arr);

    assert_equal(mla_arr.size, 20, "List size should be 20 after adding 20 elements");
    assert_equal(mla_array_list_capacity(mla_arr), 20, "List capacity should be 20 after shrinking");
}

void ArrayListItemMemoryManagementTest() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));

    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 1, "String 2 should have refCount of 1");

    mla_array_list_t<mla_string_t, mla_string_initializer> mla_arr = mla_array_list<mla_string_t, mla_string_initializer>();
    mla_array_list_add(mla_arr, mla_str1);
    mla_array_list_add(mla_arr, mla_str2);

    assert_equal(mla_str1.dataOwner.buffer->refCount, 2, "String 1 should have refCount of 2 after adding to list");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 2, "String 2 should have refCount of 2 after adding to list");

    mla_array_list_remove(mla_arr, 0);
    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1 after removal from list");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 2, "String 2 should still have refCount of 2 after removal of String 1");

    mla_array_list_clear(mla_arr);
    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1 after clearing list");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 1, "String 2 should have refCount of 1 after clearing list");


}

void ArrayListItemMemoryManagementDestroyTest() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));

    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 1, "String 2 should have refCount of 1");

    {
        mla_array_list_t<mla_string_t, mla_string_initializer> mla_arr = mla_array_list<mla_string_t, mla_string_initializer>();
        mla_array_list_add(mla_arr, mla_str1);
        mla_array_list_add(mla_arr, mla_str2);

        assert_equal(mla_str1.dataOwner.buffer->refCount, 2, "String 1 should have refCount of 2 after adding to list");
        assert_equal(mla_str2.dataOwner.buffer->refCount, 2, "String 2 should have refCount of 2 after adding to list");
    }

    // After the array list is destroyed, the strings should be destroyed as well
    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1 after array list destruction");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 1, "String 2 should have refCount of 1 after array list destruction");

}


void ArrayListItemMemoryManagementDestroy2Test() {

    // Make sure that the string has its own memory management
    mla_string_t mla_str1 = mla_string_concat(mla_string("Hel"), mla_string("lo"));
    mla_string_t mla_str2 = mla_string_concat(mla_string("Wor"), mla_string("ld"));

    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 1, "String 2 should have refCount of 1");

    {

        mla_array_list_t<mla_string_t, mla_string_initializer> other = mla_array_list<mla_string_t, mla_string_initializer>();

        {
            mla_array_list_t<mla_string_t, mla_string_initializer> mla_arr = mla_array_list<mla_string_t, mla_string_initializer>();
            mla_array_list_add(mla_arr, mla_str1);
            mla_array_list_add(mla_arr, mla_str2);
            other = mla_arr;
        }

        assert_equal(mla_array_list_size(other), 2, "Other list should have size 2 after assignment");

        assert_equal(mla_str1.dataOwner.buffer->refCount, 2, "String 1 should have refCount of 2 after assignment to other list");
        assert_equal(mla_str2.dataOwner.buffer->refCount, 2, "String 2 should have refCount of 2 after assignment to other list");
    }

    // After the array list is destroyed, the strings should be destroyed as well
    assert_equal(mla_str1.dataOwner.buffer->refCount, 1, "String 1 should have refCount of 1 after array list destruction");
    assert_equal(mla_str2.dataOwner.buffer->refCount, 1, "String 2 should have refCount of 1 after array list destruction");


}

void addOneItemToArray(mla_array_list_t<int> &list, int i) {
    mla_array_list_add(list, i);
}

void ArrayListMultiVariableAndMutationTest() {

    my_array_list_container_t container = { mla_array_list<int>() };

    mla_array_list_add(container.arr, 1);
    mla_array_list_add(container.arr, 2);
    mla_array_list_add(container.arr, 3);

    // Create multiple variables pointing to the same array list
    mla_array_list_t<int> other1 = container.arr;
    mla_array_list_t<int>* other2 = &container.arr;

    assert_equal(mla_array_list_size(container.arr), 3, "Container should have size 3 after adding 3 items");
    assert_equal(mla_array_list_size(other1), 3, "Other1 should have size 3 after assignment");
    assert_equal(mla_array_list_size(*other2), 3, "Other2 should have size 3 after assignment");

    // Mutate the original array list
    addOneItemToArray(container.arr, 4);

    assert_equal(mla_array_list_size(container.arr), 4, "Container should have size 3 after adding 3 items");
    assert_equal(mla_array_list_size(other1), 3, "Other1 should have size 3 after assignment");
    assert_equal(mla_array_list_size(*other2), 4, "Other2 should have size 4 after mutation of container");

}


void ArrayListWithValueStructTest() {

    mla_array_list_t<my_array_list_test_struct> list = mla_array_list<my_array_list_test_struct>(10);

    my_array_list_test_struct item1 = {1, 100};
    my_array_list_test_struct item2 = {2, 200};
    my_array_list_test_struct item3 = {3, 300};

    mla_array_list_add(list, item1);
    mla_array_list_add(list, item2);
    mla_array_list_add(list, item3);


    assert_true(mla_array_list_contains(list, item1), "ArrayList should contain item1");
    assert_true(mla_array_list_contains(list, item2), "ArrayList should contain item2");
    assert_true(mla_array_list_contains(list, item3), "ArrayList should contain item3");

    my_array_list_test_struct value;
    assert_true(mla_array_list_get(list, 1, value), "ArrayList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2l, "Value test1 should be equal to 2");
    assert_equal(value.test2, 200l, "Value test2 should be equal to 200");

    my_array_list_test_struct* valueRef = mla_array_list_get_ref(list, 1);
    valueRef->test2 = 0; // Reset value for next check

    assert_true(mla_array_list_get(list, 1, value), "ArrayList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2l, "Value test1 should be equal to 2 after retrieval");
    assert_equal(value.test2, 0l, "Value test2 should be equal to 0 after retrieval");

    assert_null(mla_array_list_get_ref(list, 999), "Get ref for out of bounds index should return null");
}


void ArrayListWithValueConstStructTest() {

    mla_array_list_t<my_array_list_with_const_test_struct> list = mla_array_list<my_array_list_with_const_test_struct>(10);

    my_array_list_with_const_test_struct item1 = {1, 100};
    my_array_list_with_const_test_struct item2 = {2, 200};
    my_array_list_with_const_test_struct item3 = {3, 300};

    mla_array_list_add(list, item1);
    mla_array_list_add(list, item2);
    mla_array_list_add(list, item3);


    assert_true(mla_array_list_contains(list, item1), "ArrayList should contain item1");
    assert_true(mla_array_list_contains(list, item2), "ArrayList should contain item2");
    assert_true(mla_array_list_contains(list, item3), "ArrayList should contain item3");


    my_array_list_with_const_test_struct value;
    assert_true(mla_array_list_get(list, 1, value), "ArrayList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2l, "Value test1 should be equal to 2");
    assert_equal(value.test2, 200l, "Value test2 should be equal to 200");

    my_array_list_with_const_test_struct* valueRef = mla_array_list_get_ref(list, 1);
    valueRef->test2 = 0; // Reset value for next check

    assert_true(mla_array_list_get(list, 1, value), "ArrayList should successfully retrieve value for index 1");
    assert_equal(value.test1, 2l, "Value test1 should be equal to 2 after retrieval");
    assert_equal(value.test2, 0l, "Value test2 should be equal to 0 after retrieval");

    assert_null(mla_array_list_get_ref(list, 999), "Get ref for out of bounds index should return null");
}



void RegisterArrayListTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("ContainsMlaString", test_category, ArrayListContainsMlaStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Contains", test_category, ArrayListContainsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Remove", test_category, ArrayListRemoveTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Clear", test_category, ArrayListClearTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOf", test_category, ArrayListIndexOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AddMuchItems", test_category, ArrayListAddMuchItemsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AddMuchItemsNoGrow", test_category, ArrayListAddMuchItemsNoGrowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Grow", test_category, ArrayListGrowTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Shrink", test_category, ArrayListShrinkTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagement", test_category, ArrayListItemMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagementDestroy", test_category, ArrayListItemMemoryManagementDestroyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ItemMemoryManagementDestroy2", test_category, ArrayListItemMemoryManagementDestroy2Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultiVariableAndMutation", test_category, ArrayListMultiVariableAndMutationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WithValueStruct", test_category, ArrayListWithValueStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("WithValueConstStruct", test_category, ArrayListWithValueConstStructTest);
    mla_test_executor_register_test(p_TestExecutor, test);


}

mla_array_list_t<int>  mla_array = mla_array_list_empty<int>();

void SetupArrayListContainsBenchmark() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>(CONST_LIST_CONTAINS_COUNT);

    for (int i = 0; i < CONST_LIST_CONTAINS_COUNT; ++i) {
        mla_array_list_add(mla_arr, i);
    }

}

void ArrayListContainsBenchmark() {

    for (int i = 0; i < CONST_LIST_CONTAINS_COUNT; ++i) {
        mla_bool_t found = mla_array_list_contains<int>(mla_array, i);

        if (!found) {
            // Fail
            static_assert(true, "Element not found in array");
        }
    }

}

void TearDownArrayListContainsBenchmark() {
    mla_array = mla_array_list_empty<int>();
}

void ArrayListAddMuchItemsBenchmark() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>(1000);
    for (int i = 0; i < 1000; ++i) {
        mla_array_list_add(mla_arr, i);
    }
}

void ArrayListAddMuchItemsWithResizeBenchmark() {

    mla_array_list_t<int> mla_arr = mla_array_list<int>(10);
    for (int i = 0; i < 1000; ++i) {
        mla_array_list_add(mla_arr, i);
    }
}

mla_array_list_t<mla_string_t, mla_string_initializer>  mla_string_array = mla_array_list_empty<mla_string_t, mla_string_initializer>();

void SetupArrayListDestroyBenchmark() {

    mla_string_array = mla_array_list<mla_string_t, mla_string_initializer>(1000);

    for (int i = 0; i < 1000; ++i) {
        mla_string_t data = mla_string_concat(mla_string("Item"), mla_string("Next"));
        mla_array_list_add(mla_string_array, data);
    }

}

void ArrayListDestroyBenchmark() {

    mla_string_array = mla_array_list_empty<mla_string_t, mla_string_initializer>();
}

void TearDownArrayListDestroyBenchmark() {
    mla_string_array = mla_array_list_empty<mla_string_t, mla_string_initializer>();
}


void RegisterArrayListBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {

    mla_benchmark_t benchmark = mla_benchmark("Contains", benchmark_category, ArrayListContainsBenchmark, SetupArrayListContainsBenchmark, TearDownArrayListContainsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("AddMuchItems", benchmark_category, ArrayListAddMuchItemsBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("AddMuchItemsWithResize", benchmark_category, ArrayListAddMuchItemsWithResizeBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    benchmark = mla_benchmark("ArrayCleanUp", benchmark_category, ArrayListDestroyBenchmark, SetupArrayListDestroyBenchmark, TearDownArrayListDestroyBenchmark);
    mla_benchmark_set_iteration_division(benchmark, 100);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif
