#ifndef _SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef _UNISTD_H
#include <sys/unistd.h>
#endif

#ifndef ELEVEN_STICKS_NET_TOOLS_HPP
#define ELEVEN_STICKS_NET_TOOLS_HPP

bool send_all(int fd, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    while (len > 0) {
        ssize_t sent = send(fd, p, len, 0);
        if (sent <= 0) return false;
        p += sent;
        len -= sent;
    }
    return true;
}
bool recv_all(int fd, void *buf, size_t len) {
    char *p = (char *)buf;
    while (len > 0) {
        ssize_t recvd = recv(fd, p, len, 0);
        if (recvd <= 0) return false;
        p += recvd;
        len -= recvd;
    }
    return true;
}

#endif