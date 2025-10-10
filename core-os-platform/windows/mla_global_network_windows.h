//
// Created by chris on 10/10/2025.
//

#ifndef COREOS_MLA_GLOBAL_NETWORK_WINDOWS_H
#define COREOS_MLA_GLOBAL_NETWORK_WINDOWS_H

#include "../../core-os/network/mla_network.h"
#include "winsock2.h"
#include "ws2tcpip.h"


mla_bool_t __windows_resolve_host(mla_network_host_t &host, const mla_string_t &hostname, mla_uint16_t port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    addrinfo hints = {
        0,
        AF_UNSPEC, // IPv4 or IPv6
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        nullptr,
        nullptr,
        nullptr
    };
    mla_c_string_t cHostName = mla_string_to_cString(hostname);

    if (cHostName.c_str == nullptr) {
        WSACleanup();
        return false;
    }

    addrinfo *result = nullptr;
    if (getaddrinfo(cHostName.c_str, nullptr, &hints, &result) != 0) {
        WSACleanup();

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
        sockaddr_in *addr = (sockaddr_in *) result->ai_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = false;
        host.port = port;
        // Store ip and port in host structure
    } else if (result->ai_family == AF_INET6) {
        sockaddr_in6 *addr = (sockaddr_in6 *) result->ai_addr;
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);

        host.address.address = mla_string_copy(ip, mla_strlen(ip));
        host.address.is_ipv6 = false;
        host.port = port;
    }

    freeaddrinfo(result);
    WSACleanup();
    return true;
}

mla_buffer_cleanup_mode __windows_socket_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void)userData;
    SOCKET sock = (SOCKET)(uintptr_t)data;
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        WSACleanup();
    }

    return CLEAN_UP_SKIP;
}

mla_size_t __windows_socket_read(const mla_stream_input_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    (void)offset;
    SOCKET sock = (SOCKET)(uintptr_t)input.userdata;
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    int bytesRead = recv(sock, (char*)buffer + offset, (int)length, 0);
    if (bytesRead == SOCKET_ERROR || bytesRead == 0) {
        return 0;
    }

    return (mla_size_t)bytesRead;
}

mla_size_t __windows_socket_remaining_bytes(const mla_stream_input_t& input) {

    SOCKET sock = (SOCKET)(uintptr_t)input.userdata;
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    char buffer[1];
    int result = recv(sock, buffer, 1, MSG_PEEK);

    if (result == SOCKET_ERROR) {
        return 0;
    }

    if (result > 0) {
        return mla_size_max; // Data available but unknown size
    }

    return 0; // No data available

}

mla_size_t __windows_socket_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    (void)offset;
    SOCKET sock = (SOCKET)(uintptr_t)output.userdata;
    if (sock == INVALID_SOCKET) {
        return 0;
    }

    int bytesSent = send(sock, (const char*)buffer + offset, (int)length, 0);
    if (bytesSent == SOCKET_ERROR || bytesSent == 0) {
        return 0;
    }

    return (mla_size_t)bytesSent;
}


mla_bool_t __windows_connect(mla_network_connection_t &connection, const mla_network_host_t &host,
                             mla_connection_type_t type, mla_size_t timeout_ms) {
    connection.host = host;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    // Create socket
    int sockType = (type == mla_connection_type_tcp) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == mla_connection_type_tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    int family = host.address.is_ipv6 ? AF_INET6 : AF_INET;

    SOCKET sock = socket(family, sockType, protocol);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    // Set socket to non-blocking mode for timeout support
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    // Prepare address structure
    sockaddr_storage addr = {
        0,
        {0},
        0,
        {0}
    };
    int addrLen;

    mla_c_string_t cAddress = mla_string_to_cString(host.address.address);
    if (cAddress.c_str == nullptr) {
        closesocket(sock);
        WSACleanup();
        return false;
    }

    if (host.address.is_ipv6) {
        sockaddr_in6 *addr6 = (sockaddr_in6*)&addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(host.port);
        inet_pton(AF_INET6, cAddress.c_str, &addr6->sin6_addr);
        addrLen = sizeof(sockaddr_in6);
    } else {
        sockaddr_in *addr4 = (sockaddr_in*)&addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(host.port);
        inet_pton(AF_INET, cAddress.c_str, &addr4->sin_addr);
        addrLen = sizeof(sockaddr_in);
    }

    if (cAddress.isOwner) {
        mla_free(const_cast<mla_char_t*>(cAddress.c_str));
    }

    // Attempt connection
    int result = connect(sock, (sockaddr*)&addr, addrLen);

    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            // Wait for connection with timeout
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            timeval timeout = {0, 0};
            timeout.tv_sec = (long)timeout_ms / 1000;
            timeout.tv_usec = (long)(timeout_ms % 1000) * 1000;

            result = select(0, nullptr, &writeSet, nullptr, &timeout);
            if (result <= 0) {
                closesocket(sock);
                WSACleanup();
                return false;
            }
        } else {
            closesocket(sock);
            WSACleanup();
            return false;
        }
    }

    // Set socket back to blocking mode
    mode = 0;
    ioctlsocket(sock, FIONBIO, &mode);

    mla_buffer_reference_t ref = mla_buffer_reference((mla_pointer_t)sock, true, __windows_socket_cleanup, 0);

    connection.inputStream = {
          sock,
            __windows_socket_read,
        __windows_socket_remaining_bytes,
        ref
    };

    connection.outputStream = {
        sock,
        __windows_socket_write,
                nullptr,
                ref
    };



    return true;
}

mla_network_low_level_operations_t g_network_low_level_operations = {
    __windows_resolve_host,
    __windows_connect,
};

#endif
