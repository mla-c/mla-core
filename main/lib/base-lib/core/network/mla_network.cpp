//
// Created by chris on 10/10/2025.
//

#include "mla_network.h"

mla_global mla_network_low_level_operations_t g_network_low_level_operations;

mla_network_ip_address_t mla_network_ip_address_invalid() {
    return {
        mla_string_empty(),
        false
    };
}

mla_network_ip_address_t mla_network_ip_address_ip4(const mla_string_t &address) {
    return {
                address,
                false
            };
}

mla_network_ip_address_t mla_network_ip_address_ip6(const mla_string_t &address) {
    return {
            address,
            true
        };
}

mla_network_host_t mla_network_host_ip4(const mla_string_t &address, mla_uint16_t port) {

    return {
            mla_network_ip_address_ip4(address), port
            };
}

mla_network_host_t mla_network_host_ip6(const mla_string_t &address, mla_uint16_t port) {

    return {
            mla_network_ip_address_ip6(address), port
        };
}

mla_network_host_t mla_network_host_invalid() {

    return {
        {
            mla_string_empty(),
        false,
        }, 0
    };
}

mla_network_connection_t mla_network_connection_disconnected() {

    return {
        mla_network_host_invalid(),
        mla_stream_noop_input(),
        mla_stream_noop_output()
    };
}


mla_array_list_t<mla_init_struct(mla_network_ip_address_t)> mla_network_get_local_ip_addresses() {

    if (g_network_low_level_operations.get_local_ip_addresses == nullptr) {
        return mla_array_list_empty<mla_init_struct(mla_network_ip_address_t)>();
    }

    return g_network_low_level_operations.get_local_ip_addresses();
}


mla_bool_t mla_network_host_resolve(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {

    if (mla_string_length(hostname) == 0) {
        host = mla_network_host_invalid();
        return false;
    }

    return g_network_low_level_operations.resolve_host(host, hostname, port);

}

mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms) {

    if (host.port == 0 || mla_string_length(host.address.address) == 0) {
        return false;
    }

    return g_network_low_level_operations.connect(connection, host, type, timeout_ms);
}

mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection) {

    // By assigning noop streams the connection gets effectively disconnected
    connection.inputStream = mla_stream_noop_input();
    connection.outputStream = mla_stream_noop_output();
    connection.host = mla_network_host_invalid();
    return true;
}

mla_bool_t mla_network_connection_is_connected(const mla_network_connection_t &connection) {
    return connection.host.port != 0 && !mla_string_is_empty(connection.host.address.address);
}

mla_network_listener_t mla_network_listener_invalid() {

    return {
        mla_network_host_invalid(),
        nullptr,
        mla_user_data_empty()
    };
}

mla_bool_t mla_network_listener_bind_and_listen(mla_network_listener_t& listener, const mla_network_host_t &host, mla_connection_type_t type) {

    if (host.port == 0 || mla_string_length(host.address.address) == 0) {
        return false;
    }

    return g_network_low_level_operations.bind_and_listen(listener, host , type);

}

mla_bool_t mla_network_listener_close(mla_network_listener_t& listener) {
    listener = mla_network_listener_invalid();
    return true;
}

mla_bool_t mla_network_listener_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection) {

    if (listener.accept_connection == nullptr) {
        return false;
    }

    return listener.accept_connection(listener, connection);
}