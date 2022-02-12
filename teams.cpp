#include <utility>
#include <deque>
#include <future>
#include <algorithm>
#include <future>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>

#include <iostream>

#include "teams.hpp"
#include "contest.hpp"
#include "collatz.hpp"
#include "err.h"

ContestResult TeamNewThreads::runContestImpl(ContestInput const & contestInput)
{
    rtimers::cxx11::DefaultTimer timer("CalcCollatzNewThreadsTimer");

    ContestResult result;
    result.resize(contestInput.size());
    uint64_t n_pools = contestInput.size() / this->getSize() + (contestInput.size() % this->getSize() ? 1 : 0);

    for (uint64_t i = 0; i < n_pools; i++) {
        std::vector<std::thread> threads;
        uint64_t limit = std::min((i + 1) * this->getSize(), contestInput.size());

        for (uint64_t idx = i * this->getSize(); idx < limit; idx++)
            threads.push_back(std::move(this->createThread(
                [&result, idx=idx, &timer, this](const InfInt & singleInput){
                    auto scopedStartStop = timer.scopedStart();
                    result[idx] = (this->getSharedResults() 
                                   ? this->getSharedResults()->get_result(singleInput)
                                   : calcCollatz(singleInput));
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
    rtimers::cxx11::DefaultTimer timer("CalcCollatzConstThreadsTimer");

    ContestResult result;
    result.resize(contestInput.size());
    uint64_t min_n_tasks = contestInput.size() / this->getSize();
    uint64_t remainder = contestInput.size() % this->getSize();
    std::vector<std::thread> threads;
    
    for (uint64_t i = 0, processed = 0; i < this->getSize(); i++) {
        uint64_t n_tasks = min_n_tasks + (i < remainder ? 1 : 0);
        threads.push_back(std::move(this->createThread(
            [&result, &timer, &contestInput, this](const uint64_t b, const uint64_t e) {
                for (uint64_t idx = b; idx < e; idx++) {
                    auto scopedStartStop = timer.scopedStart();
                    result[idx] = (this->getSharedResults() 
                                   ? this->getSharedResults()->get_result(contestInput[idx])
                                   : calcCollatz(contestInput[idx]));
                }
            },
            processed, processed + n_tasks
        )));
        processed += n_tasks;
    }

    std::for_each(threads.begin(), threads.end(),
                    [](std::thread& thread) {thread.join();});

    return result;
}

ContestResult TeamPool::runContest(ContestInput const & contestInput)
{
    rtimers::cxx11::DefaultTimer timer("CalcCollatzPoolTimer");

    ContestResult result;
    result.resize(contestInput.size());
    uint64_t n_pools = contestInput.size() / this->getSize() + (contestInput.size() % this->getSize() ? 1 : 0);

    for (uint64_t i = 0; i < n_pools; i++) {
        std::vector<std::future<void>> futures;
        uint64_t limit = std::min((i + 1) * this->getSize(), contestInput.size());
        
        for (uint64_t idx = i * this->getSize(); idx < limit; idx++)
            futures.push_back(std::move(this->pool.push(
                [&result, idx=idx, &timer, this](const InfInt & singleInput){
                    auto scopedStartStop = timer.scopedStart();
                    result[idx] = (this->getSharedResults() 
                                   ? this->getSharedResults()->get_result(singleInput)
                                   : calcCollatz(singleInput));
                },
                contestInput[idx]
            )));
        
        cxxpool::get(futures.begin(), futures.end());
    }

    return result;
}

ContestResult TeamNewProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult result;
    result.resize(contestInput.size());
    int desc;
    const char * file_name = "/tmp/results";
    if (mkfifo(file_name, 0755) == -1) syserr("Error in mkfifo\n");
    uint64_t n_pools = contestInput.size() / this->getSize() + (contestInput.size() % this->getSize() ? 1 : 0);
    
    for (uint64_t i = 0; i < n_pools; i++) {
        uint64_t limit = std::min((i + 1) * this->getSize(), contestInput.size());
        const char * input, * index;
        
        for (uint64_t idx = i * this->getSize(); idx < limit; idx++) {
            switch (fork()) {                     
                case -1: 
                    syserr("Error in fork\n");
                
                case 0:
                    index = std::to_string(idx).c_str();
                    execlp("./new_process", "./new_process", "0", file_name, index, contestInput[idx].toString().c_str(), NULL); 
                    syserr("Error in execlp\n");

                default:                            
                    break;
            }
        }
        
        std::pair<uint64_t, uint64_t> single_result;
        if((desc = open(file_name, O_RDONLY)) == -1) syserr("Parent: Error in open\n");
        uint64_t results_to_read = limit - i * this->getSize();
        do {
            wait(0);
            if (read(desc, &single_result, sizeof(std::pair<uint64_t, uint64_t>)) == -1) 
                syserr("Error in read\n");
            else {
                result[single_result.first] = single_result.second;
                results_to_read--;
            }
        } while (results_to_read);

        if(close(desc)) syserr("Parent, error in close\n");
    }

    if(unlink(file_name)) syserr("Error in unlink:");

    return result;
}

ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput)
{
    ContestResult result;
    result.resize(contestInput.size());
    int desc_inputs, desc_results;
    const char * inputs_file = "/tmp/inputs";    
    const char * results_file = "/tmp/results";
    if (mkfifo(results_file, 0755) == -1) syserr("Error in mkfifo\n");
    if (mkfifo(inputs_file, 0755) == -1) syserr("Error in mkfifo\n");    
    uint64_t min_n_tasks = contestInput.size() / this->getSize();
    uint64_t remainder = contestInput.size() % this->getSize();

    for (uint64_t i = 0, processed = 0; i < this->getSize(); i++) {
        uint64_t n_tasks = min_n_tasks + (i < remainder ? 1 : 0);
        const char * n_tasks_str =  std::to_string(n_tasks).c_str();
        switch (fork()) {                     
            case -1: 
                syserr("Error in fork\n");
            
            case 0:
                execl("./new_process", "./new_process", "1", results_file, inputs_file, n_tasks_str, NULL);
                syserr("Error in execv\n");

            default:                            
                break;
        }
        processed += n_tasks;
    }

    if((desc_inputs = open(inputs_file, O_WRONLY)) == -1) syserr("Child: Error in open\n");
    for (uint64_t idx = 0; idx < contestInput.size(); idx++) {
        std::pair<uint64_t, InfInt> pair = {idx, contestInput[idx]};
        if (write(desc_inputs, &pair, sizeof(std::pair<uint64_t, InfInt>)) < 0)
            syserr("Error in write\n");
    }
    if(close(desc_inputs)) syserr("Parent, error in close\n");


    if((desc_results = open(results_file, O_RDONLY)) == -1) syserr("Parent: Error in open\n");
    uint64_t results_to_read = contestInput.size();
    std::pair<uint64_t, uint64_t> single_result;
    do {
        if (read(desc_results, &single_result, sizeof(std::pair<uint64_t, uint64_t>)) == -1)
            syserr("Error in read\n");
        else {
            result[single_result.first] = single_result.second;
            results_to_read--;
        }
    } while (results_to_read);
    while(wait(0) != -1);

    if(close(desc_results)) syserr("Parent, error in close\n");

    if(unlink(results_file)) syserr("Error in unlink:");
    if(unlink(inputs_file)) syserr("Error in unlink:");

    return result;
}

ContestResult TeamAsync::runContest(ContestInput const & contestInput)
{
    rtimers::cxx11::DefaultTimer timer("CalcCollatzAsyncTimer");

    ContestResult result;
    result.resize(contestInput.size());
    std::vector<std::future<void>> futures;

    for (uint64_t idx = 0; idx < contestInput.size(); idx++)
        futures.push_back(std::async(
            [&result, idx=idx, &timer, this](const InfInt & singleInput){
                auto scopedStartStop = timer.scopedStart();
                result[idx] = (this->getSharedResults() 
                               ? this->getSharedResults()->get_result(singleInput)
                               : calcCollatz(singleInput));
            },
            contestInput[idx]
        ));
    
    std::for_each(futures.begin(), futures.end(),
                    [](std::future<void> & future){future.get();});
    
    return result;
}
