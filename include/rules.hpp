#ifndef ELEVEN_STICKS_RULES_HPP
#define ELEVEN_STICKS_RULES_HPP
#include <cstddef>

#define TIMEOUT 60

struct Rules {
    size_t count;
    size_t max_take;
    size_t min_take;
    size_t players;
    explicit Rules(size_t c = 11UL, size_t mx = 3UL, size_t mn = 1UL, size_t p = 2UL)
        : count(c), max_take(mx), min_take(mn), players(p) {}
};

#endif
