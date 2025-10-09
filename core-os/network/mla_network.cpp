//
// Created by chris on 10/10/2025.
//

#include "mla_network.h"

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

}

mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_size_t timeout_ms) {

}

mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection) {

}