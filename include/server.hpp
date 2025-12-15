#ifndef ELEVEN_STICKS_SERVER_HPP
#define ELEVEN_STICKS_SERVER_HPP
#include "ret_codes.hpp"
#include "rules.hpp"

#define SERVER_PORT 8080

RetCodes server(Rules game_rules = Rules());

#endif
