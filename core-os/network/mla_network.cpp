//
// Created by chris on 10/10/2025.
//

#include "mla_network.h"

mla_global mla_network_low_level_operations_t g_network_low_level_operations;

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


mla_bool_t mla_network_host_resolve(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {

    if (hostname.length == 0) {
        host = mla_network_host_invalid();
        return false;
    }

    return g_network_low_level_operations.resolve_host(host, hostname, port);

}

mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_size_t timeout_ms) {

    if (host.port == 0 || host.address.address.length == 0) {
        return false;
    }

    return g_network_low_level_operations.connect(connection, host, timeout_ms);
}

mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection) {

    // By assigning noop streams the connection gets effectively disconnected
    connection.inputStream = mla_stream_noop_input();
    connection.outputStream = mla_stream_noop_output();
    connection.host = mla_network_host_invalid();
    return true;
}