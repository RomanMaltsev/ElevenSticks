#include "client.hpp"

#include <arpa/inet.h>
#include <iostream>

Client::Client() : fd(socket(AF_INET, SOCK_STREAM, 0)), server_fd(-1), addr{} {
    if (server_fd < 0) {
        perror("socket() failed");
        throw std::runtime_error("socket() failed");
    }
    std::string ip;
    uint16_t port;

    std::cout << "Enter server IP: ";
    std::cin >> ip;

    std::cout << "Enter server port: ";
    std::cin >> port;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        throw std::runtime_error("Invalid IP address");
    }
};

RetCodes Client::run() {
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        return RetCodes::ERR_CONNECTION_FAILED;
    }

    return RetCodes::SUCCESS;
}
