#ifndef SHAREDRESULTS_HPP
#define SHAREDRESULTS_HPP

#include <map>

#include "lib/infint/InfInt.h"
#include "collatz.hpp"

class SharedResults
{
public:
    uint64_t get_result(const InfInt & input) {
        cache_t::iterator it = this->cache.find(input);
        if (it == this->cache.end()) {
            uint64_t res = calcCollatz(input);
            cache[input] = res;
            return res;
        }
        return it->second;
    }

private:
    using cache_t = std::map<InfInt, uint64_t>;
    cache_t cache;
};

#endif