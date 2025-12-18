#ifndef ELEVEN_STICKS_SERVER_HPP
#define ELEVEN_STICKS_SERVER_HPP
#include "logger.hpp"
#include "message_classes.hpp"
#include "net_tools.hpp"
#include "ret_codes.hpp"
#include "rules.hpp"

#include <fstream>
#include <sys/socket.h>
#include <unistd.h>

#define PLAYER_COUNT 2
#define TIMEOUT 5

#define SERVER_PORT 8080

class Server {
  public:
    explicit Server(Rules rules = Rules(), std::string_view file = "log.txt", uint32_t player_count = PLAYER_COUNT);
    ~Server();
    RetCodes run();

  private:
    sockaddr_in addr{};
    Rules game_rules;
    std::ofstream log_file;
    Logger logger;
    int server_fd;
    uint32_t player_count;
    RetCodes game();
    RetCodes open_server();
    int player[PLAYER_COUNT];
};

#endif
