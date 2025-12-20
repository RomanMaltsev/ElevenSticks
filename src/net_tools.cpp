#include "net_tools.hpp"

#include <iostream>

bool send_all(int fd, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    while (len > 0) {
        ssize_t sent = send(fd, p, len, 0);
        if (sent < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (sent == 0) return false;
        p += sent;
        len -= sent;
    }
    return true;
}
bool recv_all(int fd, void *buf, size_t len) {
    char *p = (char *)buf;
    while (len > 0) {
        ssize_t recvd = recv(fd, p, len, 0);
        if (recvd < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (recvd == 0) return false;
        p += recvd;
        len -= recvd;
    }
    return true;
}

bool send_msg(int fd, MessageClasses type, const void *payload, size_t payload_size) {
    uint32_t header[2];
    header[0] = htonl(static_cast<uint32_t>(type));
    header[1] = htonl(static_cast<uint32_t>(payload_size));
    if (!send_all(fd, header, sizeof(header))) return false;

    if (payload_size > 0) {
        if (!send_all(fd, payload, payload_size)) return false;
    }
    return true;
}

bool recv_msg(int fd, MessageClasses &type, void *payload, size_t payload_capacity, size_t &payload_size) {
    uint32_t header[2];
    if (!recv_all(fd, header, sizeof(header))) {
        return false;
    }
    type = static_cast<MessageClasses>(ntohl(header[0]));
    payload_size = ntohl(header[1]);

    if (payload_size > payload_capacity) {
        return false;
    }

    if (payload_size > 0) {
        if (!recv_all(fd, payload, payload_size)) {
            return false;
        }
    }
    return true;
}
