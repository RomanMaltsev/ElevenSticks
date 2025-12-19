#include "server.hpp"
#include "message_classes.hpp"
#include <sys/unistd.h>

RetCodes Server::open_server() {
    if (listen(server_fd, SOMAXCONN) < 0) {
        logger.error("listen() failed");
        return RetCodes::ERR_INTERNAL;
    }
    for (uint32_t i = 0; i < player_count; ++i) {
        logger.info("Waiting for player " + std::to_string(i + 1) + " to connect (timeout " + std::to_string(TIMEOUT) +
                    "s)...");

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        timeval tv{};
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        int ready = select(server_fd + 1, &readfds, nullptr, nullptr, &tv);
        if (ready == 0) {
            logger.error("Timeout waiting for player " + std::to_string(i + 1) + ".");
            return RetCodes::ERR_TIMEOUT;
        }
        if (ready < 0) {
            logger.error("select() failed");
            return RetCodes::ERR_INTERNAL;
        }

        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            logger.error("accept() failed");
            return RetCodes::ERR_CONNECTION_FAILED;
        }

        player[i] = client_fd;
        MessageClasses msg_class = MessageClasses::INFO;
        uint32_t msg_class_net = htonl((uint32_t)msg_class);
        ssize_t bytes_sent = send_all(player[i], &msg_class_net, sizeof(msg_class_net));
        if (bytes_sent <= 0) {
            logger.error("Error: Failed to send data to player " + std::to_string(i + 1) + ".");
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        bytes_sent = send_all(player[i], &game_rules, sizeof(game_rules));
        if (bytes_sent <= 0) {
            logger.error("Error: Failed to send data to player " + std::to_string(i + 1) + ".");
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        logger.info("Player " + std::to_string(i + 1) + " connected.");
    }

    return RetCodes::SUCCESS;
}

RetCodes Server::game() {
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
                logger.error("Error: Failed to send data to player " + std::to_string(current_player + 1) + ".");
                return RetCodes::ERR_CONNECTION_FAILED;
            }
            bytes_sent = send_all(player[i], &net_sticks, sizeof(net_sticks));
            if (bytes_sent <= 0) {
                logger.error("Error: Failed to send data to player " + std::to_string(current_player + 1) + ".");
                return RetCodes::ERR_CONNECTION_FAILED;
            }
        }

        ssize_t bytes_received = recv_all(player[current_player], &msg_class_net, sizeof(msg_class_net));
        if (bytes_received <= 0 || ntohl(msg_class_net) != (uint32_t)MessageClasses::PLAYER_ACTION) {
            logger.error("Error: Invalid move by player " + std::to_string(current_player + 1) + ".");
            return RetCodes::ERR_INVALID_ARGS;
        }
        uint32_t taken = 0;
        bytes_received = recv_all(player[current_player], &taken, sizeof(taken));
        if (bytes_received <= 0 || ntohl(taken) < game_rules.min_take || ntohl(taken) > game_rules.max_take ||
            ntohl(taken) > sticks_left) {
            logger.error("Error: Invalid move by player " + std::to_string(current_player + 1) + ".");
            return RetCodes::ERR_INVALID_ARGS;
        }
        taken = ntohl(taken);
        sticks_left -= taken;
        logger.info("Player " + std::to_string(current_player + 1) + " took " + std::to_string(taken) + " sticks. " +
                    std::to_string(sticks_left) + " left.");
        turn++;
    }
    uint32_t winner = (turn - 1) % player_count;
    uint32_t net_result = htonl((uint32_t)MessageClasses::GAME_RESULT);
    uint32_t winner_net = htonl(winner + 1);
    logger.info("Player " + std::to_string(winner + 1) + " wins!");
    for (uint32_t i = 0; i < player_count; ++i) {
        send_all(player[i], &net_result, sizeof(net_result));
        send_all(player[i], &winner_net, sizeof(winner_net));
    }
    return RetCodes::SUCCESS;
}

RetCodes Server::run() {
    RetCodes ret = open_server();
    if (ret != RetCodes::SUCCESS) {
        return ret;
    }

    ret = game();
    if (ret != RetCodes::SUCCESS) {
        return ret;
    }
    return RetCodes::SUCCESS;
}

Server::Server(Rules rules, std::string_view file, uint32_t player_count)
    : game_rules(rules), log_file(file.data(), std::ios::out | std::ios::app), logger(log_file), player{-1, -1},
      server_fd(socket(AF_INET, SOCK_STREAM, 0)), player_count(player_count) {
    if (!log_file.is_open()) {
        throw std::runtime_error("Cannot open log file");
    }
    if (server_fd < 0) {
        logger.error("socket() failed");
        throw std::runtime_error("socket() failed");
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1) {
        logger.error("bind() failed");
        throw std::runtime_error("bind() failed");
    }

    logger.info("Server initialized");
}

Server::~Server() {
    if (player != nullptr) {
        for (uint32_t i = 0; i < player_count; ++i) {
            if (player[i] != -1) {
                close(player[i]);
            }
        }
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    logger.info("SHUTDOWN...\n");
}