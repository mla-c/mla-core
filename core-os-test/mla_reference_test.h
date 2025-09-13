#ifndef COREOS_MLA_REFERENCE_TEST_H
#define COREOS_MLA_REFERENCE_TEST_H

#include "../core-os/system/mla_reference.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

static mla_bool_t is_resource_destroyed;

class mla_reference_test_external_resource {
public:


    ~mla_reference_test_external_resource() {
        // Destructor logic for external resource
        is_resource_destroyed = true;
    }
};


void SimpleMemoryManagementTest() {

    is_resource_destroyed = false;
    mla_reference_test_external_resource* resource = new mla_reference_test_external_resource();

    {
        mla_buffer_reference_t other = mla_buffer_reference_noOwner();
        mla_buffer_reference_t other2 = mla_buffer_reference_noOwner();
        {
            mla_buffer_reference_t reference = mla_class_reference(resource);
            assert_equal(reference.buffer->refCount, (mla_size_t)1, "Reference count should be 1 after creation");
            other = reference; // Copy the reference
            assert_equal(reference.buffer->refCount, (mla_size_t)2, "Reference count should be 2 after copying");
            other2 = other;
            assert_equal(reference.buffer->refCount, (mla_size_t)3, "Reference count should be 2 after copying");
            other = mla_buffer_reference_noOwner(); // Clear the reference
            assert_equal(reference.buffer->refCount, (mla_size_t)2, "Reference count should be 2 after clearing the first reference");

            assert_false(is_resource_destroyed, "External resource should not be destroyed yet");
        }

        assert_equal(other2.buffer->refCount, (mla_size_t)1, "Reference count should be 2 after copying to other2");
        assert_false(is_resource_destroyed, "External resource should not be destroyed yet");

    }

    assert_true(is_resource_destroyed, "External resource should be destroyed after reference goes out of scope");
}

void RegisterReferenceTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleMemoryManagement", test_category, SimpleMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
