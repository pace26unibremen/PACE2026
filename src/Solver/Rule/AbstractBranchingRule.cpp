#include "AbstractBranchingRule.hpp"

namespace solver
{

AbstractBranchingRule::AbstractBranchingRule(int _maxBranch)
    : maxBranch(_maxBranch)
{}

int AbstractBranchingRule::Branch() const
{
    return branch;
}

int AbstractBranchingRule::MaxBranch() const
{
    return maxBranch;
}

bool AbstractBranchingRule::isFullyExplored() const
{
    return branch >= maxBranch;
}

}  //namespace solver
