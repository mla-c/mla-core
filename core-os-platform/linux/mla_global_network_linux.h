//
// Created by christian on 10/17/25.
//

#ifndef COREOS_MLA_GLOBAL_NETWORK_LINUX_H
#define COREOS_MLA_GLOBAL_NETWORK_LINUX_H

#include "../../core-os/network/mla_network.h"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

mla_bool_t __linux_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    struct addrinfo hints = {
        0,
        AF_UNSPEC,    // IPv4 or IPv6
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        nullptr,
        nullptr,
        nullptr
    };

    mla_c_string_t cHostName = mla_string_to_cString(hostname);
    if (cHostName.c_str == nullptr) {
        return false;
    }

    struct addrinfo *result = nullptr;
    if (getaddrinfo(cHostName.c_str, nullptr, &hints, &result) != 0) {
        if (cHostName.isOwner) {
            mla_free(const_cast<mla_char_t *>(cHostName.c_str));
        }
        return false;
    }

    if (cHostName.isOwner) {
        mla_free(const_cast<mla_char_t *>(cHostName.c_str));
    }

    // Extract IP address from first result
    if (result->ai_family == AF_INET) {
        struct sockaddr_in *addr = (struct sockaddr_in *) result->ai_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = false;
        host.port = port;
    } else if (result->ai_family == AF_INET6) {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *) result->ai_addr;
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = true;
        host.port = port;
    }

    freeaddrinfo(result);
    return true;
}

mla_buffer_cleanup_mode __linux_socket_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    int sock = (int)(intptr_t)data;
    if (sock >= 0) {
        close(sock);
    }
    return CLEAN_UP_SKIP;
}

mla_size_t __linux_socket_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void)offset;
    int sock = (int)(intptr_t)input.userdata;
    if (sock < 0) {
        return 0;
    }

    ssize_t bytesRead = recv(sock, (char*)buffer + offset, length, 0);
    if (bytesRead <= 0) {
        return 0;
    }

    return (mla_size_t)bytesRead;
}

mla_size_t __linux_socket_remaining_bytes(const mla_stream_input_t& input) {
    int sock = (int)(intptr_t)input.userdata;
    if (sock < 0) {
        return 0;
    }

    char buffer[1];
    ssize_t result = recv(sock, buffer, 1, MSG_PEEK);

    if (result < 0) {
        return 0;
    }

    if (result > 0) {
        return mla_size_max; // Data available but unknown size
    }

    return 0; // No data available
}

mla_size_t __linux_socket_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)offset;
    int sock = (int)(intptr_t)output.userdata;
    if (sock < 0) {
        return 0;
    }

    ssize_t bytesSent = send(sock, (const char*)buffer + offset, length, 0);
    if (bytesSent <= 0) {
        return 0;
    }

    return (mla_size_t)bytesSent;
}

mla_bool_t __linux_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
                           mla_connection_type_t type, mla_size_t timeout_ms) {
    connection.host = host;

    // Create socket
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    int family = host.address.is_ipv6 ? AF_INET6 : AF_INET;

    int sock = socket(family, sockType, protocol);
    if (sock < 0) {
        return false;
    }

    // Set socket to non-blocking mode for timeout support
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    // Prepare address structure
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrLen;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    if (cAddress.c_str == nullptr) {
        close(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6*)&addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        inet_pton(AF_INET6, cAddress.c_str, &addr6->sin6_addr);
        addrLen = sizeof(struct sockaddr_in6);
    } else {
        struct sockaddr_in *addr4 = (struct sockaddr_in*)&addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        inet_pton(AF_INET, cAddress.c_str, &addr4->sin_addr);
        addrLen = sizeof(struct sockaddr_in);
    }

    if (cAddress.isOwner) {
        mla_free(const_cast<mla_char_t*>(cAddress.c_str));
    }

    // Attempt connection
    int result = connect(sock, (struct sockaddr*)&addr, addrLen);

    if (result < 0) {
        if (errno == EINPROGRESS) {
            // Wait for connection with timeout
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            struct timeval timeout = {0, 0};
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;

            result = select(sock + 1, nullptr, &writeSet, nullptr, &timeout);
            if (result <= 0) {
                close(sock);
                return false;
            }

            // Check if connection was successful
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                close(sock);
                return false;
            }
        } else {
            close(sock);
            return false;
        }
    }

    // Set socket back to blocking mode
    fcntl(sock, F_SETFL, flags);

    mla_buffer_reference_t ref = mla_buffer_reference((mla_pointer_t)(intptr_t)sock, true, __linux_socket_cleanup, 0);

    connection.inputStream = {
        (mla_callback_userdata)sock,
        __linux_socket_read,
        __linux_socket_remaining_bytes,
        ref
    };

    connection.outputStream = {
        (mla_callback_userdata)sock,
        __linux_socket_write,
        nullptr,
        ref
    };

    return true;
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    __linux_resolve_host,
    __linux_connect,
};


#endif