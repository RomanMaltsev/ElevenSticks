#include "server.hpp"
#include "ret_codes.hpp"
#include "rules.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

RetCodes server(Rules game_rules = Rules()) {
    size_t turn = 0;
    if (game_rules.players != 2) {
        std::cerr << "Error: Only 2 players are supported in this version.\n";
        return RetCodes::ERR_NOT_SUPPORTED;
    }
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket() failed");
        return RetCodes::ERR_INTERNAL;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind() failed");
        return RetCodes::ERR_INTERNAL;
    }
    int player[2];
    listen(server_fd, SOMAXCONN);
    for (size_t i = 0; i < 2; ++i) {
        std::cout << "Waiting for player " << (i + 1) << " to connect...\n";
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Error: Failed to accept connection for player " << (i + 1) << ".\n";
            return RetCodes::ERR_CONNECTION_FAILED;
        }
        player[i] = client_fd;
        std::cout << "Player " << (i + 1) << " connected.\n";
    }

    return RetCodes::SUCCESS;
}
