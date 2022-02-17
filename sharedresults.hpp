#ifndef SHAREDRESULTS_HPP
#define SHAREDRESULTS_HPP

#include <map>

#include "lib/infint/InfInt.h"
#include "collatz.hpp"

class SharedResults
{
public:

    SharedResults() {
        for (uint64_t i = 0; i < size; i++)
            cache[i] = 0;
    }

    uint64_t get_result(const InfInt & input) {
        if (input > max_key || (input != 1 && cache[input.toUnsignedLongLong()] == 0)) {
            uint64_t res = calcCollatz(input);
            if (input <= max_key)
                cache[input.toUnsignedLongLong()] = res;
            return res;
        }
        return cache[input.toUnsignedLongLong()];
    }

private:
    static const uint64_t size = 1e6;
    static const uint64_t max_key = size - 1;
    uint64_t cache [size];
};

#endif