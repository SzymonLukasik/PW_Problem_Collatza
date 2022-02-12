#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <list>
#include "lib/infint/InfInt.h"

#include "collatz.hpp"
#include "err.h"

int main(int argc, char *argv[])
{
    int desc_output;
    
    if (*argv[1] == '0') {
        uint64_t idx = std::stoull(argv[3]);
        uint64_t result = calcCollatz(InfInt(argv[4]));
        std::pair<uint64_t, uint64_t> pair = {idx, result};
        if ((desc_output = open(argv[2], O_WRONLY)) == -1) syserr("Child: Error in open\n");
        if (write(desc_output, &pair, sizeof(std::pair<uint64_t, uint64_t>)) < 0)
            syserr("Error in write\n");
    } else {
        uint64_t n_tasks = std::stoull(argv[4]);
        int desc_input;
        std::list<std::pair<uint64_t, InfInt>> input_pairs;
        std::pair<uint64_t, InfInt> input_pair;
        if ((desc_input = open(argv[3], O_RDONLY)) == -1) syserr("Child: Error in open\n");
        for (uint64_t i = 0; i < n_tasks; i++) {
            if (read(desc_input, &input_pair, sizeof(std::pair<uint64_t, InfInt>)) == -1)
                syserr("Error in read\n");
            input_pairs.push_back(input_pair);
        }
        if(close(desc_input)) syserr("Error in close\n");

        if ((desc_output = open(argv[2], O_WRONLY)) == -1) syserr("Child: Error in open\n");
        for (const std::pair<uint64_t, InfInt> & input_pair : input_pairs) {
            std::pair<uint64_t, uint64_t> output_pair = {input_pair.first, calcCollatz(input_pair.second)};
            if (write(desc_output, &output_pair, sizeof(std::pair<uint64_t, uint64_t>)) < 0)
                syserr("Error in write\n");
        }
    }

    if(close(desc_output)) syserr("Error in close\n");
    exit(0);
}