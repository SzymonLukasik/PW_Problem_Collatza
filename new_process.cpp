#include <iostream>
#include <fcntl.h>
#include <unistd.h>


#include "lib/infint/InfInt.h"

#include "collatz.hpp"
#include "err.h"

int main(int argc, char *argv[])
{
    int desc;
    desc = open(argv[1], O_WRONLY);
    if(desc == -1) syserr("Child: Error in open\n");

    for (int i = 0; i < argc - 1; i++) {
        uint64_t result = calcCollatz(argv[1 + i]);
        if (write(desc, &result, sizeof(uint64_t)) < 0)
            syserr("Error in write\n");
    }
    if(close(desc)) syserr("Error in close\n");

    exit(0);
}