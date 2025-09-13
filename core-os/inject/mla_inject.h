//
// Created by chris on 9/13/2025.
//

#ifndef COREOS_MLA_INJECT_H
#define COREOS_MLA_INJECT_H

#include "../system/mla_string.h"
#include "../system/mla_array_list.h"
#include "../system/mla_hash_map.h"
#include "../system/mla_buffer.h"

//////////////////////////////////////////////////
/// Basic service injection system
//////////////////////////////////////////////////

template <typename T>
struct mla_inject_service_t {
    mla_string_t serviceName;
    T* service;
    mla_buffer_reference_t serviceOwner;
};

template <typename T>
struct  mla_inject_service_initializer {
    static mla_inject_service_t<T> init() {
        return { mla_string_empty(), nullptr, mla_buffer_reference_noOwner() };
    }
};

typedef mla_inject_service_t<mla_void_t> (*mla_inject_factory_fn)(void);

// Read functions
mla_inject_service_t<mla_void_t> mla_inject_get_service(const mla_string_t& serviceName);
mla_bool_t mla_inject_is_service_registered(const mla_string_t& serviceName);
const mla_array_list_t<mla_inject_service_t<mla_void_t>, mla_inject_service_initializer<mla_void_t>> mla_inject_get_all_services(const mla_string_t& serviceName);

// Modify functions
mla_bool_t mla_inject_register_service(const mla_string_t& serviceName, mla_inject_factory_fn factory);
mla_bool_t mla_inject_unregister_service(const mla_string_t& serviceName, mla_inject_factory_fn factory);

// Locking functions
void mla_inject_lock();
void mla_inject_unlock();

// Reset function
// Resets the entire injection container, removing all registered services
void mla_inject_reset();

//////////////////////////////////////////////////
/// Templates for easier usage
//////////////////////////////////////////////////

template <typename T>
mla_inject_service_t<T> mla_inject_get_service() {

    mla_string_t serviceName = T::get_service_name();
    mla_inject_service_t<mla_void_t> userdata = mla_inject_get_service(serviceName);

    return {
        userdata.serviceName,
        static_cast<T*>(userdata.service),
        userdata.serviceOwner
    };
}

template <typename T>
mla_bool_t mla_inject_is_service_registered() {
    mla_string_t serviceName = T::get_service_name();
    return mla_inject_is_service_registered(serviceName);
}

template <typename T>
const mla_array_list_t<mla_inject_service_t<T>> mla_inject_get_all_services() {

    mla_string_t serviceName = T::get_service_name();
    auto services = mla_inject_get_all_services(serviceName);

    auto serviceList = mla_array_list<mla_inject_service_t<T>, mla_inject_service_initializer<T>>(mla_array_list_size(services));

    for (mla_size_t i = 0; i < mla_array_list_size(serviceList); ++i) {
        mla_inject_service_t<mla_void_t> userdata = *mla_array_list_get_ref(services, i);

        mla_array_list_add(serviceList, {
            userdata.serviceName,
            static_cast<T*>(userdata.service),
            userdata.serviceOwner
        });
    }
    return serviceList;
}

template <typename T>
mla_bool_t mla_inject_register_service(mla_inject_factory_fn factory) {
    mla_string_t serviceName = T::get_service_name();
    return mla_inject_register_service(serviceName, factory);
}

template <typename T>
mla_bool_t mla_inject_unregister_service(mla_inject_factory_fn factory) {
    mla_string_t serviceName = T::get_service_name();
    return mla_inject_unregister_service(serviceName, factory);
}

template <typename T>
mla_bool_t mla_inject_register_singleton() {
    mla_string_t serviceName = T::get_service_name();
    return mla_inject_register_service(serviceName, T::get_instance);
}

template <typename T>
mla_bool_t mla_inject_unregister_singleton() {
    mla_string_t serviceName = T::get_service_name();
    return mla_inject_unregister_service(serviceName, T::get_instance);
}

//////////////////////////////////////////////////
/// Container
//////////////////////////////////////////////////

struct mla_inject_container_t {
    mla_hash_map_t<mla_string_t, mla_array_list_t<mla_inject_factory_fn>, mla_string_hash_t, mla_string_initializer, mla_array_list_initializer<mla_inject_factory_fn>> services;
    mla_bool_t isLocked;
};

#endif