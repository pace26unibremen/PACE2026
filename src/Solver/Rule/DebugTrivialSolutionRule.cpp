#include "DebugTrivialSolutionRule.hpp"
#include "../TrivialSolver.hpp"

solver::DebugTrivialSolutionRule::DebugTrivialSolutionRule(const std::shared_ptr<graph::Instance>& instance)
{
    this->instance = instance;
}

void solver::DebugTrivialSolutionRule::apply()
{
    if(this->isApplied)
    {
        throw std::invalid_argument("DebugTrivialSolutionRule : apply : rule was already applied");
    }
    isApplied = true;

    instanceBackUp = *instance;

    auto s = TrivialSolver(*instance.get());
    auto solution = s.solve();
    instance->at(0) = std::make_shared<graph::Forest>(solution);
    instance->erase(instance->begin() + 1, instance->end());
}

void solver::DebugTrivialSolutionRule::unapply()
{
    if(not this->isApplied)
    {
        throw std::invalid_argument("DebugTrivialSolutionRule : unapply : rule is not applied");
    }
    isApplied = false;

    *instance = instanceBackUp;
}

std::shared_ptr<solver::AbstractRule> solver::DebugTrivialSolutionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<DebugTrivialSolutionRule>(instance));
}