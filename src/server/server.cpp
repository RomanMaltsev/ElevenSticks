#include "server.hpp"
#include "ret_codes.hpp"
#include "rules.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

RetCodes open_server(int *player, int *server_fd, sockaddr_in *addr) {
    int server_fd_value = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_value < 0) {
        perror("socket() failed");
        return RetCodes::ERR_INTERNAL;
    }
    addr->sin_family = AF_INET;
    addr->sin_port = htons(8080);
    addr->sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd_value, (sockaddr *)addr, sizeof(*addr)) == -1) {
        perror("bind() failed");
        return RetCodes::ERR_INTERNAL;
    }
    listen(server_fd_value, SOMAXCONN);
    for (size_t i = 0; i < 2; ++i) {
        std::cout << "Waiting for player " << (i + 1) << " to connect...\n";
        int client_fd = accept(server_fd_value, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Error: Failed to accept connection for player " << (i + 1) << ".\n";
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        player[i] = client_fd;
        std::cout << "Player " << (i + 1) << " connected.\n";
    }
    return RetCodes::SUCCESS;
}

RetCodes game(Rules game_rules, int *player) {
    size_t sticks_left = game_rules.count;
    size_t turn = 0;
    while (sticks_left > 0) {
        size_t current_player = turn % game_rules.players;
        ssize_t bytes_sent = send(player[current_player], &sticks_left, sizeof(sticks_left), 0);
        if (bytes_sent <= 0) {
            std::cerr << "Error: Failed to send data to player " << (current_player + 1) << ".\n";
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        size_t taken = 0;
        ssize_t bytes_received = recv(player[current_player], &taken, sizeof(taken), 0);
        if (bytes_received <= 0 || taken < game_rules.min_take || taken > game_rules.max_take || taken > sticks_left) {
            std::cerr << "Error: Invalid move by player " << (current_player + 1) << ".\n";
            return RetCodes::ERR_INVALID_ARGS;
        }
        sticks_left -= taken;
        std::cout << "Player " << (current_player + 1) << " took " << taken << " sticks. " << sticks_left << " left.\n";
        turn++;
    }
    return RetCodes::SUCCESS;
}

RetCodes server(Rules game_rules = Rules()) {
    if (game_rules.players != 2) {
        std::cerr << "Error: Only 2 players are supported in this version.\n";
        return RetCodes::ERR_NOT_SUPPORTED;
    }
    int player[2];
    int server_fd;
    sockaddr_in addr{};
    RetCodes ret = open_server(player, &server_fd, &addr);
    if (ret != RetCodes::SUCCESS) {
        return ret;
    }

    ret = game(game_rules, player);
    if (ret != RetCodes::SUCCESS) {
        return ret;
    }

    return RetCodes::SUCCESS;
}
