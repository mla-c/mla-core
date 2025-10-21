//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_NETWORK_H
#define COREOS_MLA_NETWORK_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"
#include "../system/mla_stream.h"

enum mla_connection_type_t: mla_uint8_t {
    mla_connection_type_tcp,
    mla_connection_type_udp
};

struct mla_network_ip_address_t {
    mla_string_t address; // IP address in string format
    mla_bool_t is_ipv6;   // True if IPv6, false if IPv4
};

struct mla_network_host_t {
    mla_network_ip_address_t address;
    mla_uint16_t port;
};

mla_network_host_t mla_network_host_invalid();

//////////////////////////////////////////////////////////////////
/// Client Connection
//////////////////////////////////////////////////////////////////

struct mla_network_connection_t {
    mla_network_host_t host;
    mla_stream_input_t inputStream;
    mla_stream_output_t outputStream;
};

mla_network_connection_t mla_network_connection_disconnected();

mla_bool_t mla_network_host_resolve(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port);
mla_bool_t mla_network_connection_connect(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms);
mla_bool_t mla_network_connection_disconnect(mla_network_connection_t &connection);

//////////////////////////////////////////////////////////////////
/// Server Operations
//////////////////////////////////////////////////////////////////

struct mla_network_listener_t {
    mla_network_host_t host;
    mla_buffer_reference_t listenerOwner;
    mla_bool_t (*accept_connection)(const mla_network_listener_t& listener, mla_network_connection_t &connection);
    mla_callback_userdata userdata;
};

mla_network_listener_t mla_network_listener_invalid();

mla_bool_t mla_network_listener_bind_and_listen(mla_network_listener_t& listener, const mla_network_host_t &host, mla_connection_type_t type);
mla_bool_t mla_network_listener_close(mla_network_listener_t& listener);
mla_bool_t mla_network_listener_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection);

/////////////////////////////////////////////////////////////////
/// Low Level Operations
/////////////////////////////////////////////////////////////////

struct mla_network_low_level_operations_t {
    mla_bool_t (*resolve_host)(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port);
    mla_bool_t (*connect)(mla_network_connection_t &connection, const mla_network_host_t &host, mla_connection_type_t type, mla_size_t timeout_ms);
    mla_bool_t (*bind_and_listen)(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type);
};



#endif
