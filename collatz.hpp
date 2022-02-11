#ifndef COLLATZ_HPP
#define COLLATZ_HPP

#include <assert.h>

inline uint64_t calcCollatz(InfInt n)
{
    // It's ok even if the value overflow
    uint64_t count = 0;
    assert(n > 0);

    while (n != 1)
    {
        ++count;
        if (n % 2 == 1)
        {
            n *= 3;
            n += 1;
        }
        else
        {
            n /= 2;
        }            
    }

    return count;
}

#endif