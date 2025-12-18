#include "client.hpp"
#include "ret_codes.hpp"
#include "rules.hpp"
#include "server.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>

struct PlayerSetup {
    bool host_server = false;
    Rules rules = Rules();
};

std::optional<bool> ask_yes_no(const std::string &prompt) {
    while (true) {
        std::cout << prompt;
        char c;
        if (!(std::cin >> c)) return std::nullopt;

        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (c == 'y') return true;
        if (c == 'n') return false;

        std::cout << "Please enter y or n.\n";
    }
}

template <typename T> T ask_number(const std::string &prompt, T def) {
    while (true) {
        std::cout << prompt << " (default " << def << "): ";
        std::string line;
        std::getline(std::cin >> std::ws, line);

        if (line.empty()) return def;

        std::istringstream iss(line);
        T value;
        if (iss >> value) return value;

        std::cout << "Invalid number, try again.\n";
    }
}

Rules ask_rules() {
    size_t count = ask_number("Total stick count", 11UL);
    size_t max_take = ask_number("Maximum sticks per turn", 3UL);
    size_t min_take = ask_number("Minimum sticks per turn", 1UL);
    size_t players = ask_number("Number of players", 2UL);

    if (players != 2) {
        std::cout << "Only 2 players supported, forcing to 2.\n";
        players = 2;
    }

    return Rules(count, max_take, min_take, players);
}

std::optional<PlayerSetup> ask_player() {
    auto host = ask_yes_no("Do you want to host the game server? (y/n): ");
    if (!host) return std::nullopt;

    PlayerSetup setup;
    setup.host_server = *host;

    if (setup.host_server) {
        auto custom = ask_yes_no("Do you want to use custom rules? (y/n): ");
        if (!custom) return std::nullopt;

        setup.rules = *custom ? ask_rules() : Rules();
    }

    return setup;
}

void log_rules(const std::string &who, const Rules &rules) {
    std::cout << "Starting " << who << " with rules: " << rules.count << " sticks, "
              << "max take: " << rules.max_take << ", "
              << "min take: " << rules.min_take << ", "
              << "players: " << rules.players << ".\n";
}

int main() {
    Rules game_rules;
    bool run_server = false;
    std::cout << "Welcome to Eleven Sticks!" << std::endl
              << "This is a simple game where several players take turns removing by "
                 "default 1 to 3 sticks from a pile of 11. The player who takes the last stick loses!"
              << std::endl;
    auto setup = ask_player();
    if (setup->host_server) {
        log_rules("server", setup->rules);

        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Server fork failed: " << strerror(errno) << "\n";
            return static_cast<int>(RetCodes::ERR_INTERNAL);
        }

        if (pid == 0) {
            std::cout << "Server successfully started. "
                      << "port: " << SERVER_PORT << ".\n";
            Server server(game_rules);
            return static_cast<int>(server.run());
        }

        std::cout << "Starting client...";
        Client client;
        return static_cast<int>(client.run());
    } else {
        std::cout << "Starting client...";
        Client client;
        return static_cast<int>(client.run());
    }

    return 0;
}