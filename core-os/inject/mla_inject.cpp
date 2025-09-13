//
// Created by chris on 9/13/2025.
//

#include "mla_inject.h"

#include "../log/mla_logging.h"
#include "../system/mla_hash_map.h"
#include "../system/mla_string_concat.h"
#include "mla_inject_services.h"

#define CONST_DEFAULT_SERVICE_FACTORY_LIST_CAPACITY 1

mla_inject_container_t g_inject_container = {
    mla_hash_map<mla_string_t, mla_array_list_t<mla_inject_factory_fn>, mla_string_hash_t, mla_string_initializer,
        mla_array_list_initializer<mla_inject_factory_fn> >(),
    false
};


mla_inject_service_t<mla_void_t> mla_inject_get_service(const mla_string_t &serviceName) {

    mla_array_list_t<mla_inject_factory_fn> factories = mla_array_list_empty<mla_inject_factory_fn>();

    if (mla_hash_map_get(g_inject_container.services, serviceName, factories)) {
        const mla_size_t size = mla_array_list_size(factories);

        if (size > 0) {
            // Use the last registered factory (LIFO)
            mla_inject_factory_fn factory = mla_array_list_get_unsafe(factories, size - 1);

            if (factory == nullptr) {
                mla_error(mla_string_concat("Factory is null for service: ", serviceName));
                return {serviceName, nullptr, mla_buffer_reference_noOwner()};
            }

            return factory();
        } else {
            // No factories available
            // should never happen only there for safety features
            mla_error(mla_string_concat("Empty factory list for service: ", serviceName));
            return {serviceName, nullptr, mla_buffer_reference_noOwner()};
        }
    } else {
        return {serviceName, nullptr, mla_buffer_reference_noOwner()};
    }
}

mla_bool_t mla_inject_is_service_registered(const mla_string_t &serviceName) {
    return mla_hash_map_contains(g_inject_container.services, serviceName);
}

mla_bool_t mla_inject_register_service(const mla_string_t &serviceName, mla_inject_factory_fn factory) {

    if (g_inject_container.isLocked) {
        mla_error(mla_string_concat("Unable to register service ", serviceName, ". The injection container is locked."));
        return false;
    }

    mla_array_list_t<mla_inject_factory_fn>* factories = mla_hash_map_get_ref(g_inject_container.services, serviceName);

    if (factories != nullptr) {
        mla_array_list_add(*factories, factory);
    } else {
        mla_array_list_t<mla_inject_factory_fn> newFactories = mla_array_list<mla_inject_factory_fn>(CONST_DEFAULT_SERVICE_FACTORY_LIST_CAPACITY);
        mla_array_list_add(newFactories, factory);
        mla_hash_map_push(g_inject_container.services, serviceName, newFactories);
    }

    return true;
}

mla_bool_t mla_inject_unregister_service(const mla_string_t& serviceName, mla_inject_factory_fn factory) {

    if (g_inject_container.isLocked) {
        mla_error(mla_string_concat("Unable to unregister service ", serviceName, ". The injection container is locked."));
        return false;
    }

    mla_array_list_t<mla_inject_factory_fn>* factories = mla_hash_map_get_ref(g_inject_container.services, serviceName);

    if (factories == nullptr) {
        return false;
    }

    mla_int32_t indexOf = mla_array_list_index_of(*factories, factory);

    if (indexOf < 0) {
        return false;
    }

    mla_array_list_remove(*factories, indexOf);

    if (mla_array_list_size(*factories) == 0) {
        // Remove the empty factory list
        mla_hash_map_remove(g_inject_container.services, serviceName);
    }

    return true;
}

mla_array_list_t<mla_inject_service_t<mla_void_t>, mla_inject_service_initializer<mla_void_t>> mla_inject_get_all_services(const mla_string_t& serviceName) {

    mla_array_list_t<mla_inject_service_t<mla_void_t>, mla_inject_service_initializer<mla_void_t>> result = mla_array_list_empty<mla_inject_service_t<mla_void_t>, mla_inject_service_initializer<mla_void_t>>();

    if (serviceName.length == 0) {
        // Get all services
        auto keys = mla_hash_map_keys(g_inject_container.services);

        for (mla_size_t i = 0; i < mla_array_list_size(keys); ++i) {
            mla_string_t key = *mla_array_list_get_ref(keys, i);
            mla_array_list_t<mla_inject_factory_fn> factories = mla_array_list_empty<mla_inject_factory_fn>();

            if (mla_hash_map_get(g_inject_container.services, key, factories)) {
                for (mla_size_t j = 0; j < mla_array_list_size(factories); ++j) {
                    mla_inject_factory_fn factory = *mla_array_list_get_ref(factories, j);
                    if (factory != nullptr) {
                        mla_inject_service_t<mla_void_t> service = factory();
                        mla_array_list_add(result, service);
                    }
                }
            }
        }

    } else {
        // Get services by name
        mla_array_list_t<mla_inject_factory_fn> factories = mla_array_list_empty<mla_inject_factory_fn>();

        if (mla_hash_map_get(g_inject_container.services, serviceName, factories)) {

            for (mla_size_t j = 0; j < mla_array_list_size(factories); ++j) {
                mla_inject_factory_fn factory = *mla_array_list_get_ref(factories, j);
                if (factory != nullptr) {
                    mla_inject_service_t<mla_void_t> service = factory();
                    mla_array_list_add(result, service);
                }
            }
        }
    }

    return result;
}

void mla_inject_lock() {

    g_inject_container.isLocked = true;

    // Setup the inject containers

    auto bootstrap_services = mla_inject_get_all_services<mla_inject_bootstrap_service_t>();

    for (mla_size_t i = 0; i < mla_array_list_size(bootstrap_services); ++i) {

        mla_inject_service_t<mla_inject_bootstrap_service_t> service = mla_array_list_get_unsafe(bootstrap_services, i);

        if (service.service && service.service->setup) {

            mla_debug(mla_string_concat("Start bootstrap service: ", service.service->real_service_name));
            service.service->setup(service.service);
        }

    }
}

void mla_inject_unlock() {

    // Teardown the inject containers in reverse order
    auto bootstrap_services = mla_inject_get_all_services<mla_inject_bootstrap_service_t>();

    for (mla_int32_t i = (mla_int32_t)mla_array_list_size(bootstrap_services) - 1; i >= 0; --i) {

        mla_inject_service_t<mla_inject_bootstrap_service_t> service = mla_array_list_get_unsafe(bootstrap_services, i);

        if (service.service && service.service->teardown) {

            mla_debug(mla_string_concat("Stop bootstrap service: ", service.service->real_service_name));

            service.service->teardown(service.service);
        }

    }

    g_inject_container.isLocked = false;
}

void mla_inject_reset() {

    g_inject_container.isLocked = false;
    mla_hash_map_clear(g_inject_container.services);
}
