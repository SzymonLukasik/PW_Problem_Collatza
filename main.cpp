#include <assert.h>
#include <iostream>

#include "lib/infint/InfInt.h"
#include "lib/rtimers/cxx11.hpp"

#include "generators.hpp"
#include "teams.hpp"
#include "contest.hpp"

void print_vec(auto &vec, int n = 5) {
    for (int i = 0; i < n; i++)
        std::cerr << vec[i] << ' ';
    std::cerr << '\n';
}

int main(int argc, char ** argv)
{
    rtimers::cxx11::DefaultTimer totalTimer("Total");
    auto totalStartStop = totalTimer.scopedStart();

    std::vector<std::shared_ptr<ContestGenerator>> generators = {
        std::shared_ptr<ContestGenerator>(new SameNumberContestGenerator{}),
        std::shared_ptr<ContestGenerator>(new ShortNumberContestGenerator{}),
        std::shared_ptr<ContestGenerator>(new LongNumberContestGenerator{}),
    };

    std::vector<std::shared_ptr<Team>> teams;
    teams.push_back(std::shared_ptr<Team>(new TeamSolo{1}));
    for (bool share : {false})//, true})
    {
        for (uint32_t numWorkers : {1,2,3,4,7,10})
        {
            // teams.push_back(std::shared_ptr<Team>(new TeamNewThreads{numWorkers, share}));
            // teams.push_back(std::shared_ptr<Team>(new TeamConstThreads{numWorkers, share}));
            //teams.push_back(std::shared_ptr<Team>(new TeamPool{numWorkers, share}));  
            teams.push_back(std::shared_ptr<Team>(new TeamNewProcesses{numWorkers, share}));
            // teams.push_back(std::shared_ptr<Team>(new TeamConstProcesses{numWorkers, share}));
        }
        // teams.push_back(std::shared_ptr<Team>(new TeamAsync{1, share}));
    }

    for (auto generator : generators)
    {
        for (uint32_t contestId : {2, 5, 23})
        {
            std::shared_ptr<ContestResult> expectedResult;
            for (auto team : teams)
            {
                std::string contestName = generator->getContestName(contestId);
                ContestResult lastResult;
                rtimers::cxx11::DefaultTimer timer(team->getTeamName() + contestName);
                {
                    auto scopedStartStop = timer.scopedStart();
                    lastResult = team->runContest(generator->getContest(contestId));
                }
                if (expectedResult)
                {
                    // std::cerr << "EXPECTED: ";
                    // print_vec(*expectedResult, 10);
                    // std::cerr << "LAST: ";
                    // print_vec(lastResult, 10);
                    assert(*expectedResult == lastResult);
                }
                else
                {
                    expectedResult.reset(new ContestResult{});
                    *expectedResult = lastResult;
                }
            }
        }
    }

    return 0;
}