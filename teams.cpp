#include <utility>
#include <deque>
#include <future>
#include <algorithm>
#include <future>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#include <iostream>

#include "teams.hpp"
#include "contest.hpp"
#include "collatz.hpp"
#include "err.h"

ContestResult TeamNewThreads::runContestImpl(ContestInput const & contestInput)
{
    ContestResult result;
    result.resize(contestInput.size());

    rtimers::cxx11::DefaultTimer timer("CalcCollatzNewThreadsTimer");

    uint64_t n_pools = contestInput.size() / this->getSize() + (contestInput.size() % this->getSize() ? 1 : 0);
    for (uint64_t i = 0; i < n_pools; i++) {
        std::vector<std::thread> threads;
        uint64_t limit = std::min((i + 1) * this->getSize(), contestInput.size());
        for (uint64_t idx = i * this->getSize(); idx < limit; idx++)
            threads.push_back(std::move(this->createThread(
                [&result, idx=idx, &timer](const InfInt & singleInput){
                    auto scopedStartStop = timer.scopedStart();
                    result[idx] = calcCollatz(singleInput);
                },
                contestInput[idx]
            )));
        
        std::for_each(threads.begin(), threads.end(),
                    [](std::thread& thread) {thread.join();});
    }
    return result;
}

ContestResult TeamConstThreads::runContestImpl(ContestInput const & contestInput)
{
    ContestResult result;
    result.resize(contestInput.size());

    rtimers::cxx11::DefaultTimer timer("CalcCollatzConstThreadsTimer");

    uint64_t min_n_tasks = contestInput.size() / this->getSize();
    uint64_t remainder = contestInput.size() % this->getSize();
    std::vector<std::thread> threads;
    for (uint64_t i = 0, n_processed = 0; i < this->getSize(); i++) {
        uint64_t n_tasks = min_n_tasks + (i < remainder ? 1 : 0);
        threads.push_back(std::move(this->createThread(
            [&result, &timer, &contestInput](const uint64_t b, const uint64_t e) {
                for (uint64_t idx = b; idx < e; idx++) {
                    auto scopedStartStop = timer.scopedStart();
                    result[idx] = calcCollatz(contestInput[idx]);
                }
            },
            n_processed, n_processed + n_tasks
        )));
        n_processed += n_tasks;
    }

    std::for_each(threads.begin(), threads.end(),
                    [](std::thread& thread) {thread.join();});

    return result;
}

ContestResult TeamPool::runContest(ContestInput const & contestInput)
{
    ContestResult result;
    result.resize(contestInput.size());

    rtimers::cxx11::DefaultTimer timer("CalcCollatzPoolTimer");

    std::vector<std::future<void>> futures;
    for (uint64_t idx = 0; idx < contestInput.size(); idx++)
        futures.push_back(std::move(this->pool.push(
            [&result, idx=idx, &timer](const InfInt & singleInput){
                auto scopedStartStop = timer.scopedStart();
                result[idx] = calcCollatz(singleInput);
            },
            contestInput[idx]
        )));
    
    cxxpool::get(futures.begin(), futures.end());

    return result;
}

ContestResult TeamNewProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult result;
    result.resize(contestInput.size());

    rtimers::cxx11::DefaultTimer timer("CalcCollatzNewProcessesTimer");

    int desc;  
    char *file_name = "/tmp/new_processes_results";
    if (mkfifo(file_name, 0755) == -1) syserr("Error in mkfifo\n");

    uint64_t n_pools = contestInput.size() / this->getSize() + (contestInput.size() % this->getSize() ? 1 : 0);
    for (uint64_t i = 0; i < n_pools; i++) {
        uint64_t limit = std::min((i + 1) * this->getSize(), contestInput.size());
        for (uint64_t idx = i * this->getSize(); idx < limit; idx++) 
            switch (fork()) {                     
                case -1: 
                    syserr("Error in fork\n");
                
                case 0:                             
                    execlp("./new_process", "./new_process", file_name, contestInput[idx].toString().c_str(), NULL); 
                    syserr("Error in execlp\n");

                default:                            
                    break;
            }
        }
    
    desc = open(file_name, O_RDONLY);
    if(desc == -1) syserr("Parent: Error in open\n");

    int n_read_bytes;
    int read_results = 0;
    uint64_t single_result;
    do {
        if ((n_read_bytes = read(desc, &single_result, sizeof(uint64_t))) == -1)
            syserr("Error in read\n");
        else if (n_read_bytes > 0)
            read_results++;
    } while (read_results < contestInput.size());

    if(close(desc)) syserr("Parent, error in close\n");

    if(unlink(file_name)) syserr("Error in unlink:");

    return result;
}

ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    //TODO
    return r;
}

ContestResult TeamAsync::runContest(ContestInput const & contestInput)
{
    ContestResult r;
    //TODO
    return r;
}
