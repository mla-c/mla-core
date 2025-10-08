//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_NETWORK_H
#define COREOS_MLA_NETWORK_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"

struct mla_network_ip_address_t {
    mla_string_t address; // IP address in string format
    mla_bool_t is_ipv6;   // True if IPv6, false if IPv4
};

struct mla_network_host_t {
    mla_network_ip_address_t address;
    mla_uint16_t port;
};

mla_network_host_t mla_network_host_invalid();

struct mla_network_connection_t {
    const mla_network_host_t host;
    mla_stream_input_t inputStream;
    mla_stream_output_t outputStream;
};

mla_network_connection_t mla_network_connection_disconnected();

mla_bool_t mla_network_host_resolve(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port);
mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_size_t timeout_ms);
mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection);

#endif
