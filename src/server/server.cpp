#include "server.hpp"
#include "message_classes.hpp"
#include <sys/unistd.h>

[[nodiscard]] RetCodes Server::open_server() {
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
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        player[i] = client_fd;
        MessageClasses msg_class = MessageClasses::INFO;
        if (!send_msg(player[i], msg_class, &game_rules, sizeof(game_rules))) {
            logger.error("Error: Failed to send game rules to player " + std::to_string(i + 1) + ".");
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        logger.info("Player " + std::to_string(i + 1) + " connected.");
    }

    return RetCodes::SUCCESS;
}

[[nodiscard]] RetCodes Server::game() {
    uint32_t sticks_left = game_rules.count;
    uint32_t turn = 0;
    while (sticks_left >= game_rules.min_take && sticks_left <= game_rules.count) {
        uint32_t current_player = turn % player_count;
        MessageClasses msg_class = MessageClasses::GAME_UPDATE;
        for (size_t i = 0; i < player_count; ++i) {
            if (!send_msg(player[i], msg_class, &sticks_left, sizeof(sticks_left))) {
                logger.error("Error: Failed to send game update to player " + std::to_string(i + 1) + ".");
            }
            logger.info("Sent game update to player " + std::to_string(i + 1) + ": " + std::to_string(sticks_left) +
                        " sticks left.");
        }
        msg_class = MessageClasses::PLAYER_ACTION;
        send_msg(player[current_player], msg_class, nullptr, 0);
        uint32_t taken = 0;
        size_t payload_size = 0;
        recv_msg(player[current_player], msg_class, &taken, sizeof(taken), payload_size);
        sticks_left -= taken;
        logger.info("Player " + std::to_string(current_player + 1) + " took " + std::to_string(taken) + " sticks. " +
                    std::to_string(sticks_left) + " left.");
        turn++;
    }
    uint32_t winner = ((turn - 1) % player_count) + 1;
    logger.info("Player " + std::to_string(winner) + " wins!");
    for (uint32_t i = 0; i < player_count; ++i) {
        if (!send_msg(player[i], MessageClasses::GAME_RESULT, &winner, sizeof(winner))) {
            logger.error("Error: Failed to send game result to player " + std::to_string(i + 1) + ".");
        }
    }
    return RetCodes::SUCCESS;
}

[[nodiscard]] RetCodes Server::run() {
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
    : game_rules(rules), log_file(file.data(), std::ios::out | std::ios::app), logger(log_file),
      server_fd(socket(AF_INET, SOCK_STREAM, 0)), player_count(player_count), player{-1, -1} {
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
    for (uint32_t i = 0; i < player_count; ++i) {
        if (player[i] != -1) {
            close(player[i]);
        }
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    logger.info("SHUTDOWN...\n");
}