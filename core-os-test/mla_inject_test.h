//
// Created by chris on 9/13/2025.
//

#ifndef COREOS_MLA_INJECT_TEST_H
#define COREOS_MLA_INJECT_TEST_H

#include "../core-os/inject/mla_inject.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

// Define a simple service struct
struct mla_test_service_singleton_t {

    static mla_string_t get_service_name() {
        static mla_string_t serviceName = mla_string("TestServiceSingleton");
        return serviceName;
    }
    static mla_inject_service_t<void> get_instance() {

        static mla_test_service_singleton_t test_service_instance = { 42 };
        return { get_service_name(), &test_service_instance, mla_buffer_reference_noOwner() };
    }
    mla_test_int32_t value;
};


void RegisterGetUnregisterSingletonTest() {

    mla_inject_service_t<mla_test_service_singleton_t> service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_null(service.service, "Service should not be registered yet.");
    assert_false(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should not be registered yet.");

    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register singleton service.");
    assert_true(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should be registered now.");
    service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_not_null(service.service, "Service should be available after registration.");
    assert_equal(service.service->value, (mla_test_int32_t)42, "Service value should be 42.");
    service.service->value = 100;
    service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_equal(service.service->value, (mla_test_int32_t)100, "Service value should be 100.");

    mla_inject_service_t<mla_test_service_singleton_t> service2 = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_equal(service.service, service2.service, "Both service instances should be the same.");

    assert_true(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Failed to unregister singleton service.");
    assert_false(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should not be registered anymore.");
    service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_null(service.service, "Service should not be available after unregistration.");

}

// Define a simple service struct
struct mla_test_service_instance_per_request_t {

    static mla_string_t get_service_name() {
        static mla_string_t serviceName = mla_string("TestServiceInstancePerRequest");
        return serviceName;
    }
    static mla_inject_service_t<void> get_instance() {

        mla_pointer_t data = mla_malloc(sizeof(mla_test_service_instance_per_request_t));
        mla_memset(data, 0, sizeof(mla_test_service_instance_per_request_t));
        mla_test_service_instance_per_request_t* instance = static_cast<mla_test_service_instance_per_request_t*>(data);
        instance->value = 42;
        return { get_service_name(), instance, mla_buffer_reference(instance) };
    }
    mla_test_int32_t value;
};



void RegisterGetUnregisterInstancePerRequestTest() {

    mla_inject_service_t<mla_test_service_instance_per_request_t> service = mla_inject_get_service<mla_test_service_instance_per_request_t>();
    assert_null(service.service, "Service should not be registered yet.");
    assert_false(mla_inject_is_service_registered<mla_test_service_instance_per_request_t>(), "Service should not be registered yet.");


    assert_true(mla_inject_register_service<mla_test_service_instance_per_request_t>(), "Failed to register singleton service.");
    assert_true(mla_inject_is_service_registered<mla_test_service_instance_per_request_t>(), "Service should be registered now.");
    service = mla_inject_get_service<mla_test_service_instance_per_request_t>();
    assert_not_null(service.service, "Service should be available after registration.");
    assert_equal(service.service->value, (mla_test_int32_t)42, "Service value should be 42.");
    service.service->value = 100;
    service = mla_inject_get_service<mla_test_service_instance_per_request_t>();
    assert_not_equal(service.service->value, (mla_test_int32_t)100, "Service value should not be 100, as it's a new instance.");
    assert_equal(service.service->value, (mla_test_int32_t)42, "Service value should be 42.");

    mla_inject_service_t<mla_test_service_instance_per_request_t> service2 = mla_inject_get_service<mla_test_service_instance_per_request_t>();
    assert_not_equal(service.service, service2.service, "Both service instances should be the same.");

    assert_true(mla_inject_unregister_service<mla_test_service_instance_per_request_t>(), "Failed to unregister singleton service.");
    assert_false(mla_inject_is_service_registered<mla_test_service_instance_per_request_t>(), "Service should not be registered anymore.");
    service = mla_inject_get_service<mla_test_service_instance_per_request_t>();
    assert_null(service.service, "Service should not be available after unregistration.");

}


void RegisterInjectTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("Register/Get/Unregister Singleton", test_category, RegisterGetUnregisterSingletonTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Register/Get/Unregister InstancePerRequest", test_category, RegisterGetUnregisterInstancePerRequestTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif