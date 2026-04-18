#include "AbstractBranchingRule.hpp"

namespace solver
{

AbstractBranchingRule::AbstractBranchingRule(const std::shared_ptr<graph::Instance>& _instance,
                                             const std::shared_ptr<Context>& _context,
                                             const int _maxBranch) :
    AbstractRule(_instance,_context, false),
    maxBranch(_maxBranch)
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
