#ifndef ELEVEN_STICKS_NET_TOOLS_HPP
#define ELEVEN_STICKS_NET_TOOLS_HPP
#include "message_classes.hpp"
#include <cstdint>
#include <netinet/in.h>
bool send_msg(int fd, MessageClasses type, const void *payload, size_t payload_size);
bool recv_msg(int fd, MessageClasses &type, void *payload, size_t payload_capacity, size_t &payload_size);

#endif