#ifndef ELEVEN_STICKS_CLIENT_HPP
#define ELEVEN_STICKS_CLIENT_HPP
#include "net_tools.hpp"
#include "ret_codes.hpp"
#include "rules.hpp"

class Client {
  public:
    Client();
    RetCodes run();

  private:
    [[nodiscard]] RetCodes reachServer();
    [[nodiscard]] RetCodes acceptRules();
    void askSticks(uint32_t &take, uint32_t sticks) const;
    [[nodiscard]] RetCodes gameLoop() const;
    sockaddr_in addr{};
    Rules game_rules;
    int server_fd;
    int fd;
};

#endif
