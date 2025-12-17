#include "../TrivialSolver.hpp"
#include "DebugAssertFalseRule.hpp"

#include <cassert>

solver::DebugAssertFalseRule::DebugAssertFalseRule(const std::shared_ptr<graph::Instance>& instance)
{
    this->instance = instance;
}

void solver::DebugAssertFalseRule::apply()
{
    if(this->isApplied)
    {
        throw std::invalid_argument("DebugAssertFalseRule : apply : rule was already applied");
    }
    isApplied = true;

    graph::WriteInstance(instance,pathToWriteInstance);
    assert(false);
}

void solver::DebugAssertFalseRule::unapply()
{
    if(not this->isApplied)
    {
        throw std::invalid_argument("DebugAssertFalseRule : unapply : rule is not applied");
    }
    isApplied = false;
}

std::shared_ptr<solver::AbstractRule> solver::DebugAssertFalseRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<DebugAssertFalseRule>(instance));
}