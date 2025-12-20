#include "client.hpp"
#include "message_classes.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <iostream>

Client::Client() : addr{}, game_rules(Rules()), fd(socket(AF_INET, SOCK_STREAM, 0)) {
    if (fd < 0) {
        perror("socket() failed");
        throw std::runtime_error("socket() failed");
    }
};

[[nodiscard]] RetCodes Client::reachServer() {
    std::string ip;
    uint16_t port;

    std::cout << "Enter server IP: ";
    std::cin >> ip;

    std::cout << "Enter server port: ";
    std::cin >> port;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        perror("Invalid IP address or port");
        return RetCodes::ERR_INVALID_ARGS;
    }
    if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        perror("connect failed");
        return RetCodes::ERR_CONNECTION_FAILED;
    }
    timeval tv{TIMEOUT, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return RetCodes::SUCCESS;
}

[[nodiscard]] RetCodes Client::acceptRules() {
    MessageClasses msg = MessageClasses::INFO;
    size_t payload_size = 0;

    if (!recv_msg(fd, msg, &game_rules, sizeof(Rules), payload_size)) {
        return RetCodes::ERR_CONNECTION_FAILED;
    }

    if (msg != MessageClasses::INFO) {
        return RetCodes::ERR_PROTOCOL;
    }

    return RetCodes::SUCCESS;
}

void Client::askSticks(uint32_t &take, uint32_t sticks) const {
    std::cout << "Your turn! Sticks left: " << sticks << std::endl;
    while (take < game_rules.min_take || take > game_rules.max_take || sticks - take > game_rules.count) {
        std::cout << "Enter number of sticks to take (" << game_rules.min_take << "-"
                  << ((uint32_t)game_rules.max_take < sticks ? game_rules.max_take : sticks) << "): ";
        std::cin >> take;
        if (take < game_rules.min_take || take > game_rules.max_take || sticks - take > game_rules.count) {
            std::cout << "Invalid number of sticks." << std::endl;
        }
    }
}

[[nodiscard]] RetCodes Client::gameLoop() const {

    MessageClasses msg;
    uint32_t sticks_left = game_rules.count;
    uint32_t recieved = 0;
    size_t payload_size = 0;
    do {
        if (!recv_msg(fd, msg, &recieved, sizeof(recieved), payload_size)) {
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        switch (msg) {
        case MessageClasses::GAME_UPDATE:
            if ((sticks_left - recieved > game_rules.max_take || sticks_left - recieved < game_rules.min_take) &&
                recieved != sticks_left) {
                std::cout << "Invalid update from server: " << recieved << " sticks left compared to " << sticks_left
                          << std::endl;
                std::cout << "Server sent invalid update." << std::endl;
            }
            sticks_left = recieved;
            std::cout << "Sticks left: " << sticks_left << std::endl;
            break;
        case MessageClasses::PLAYER_ACTION: {
            uint32_t take = 0;
            askSticks(take, sticks_left);
            if (!send_msg(fd, msg, &take, sizeof(take))) {
                return RetCodes::ERR_CONNECTION_FAILED;
            }
            break;
        }
        case MessageClasses::GAME_RESULT:
            break;
        default:
            return RetCodes::ERR_PROTOCOL;
            break;
        }
    } while (msg != MessageClasses::GAME_RESULT);
    std::cout << "Game over! Winner is player " << recieved << "." << std::endl;

    return RetCodes::SUCCESS;
}

[[nodiscard]] RetCodes Client::run() {
    RetCodes rc;
    rc = reachServer();
    if (rc != RetCodes::SUCCESS) {
        std::cout << "Failed to connect to server." << std::endl;
        return rc;
    }
    rc = acceptRules();
    if (rc != RetCodes::SUCCESS) {
        std::cout << "Failed to accept game rules from server." << std::endl;
        return rc;
    }
    std::cout << "Game rules accepted: " << game_rules.count << " sticks, max take: " << game_rules.max_take
              << ", min take: " << game_rules.min_take << ", players: " << game_rules.players << "." << std::endl;
    rc = gameLoop();
    if (rc != RetCodes::SUCCESS) {
        std::cout << "Game loop ended with error." << std::endl;
        return rc;
    }

    return RetCodes::SUCCESS;
}
