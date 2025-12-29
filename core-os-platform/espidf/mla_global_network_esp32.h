//
// Created by copilot on 12/28/2024.
//

#ifndef COREOS_MLA_GLOBAL_NETWORK_ESP32_H
#define COREOS_MLA_GLOBAL_NETWORK_ESP32_H

#include "../../core-os/network/mla_network.h"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

mla_bool_t __esp32_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    struct addrinfo hints;
    mla_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

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
    } else {
        // Unsupported address family
        freeaddrinfo(result);
        return false;
    }

    freeaddrinfo(result);
    return true;
}

mla_buffer_cleanup_mode __esp32_socket_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    int sock = (int)(intptr_t)data;
    if (sock >= 0) {
        close(sock);
    }
    return CLEAN_UP_SKIP;
}

mla_size_t __esp32_socket_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
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

mla_size_t __esp32_socket_remaining_bytes(const mla_stream_input_t& input) {
    int sock = (int)(intptr_t)input.userdata;
    if (sock < 0) {
        return 0;
    }

    int pending = 0;
    if (ioctl(sock, FIONREAD, &pending) == 0) {
        if (pending > 0) {
            return mla_size_max;
        }
    }

    return 0;
}

mla_size_t __esp32_socket_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
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

mla_bool_t __esp32_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
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

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    if (type == mla_connection_type_tcp) {
        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    }

    mla_buffer_reference_t ref = mla_buffer_reference((mla_pointer_t)(intptr_t)sock, true, __esp32_socket_cleanup, 0);

    connection.inputStream = {
        (mla_callback_userdata)sock,
        __esp32_socket_read,
        __esp32_socket_remaining_bytes,
        ref
    };

    connection.outputStream = {
        (mla_callback_userdata)sock,
        __esp32_socket_write,
        nullptr,
        ref
    };

    return true;
}

mla_bool_t __esp32_accept_connection(const mla_network_listener_t& listener, mla_network_connection_t &connection) {
    int listenSock = (int)(intptr_t)listener.userdata;
    if (listenSock < 0) {
        return false;
    }

    int sockType = 0;
    socklen_t optLen = sizeof(sockType);
    if (getsockopt(listenSock, SOL_SOCKET, SO_TYPE, &sockType, &optLen) != 0 || sockType != SOCK_STREAM) {
        return false;
    }

    struct sockaddr_storage clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSock < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No pending connection; non-blocking accept
            return false;
        }
        return false;
    }

    // Set accepted socket to blocking mode for normal I/O
    int flags = fcntl(clientSock, F_GETFL, 0);
    fcntl(clientSock, F_SETFL, flags & ~O_NONBLOCK);

    // Disable Nagle's algorithm (TCP_NODELAY) by default for better responsiveness
    int nodelay = 1;
    setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    // Fill connection.host from peer address
    mla_network_host_t peer = mla_network_host_invalid();
    char ip4[INET_ADDRSTRLEN] = {0};
    char ip6[INET6_ADDRSTRLEN] = {0};

    if (clientAddr.ss_family == AF_INET) {
        struct sockaddr_in* a4 = (struct sockaddr_in*)&clientAddr;
        inet_ntop(AF_INET, &a4->sin_addr, ip4, sizeof(ip4));
        peer.address.address = mla_string_copy(ip4, mla_strlen(ip4));
        peer.address.is_ipv6 = false;
        peer.port = ntohs(a4->sin_port);
    } else if (clientAddr.ss_family == AF_INET6) {
        struct sockaddr_in6* a6 = (struct sockaddr_in6*)&clientAddr;
        inet_ntop(AF_INET6, &a6->sin6_addr, ip6, sizeof(ip6));
        peer.address.address = mla_string_copy(ip6, mla_strlen(ip6));
        peer.address.is_ipv6 = true;
        peer.port = ntohs(a6->sin6_port);
    } else {
        close(clientSock);
        return false;
    }

    connection.host = peer;

    mla_buffer_reference_t ref = mla_buffer_reference((mla_pointer_t)(intptr_t)clientSock, true, __esp32_socket_cleanup, 0);

    connection.inputStream = {
        (mla_callback_userdata)clientSock,
        __esp32_socket_read,
        __esp32_socket_remaining_bytes,
        ref
    };

    connection.outputStream = {
        (mla_callback_userdata)clientSock,
        __esp32_socket_write,
        nullptr,
        ref
    };

    return true;
}

mla_bool_t __esp32_bind_and_listen(mla_network_listener_t &listener, const mla_network_host_t &host, mla_connection_type_t type) {
    listener.host = host;

    int family = host.address.is_ipv6 ? AF_INET6 : AF_INET;
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;

    int sock = socket(family, sockType, protocol);
    if (sock < 0) {
        return false;
    }

    // Enable address reuse
    int reuseAddr = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));

    // Allow dual-stack for IPv6
    if (family == AF_INET6) {
        int v6only = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
    }

    struct sockaddr_storage ss;
    mla_memset(&ss, 0, sizeof(ss));
    socklen_t addrLen = 0;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    if (cAddress.c_str == nullptr) {
        close(sock);
        return false;
    }

    if (host.address.is_ipv6) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&ss;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET6, cAddress.c_str, &addr6->sin6_addr) == 1);
        if (!ok) {
            addr6->sin6_addr = in6addr_any;
        }
        addrLen = sizeof(struct sockaddr_in6);
    } else {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&ss;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        mla_bool_t ok = (inet_pton(AF_INET, cAddress.c_str, &addr4->sin_addr) == 1);
        if (!ok) {
            addr4->sin_addr.s_addr = htonl(INADDR_ANY);
        }
        addrLen = sizeof(struct sockaddr_in);
    }

    if (cAddress.isOwner) {
        mla_free(const_cast<mla_char_t*>(cAddress.c_str));
    }

    if (bind(sock, (struct sockaddr*)&ss, addrLen) < 0) {
        close(sock);
        return false;
    }

    if (type == mla_connection_type_tcp) {
        if (listen(sock, SOMAXCONN) < 0) {
            close(sock);
            return false;
        }

        // Make the listening socket non-blocking so accept() will not block
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

    listener.listenerOwner = mla_buffer_reference((mla_pointer_t)(intptr_t)sock, true, __esp32_socket_cleanup, 0);
    listener.accept_connection = __esp32_accept_connection;
    listener.userdata = (mla_callback_userdata)sock;

    return true;
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    __esp32_resolve_host,
    __esp32_connect,
    __esp32_bind_and_listen
};

#endif
