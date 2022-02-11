#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <string>

#include "lib/infint/InfInt.h"

#include "contest.hpp"

class ContestGenerator
{
public:
    virtual ~ContestGenerator() {}

    virtual ContestInput getContest(int32_t id) = 0;
    virtual std::string getGeneratorName() = 0;

    virtual std::string getContestName(uint32_t contestId)
    {
        return "[" + this->getGeneratorName() + " | " + std::to_string(contestId) + "]";
    }
};

class LongNumberContestGenerator : public ContestGenerator
{
public:
    virtual ContestInput getContest(int32_t id)
    {
        ContestInput result;
        for (int j = id + 1; j <= (id + 1) * 2; ++j)
        {
            std::stringstream ss("");
    
            for (int i = 1; i <= j; ++i)
            {
                ss << i;
            }
            result.push_back(InfInt(ss.str()));
        }
        return result;
    }

    virtual std::string getGeneratorName() { return "LongNumber"; }
};

class ShortNumberContestGenerator : public ContestGenerator
{
public:
    virtual ContestInput getContest(int32_t id)
    {
        ContestInput result;
        for (int j = id + 1; j <= (id + 1) * 100; ++j)
        {
            result.push_back(InfInt(j));
        }
        return result;
    }

    virtual std::string getGeneratorName() { return "ShortNumber"; }
};

class SameNumberContestGenerator : public ContestGenerator
{
public:
    virtual ContestInput getContest(int32_t id)
    {
        ContestInput result;
        for (int j = id + 1; j <= (id + 1) * 1000; ++j)
        {
            result.push_back(InfInt(id));
        }
        return result;
    }

    virtual std::string getGeneratorName() { return "SameNumber"; }
};

#endif