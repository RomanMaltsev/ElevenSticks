#include "server.hpp"
#include "message_classes.hpp"
#include "net_tools.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PLAYER_COUNT 2

RetCodes open_server(int *player, int *server_fd, sockaddr_in *addr, uint32_t player_count = PLAYER_COUNT) {
    int server_fd_value = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_value < 0) {
        perror("socket() failed");
        return RetCodes::ERR_INTERNAL;
    }
    *server_fd = server_fd_value;
    addr->sin_family = AF_INET;
    addr->sin_port = htons(SERVER_PORT);
    addr->sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd_value, (sockaddr *)addr, sizeof(*addr)) == -1) {
        perror("bind() failed");
        return RetCodes::ERR_INTERNAL;
    }
    if (listen(server_fd_value, SOMAXCONN) < 0) {
        perror("listen() failed");
        return RetCodes::ERR_INTERNAL;
    }
    for (uint32_t i = 0; i < player_count; ++i) {
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

RetCodes game(Rules game_rules, int *player, uint32_t player_count = PLAYER_COUNT) {
    uint32_t sticks_left = game_rules.count;
    uint32_t turn = 0;
    while (sticks_left > 0) {
        uint32_t current_player = turn % player_count;
        uint32_t net_sticks = htonl(sticks_left);
        MessageClasses msg_class = MessageClasses::GAME_UPDATE;
        uint32_t msg_class_net = htonl((uint32_t)msg_class);
        for (size_t i = 0; i < player_count; ++i) {
            ssize_t bytes_sent = send_all(player[i], &msg_class_net, sizeof(msg_class_net));
            if (bytes_sent <= 0) {
                std::cerr << "Error: Failed to send data to player " << (current_player + 1) << ".\n";
                return RetCodes::ERR_CONNECTION_FAILED;
            }
            bytes_sent = send_all(player[i], &net_sticks, sizeof(net_sticks));
            if (bytes_sent <= 0) {
                std::cerr << "Error: Failed to send data to player " << (current_player + 1) << ".\n";
                return RetCodes::ERR_CONNECTION_FAILED;
            }
        }

        ssize_t bytes_received = recv_all(player[current_player], &msg_class_net, sizeof(msg_class_net));
        if (bytes_received <= 0 || ntohl(msg_class_net) != (uint32_t)MessageClasses::PLAYER_ACTION) {
            std::cerr << "Error: Invalid move by player " << (current_player + 1) << ".\n";
            return RetCodes::ERR_INVALID_ARGS;
        }
        uint32_t taken = 0;
        bytes_received = recv_all(player[current_player], &taken, sizeof(taken));
        if (bytes_received <= 0 || ntohl(taken) < game_rules.min_take || ntohl(taken) > game_rules.max_take ||
            ntohl(taken) > sticks_left) {
            std::cerr << "Error: Invalid move by player " << (current_player + 1) << ".\n";
            return RetCodes::ERR_INVALID_ARGS;
        }
        taken = ntohl(taken);
        sticks_left -= taken;
        std::cout << "Player " << (current_player + 1) << " took " << taken << " sticks. " << sticks_left << " left.\n";
        turn++;
    }
    uint32_t winner = (turn - 1) % player_count;
    uint32_t net_result = htonl((uint32_t)MessageClasses::GAME_RESULT);
    uint32_t winner_net = htonl(winner + 1);
    std::cout << "Player " << (winner + 1) << " wins!\n";
    for (uint32_t i = 0; i < player_count; ++i) {
        send_all(player[i], &net_result, sizeof(net_result));
        send_all(player[i], &winner_net, sizeof(winner_net));
    }
    return RetCodes::SUCCESS;
}

void server_cleanup(int *player, const int *server_fd, uint32_t player_count = PLAYER_COUNT) {
    if (player != nullptr) {
        for (uint32_t i = 0; i < player_count; ++i) {
            if (player[i] != -1) {
                close(player[i]);
            }
        }
    }
    if (server_fd && *server_fd != -1) {
        close(*server_fd);
    }
}

RetCodes server(Rules game_rules = Rules()) {
    if (game_rules.players != PLAYER_COUNT) {
        std::cerr << "Error: Only " << PLAYER_COUNT << " players are supported in this version.\n";
        return RetCodes::ERR_NOT_SUPPORTED;
    }
    int player[PLAYER_COUNT] = {-1, -1};
    int server_fd = -1;
    sockaddr_in addr{};
    RetCodes ret = open_server(player, &server_fd, &addr);
    if (ret != RetCodes::SUCCESS) {
        server_cleanup(player, &server_fd);
        return ret;
    }

    ret = game(game_rules, player);
    if (ret != RetCodes::SUCCESS) {
        server_cleanup(player, &server_fd);
        return ret;
    }

    server_cleanup(player, &server_fd);
    return RetCodes::SUCCESS;
}
