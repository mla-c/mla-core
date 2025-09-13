//
// Created by chris on 9/13/2025.
//

#ifndef COREOS_MLA_INJECT_TEST_H
#define COREOS_MLA_INJECT_TEST_H

#include "../core-os/inject/mla_inject.h"
#include "../core-os-test-support/mla_test_executor.h"

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


inline void RegisterGetUnregisterSingletonTest() {

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
    service.service->value = 42;

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



inline void RegisterGetUnregisterInstancePerRequestTest() {

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


// Define a service with dependencies
struct mla_test_service_with_dependency_t {
    static mla_string_t get_service_name() {
        static mla_string_t serviceName = mla_string("TestServiceWithDependency");
        return serviceName;
    }
    static mla_inject_service_t<void> get_instance() {
        static mla_test_service_with_dependency_t test_service_instance = { 99, true };
        return { get_service_name(), &test_service_instance, mla_buffer_reference_noOwner() };
    }
    mla_test_int32_t id;
    mla_bool_t initialized;
};

// Define another service type for testing multiple services
struct mla_test_service_b_t {
    static mla_string_t get_service_name() {
        static mla_string_t serviceName = mla_string("TestServiceB");
        return serviceName;
    }
    static mla_inject_service_t<void> get_instance() {
        static mla_test_service_b_t test_service_instance = { "ServiceB" };
        return { get_service_name(), &test_service_instance, mla_buffer_reference_noOwner() };
    }
    const char* name;
};

// Test multiple services registration and retrieval
inline void MultipleServicesTest() {
    // Clean state
    mla_inject_reset();

    // Register multiple services
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register service A.");
    assert_true(mla_inject_register_service<mla_test_service_b_t>(), "Failed to register service B.");
    assert_true(mla_inject_register_service<mla_test_service_with_dependency_t>(), "Failed to register service with dependency.");

    // Verify all services are registered
    assert_true(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service A should be registered.");
    assert_true(mla_inject_is_service_registered<mla_test_service_b_t>(), "Service B should be registered.");
    assert_true(mla_inject_is_service_registered<mla_test_service_with_dependency_t>(), "Service with dependency should be registered.");

    // Get all services and verify they work
    auto serviceA = mla_inject_get_service<mla_test_service_singleton_t>();
    auto serviceB = mla_inject_get_service<mla_test_service_b_t>();
    auto serviceDep = mla_inject_get_service<mla_test_service_with_dependency_t>();

    assert_not_null(serviceA.service, "Service A should be available.");
    assert_not_null(serviceB.service, "Service B should be available.");
    assert_not_null(serviceDep.service, "Service with dependency should be available.");

    assert_equal(serviceA.service->value, (mla_test_int32_t)42, "Service A value should be 42.");
    assert_equal(serviceDep.service->id, (mla_test_int32_t)99, "Service dependency id should be 99.");
    assert_true(serviceDep.service->initialized, "Service dependency should be initialized.");

    // Clean up
    mla_inject_reset();
}

// Test container locking functionality
inline void ContainerLockingTest() {
    // Clean state
    mla_inject_reset();

    // Register a service before locking
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register service before locking.");
    assert_true(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should be registered before locking.");

    // Lock the container
    mla_inject_lock();

    // Try to register a new service (should fail)
    assert_false(mla_inject_register_service<mla_test_service_b_t>(), "Should not be able to register service when locked.");
    assert_false(mla_inject_is_service_registered<mla_test_service_b_t>(), "Service B should not be registered when locked.");

    // Try to unregister existing service (should fail)
    assert_false(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Should not be able to unregister service when locked.");
    assert_true(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should still be registered when locked.");

    // Getting services should still work
    auto service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_not_null(service.service, "Should still be able to get services when locked.");

    // Unlock the container
    mla_inject_unlock();

    // Now registration should work again
    assert_true(mla_inject_register_service<mla_test_service_b_t>(), "Should be able to register service after unlocking.");
    assert_true(mla_inject_is_service_registered<mla_test_service_b_t>(), "Service B should be registered after unlocking.");

    // Unregistration should work again
    assert_true(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Should be able to unregister service after unlocking.");
    assert_false(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should be unregistered after unlocking.");

    // Clean up
    mla_inject_reset();
}

// Test container reset functionality
inline void ContainerResetTest() {
    // Register multiple services
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register service A.");
    assert_true(mla_inject_register_service<mla_test_service_b_t>(), "Failed to register service B.");

    // Lock the container
    mla_inject_lock();

    // Verify services are registered and container is locked
    assert_true(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service A should be registered before reset.");
    assert_true(mla_inject_is_service_registered<mla_test_service_b_t>(), "Service B should be registered before reset.");

    // Reset should clear everything and unlock
    mla_inject_reset();

    // Verify all services are gone and container is unlocked
    assert_false(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service A should not be registered after reset.");
    assert_false(mla_inject_is_service_registered<mla_test_service_b_t>(), "Service B should not be registered after reset.");

    // Should be able to register services again (proves unlocked)
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Should be able to register service after reset.");

    // Clean up
    mla_inject_reset();
}

// Test unregistering non-existent service
inline void UnregisterNonExistentServiceTest() {
    // Clean state
    mla_inject_reset();

    // Try to unregister a service that was never registered
    assert_false(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Should not be able to unregister non-existent service.");
    assert_false(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should not be registered.");

    // Register and then unregister
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register service.");
    assert_true(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Failed to unregister service.");

    // Try to unregister again (should fail)
    assert_false(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Should not be able to unregister already unregistered service.");

    // Clean up
    mla_inject_reset();
}

// Test getting non-existent service
inline void GetNonExistentServiceTest() {
    // Clean state
    mla_inject_reset();

    // Try to get a service that was never registered
    auto service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_null(service.service, "Should get null for non-existent service.");
    assert_false(mla_inject_is_service_registered<mla_test_service_singleton_t>(), "Service should not be registered.");

    // Register service, get it, then unregister and try to get again
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register service.");
    service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_not_null(service.service, "Should get valid service after registration.");

    assert_true(mla_inject_unregister_service<mla_test_service_singleton_t>(), "Failed to unregister service.");
    service = mla_inject_get_service<mla_test_service_singleton_t>();
    assert_null(service.service, "Should get null for unregistered service.");

    // Clean up
    mla_inject_reset();
}

// Define a service that creates new instances with different IDs
struct mla_test_service_counter_t {
    static mla_string_t get_service_name() {
        static mla_string_t serviceName = mla_string("TestServiceCounter");
        return serviceName;
    }
    static mla_inject_service_t<void> get_instance() {
        static mla_test_int32_t counter = 0;
        mla_pointer_t data = mla_malloc(sizeof(mla_test_service_counter_t));
        mla_memset(data, 0, sizeof(mla_test_service_counter_t));
        mla_test_service_counter_t* instance = static_cast<mla_test_service_counter_t*>(data);
        instance->id = ++counter;
        return { get_service_name(), instance, mla_buffer_reference(instance) };
    }
    mla_test_int32_t id;
};

// Test multiple instances of the same service type
inline void MultipleInstancesTest() {
    // Clean state
    mla_inject_reset();

    assert_true(mla_inject_register_service<mla_test_service_counter_t>(), "Failed to register counter service.");

    // Get multiple instances and verify they're different
    auto service1 = mla_inject_get_service<mla_test_service_counter_t>();
    auto service2 = mla_inject_get_service<mla_test_service_counter_t>();
    auto service3 = mla_inject_get_service<mla_test_service_counter_t>();

    assert_not_null(service1.service, "First service instance should be valid.");
    assert_not_null(service2.service, "Second service instance should be valid.");
    assert_not_null(service3.service, "Third service instance should be valid.");

    // Each instance should have a different ID
    assert_equal(service1.service->id, (mla_test_int32_t)1, "First instance should have ID 1.");
    assert_equal(service2.service->id, (mla_test_int32_t)2, "Second instance should have ID 2.");
    assert_equal(service3.service->id, (mla_test_int32_t)3, "Third instance should have ID 3.");

    // Instances should be different objects
    assert_not_equal(service1.service, service2.service, "First and second instances should be different objects.");
    assert_not_equal(service2.service, service3.service, "Second and third instances should be different objects.");
    assert_not_equal(service1.service, service3.service, "First and third instances should be different objects.");

    // Clean up
    mla_inject_reset();
}

// Test service name consistency
inline void ServiceNameConsistencyTest() {
    // Clean state
    mla_inject_reset();

    // Get service names directly
    mla_string_t singletonName = mla_test_service_singleton_t::get_service_name();
    mla_string_t serviceBName = mla_test_service_b_t::get_service_name();
    mla_string_t dependencyName = mla_test_service_with_dependency_t::get_service_name();

    // Register services
    assert_true(mla_inject_register_service<mla_test_service_singleton_t>(), "Failed to register singleton service.");
    assert_true(mla_inject_register_service<mla_test_service_b_t>(), "Failed to register service B.");
    assert_true(mla_inject_register_service<mla_test_service_with_dependency_t>(), "Failed to register dependency service.");

    // Get services and verify names match
    auto singletonService = mla_inject_get_service<mla_test_service_singleton_t>();
    auto serviceBService = mla_inject_get_service<mla_test_service_b_t>();
    auto dependencyService = mla_inject_get_service<mla_test_service_with_dependency_t>();

    assert_not_null(singletonService.service, "Singleton service should be available.");
    assert_not_null(serviceBService.service, "Service B should be available.");
    assert_not_null(dependencyService.service, "Dependency service should be available.");

    // Verify service names are consistent
    assert_true(mla_string_equals(singletonService.serviceName, singletonName), "Singleton service name should match.");
    assert_true(mla_string_equals(serviceBService.serviceName, serviceBName), "Service B name should match.");
    assert_true(mla_string_equals(dependencyService.serviceName, dependencyName), "Dependency service name should match.");

    // Clean up
    mla_inject_reset();
}

inline void RegisterInjectTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("Register/Get/Unregister Singleton", test_category, RegisterGetUnregisterSingletonTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Register/Get/Unregister InstancePerRequest", test_category, RegisterGetUnregisterInstancePerRequestTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Multiple Services Registration", test_category, MultipleServicesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Container Locking Functionality", test_category, ContainerLockingTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Container Reset Functionality", test_category, ContainerResetTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Unregister Non-Existent Service", test_category, UnregisterNonExistentServiceTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Get Non-Existent Service", test_category, GetNonExistentServiceTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Multiple Instances of Same Service", test_category, MultipleInstancesTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Service Name Consistency", test_category, ServiceNameConsistencyTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif
