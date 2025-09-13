//
// Created by chris on 9/13/2025.
//

#ifndef COREOS_MLA_INJECT_TEST_H
#define COREOS_MLA_INJECT_TEST_H

#include "../core-os/inject/mla_inject.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

const mla_string_t serviceName = mla_string("TestService");

// Define a simple service struct
struct TestService {

    static mla_string_t get_service_name() {
        return serviceName;
    }
    static mla_inject_service_t<void> get_instance() {

        static TestService test_service_instance = { 42 };
        return { serviceName, &test_service_instance, mla_buffer_reference_noOwner() };
    }
    mla_test_int32_t value;
};


void RegisterGetUnregisterSingletonTest() {

    mla_inject_service_t<TestService> service = mla_inject_get_service<TestService>();
    assert_null(service.service, "Service should not be registered yet.");
    assert_false(mla_inject_is_service_registered<TestService>(), "Service should not be registered yet.");

    assert_true(mla_inject_register_singleton<TestService>(), "Failed to register singleton service.");
    assert_true(mla_inject_is_service_registered<TestService>(), "Service should be registered now.");
    service = mla_inject_get_service<TestService>();
    assert_not_null(service.service, "Service should be available after registration.");
    assert_equal(service.service->value, (mla_test_int32_t)42, "Service value should be 42.");
    service.service->value = 100;
    service = mla_inject_get_service<TestService>();
    assert_equal(service.service->value, (mla_test_int32_t)100, "Service value should be 100.");
    assert_true(mla_inject_unregister_singleton<TestService>(), "Failed to unregister singleton service.");
    assert_false(mla_inject_is_service_registered<TestService>(), "Service should not be registered anymore.");
    service = mla_inject_get_service<TestService>();
    assert_null(service.service, "Service should not be available after unregistration.");

}


void RegisterInjectTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("Register/Get/Unregister Singleton", test_category, RegisterGetUnregisterSingletonTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif