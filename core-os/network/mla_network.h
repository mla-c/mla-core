//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_NETWORK_H
#define COREOS_MLA_NETWORK_H

#include "../mla_data_types.h"
#include "../system/mla_string.h"

struct mla_network_ip_address {
    mla_string_t address; // IP address in string format
    mla_bool_t is_ipv6;   // True if IPv6, false if IPv4
};

struct mla_network_host {
    mla_network_ip_address address;
    mla_uint16_t port;
};

#endif
