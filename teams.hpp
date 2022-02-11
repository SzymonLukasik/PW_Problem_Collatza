#ifndef TEAMS_HPP
#define TEAMS_HPP

#include <thread>
#include <assert.h>

#include "lib/rtimers/cxx11.hpp"
#include "lib/pool/cxxpool.h"

#include "contest.hpp"
#include "collatz.hpp"
#include "sharedresults.hpp"

class Team
{
public:
    Team(uint32_t sizeArg, bool shareResults): size(sizeArg), sharedResults() 
    {
        assert(this->size > 0);

        if (shareResults)
        {
            this->sharedResults.reset(new SharedResults{});
        }
    }
    virtual ~Team() {}

    virtual std::string getInnerName() = 0;

    std::shared_ptr<SharedResults> getSharedResults()
    {
        return this->sharedResults;
    }

    virtual ContestResult runContest(ContestInput const & contest) = 0;
    std::string getXname() { return this->getSharedResults() ? "X" : ""; }
    virtual std::string getTeamName() { return this->getInnerName() + this->getXname() + "<" + std::to_string(this->size) + ">"; }
    uint32_t getSize() const { return this->size; }


private:
    std::shared_ptr<SharedResults> sharedResults;
    uint32_t size;
};

class TeamSolo : public Team
{
public:
    TeamSolo(uint32_t sizeArg): Team(1, false) {} // ignore size, don't share

    virtual ContestResult runContest(ContestInput const & contestInput)
    {
        ContestResult result;
        result.resize(contestInput.size());
        uint64_t idx = 0;

        rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzSoloTimer");

        for(InfInt const & singleInput : contestInput)
        {
            auto scopedStartStop = soloTimer.scopedStart();
            result[idx] = calcCollatz(singleInput);
            ++idx;
        }
        return result;
    }

    virtual std::string getInnerName() { return "TeamSolo"; }
};

class TeamThreads : public Team
{
public:
    TeamThreads(uint32_t sizeArg, bool shareResults): Team(sizeArg, shareResults), createdThreads(0) {}
    
    template< class Function, class... Args >
    std::thread createThread(Function&& f, Args&&... args)
    {
        ++this->createdThreads;
        return std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
    }

    void resetThreads() { this->createdThreads = 0; } 
    uint64_t getCreatedThreads() { return this->createdThreads; }

private:
    uint64_t createdThreads;
};

class TeamNewThreads : public TeamThreads
{
public:
    TeamNewThreads(uint32_t sizeArg, bool shareResults): TeamThreads(sizeArg, shareResults) {}

    virtual ContestResult runContest(ContestInput const & contestInput)
    {
        this->resetThreads();
        ContestResult result = this->runContestImpl(contestInput);
        assert(contestInput.size() == this->getCreatedThreads());
        return result;
    }

    virtual ContestResult runContestImpl(ContestInput const & contestInput);

    virtual std::string getInnerName() { return "TeamNewThreads"; }
};

class TeamConstThreads : public TeamThreads
{
public:
    TeamConstThreads(uint32_t sizeArg, bool shareResults): TeamThreads(sizeArg, shareResults) {}

    virtual ContestResult runContest(ContestInput const & contestInput)
    {
        this->resetThreads();
        ContestResult result = this->runContestImpl(contestInput);
        assert(this->getSize() == this->getCreatedThreads());
        return result;
    }

    virtual ContestResult runContestImpl(ContestInput const & contestInput);

    virtual std::string getInnerName() { return "TeamConstThreads"; }
};

class TeamPool : public Team
{
public:
    TeamPool(uint32_t sizeArg, bool shareResults): Team(sizeArg, shareResults), pool(sizeArg) {}

    virtual ContestResult runContest(ContestInput const & contestInput);

    virtual std::string getInnerName() { return "TeamPool"; }
private:
    cxxpool::thread_pool pool;
};

class TeamNewProcesses : public Team
{
public:
    TeamNewProcesses(uint32_t sizeArg, bool shareResults): Team(sizeArg, shareResults) {}

    virtual ContestResult runContest(ContestInput const & contestInput);

    virtual std::string getInnerName() { return "TeamNewProcesses"; }
};

class TeamConstProcesses : public Team
{
public:
    TeamConstProcesses(uint32_t sizeArg, bool shareResults): Team(sizeArg, shareResults) {}

    virtual ContestResult runContest(ContestInput const & contestInput);

    virtual std::string getInnerName() { return "TeamConstProcesses"; }
};

class TeamAsync : public Team
{
public:
    TeamAsync(uint32_t sizeArg, bool shareResults): Team(1, shareResults) {} // ignore size

    virtual ContestResult runContest(ContestInput const & contestInput);

    virtual std::string getInnerName() { return "TeamAsync"; }
};

#endif