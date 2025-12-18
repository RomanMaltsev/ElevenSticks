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
    sockaddr_in addr{};
    Rules game_rules;
    int server_fd;
    int fd;
};

#endif
