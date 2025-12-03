#include "DebugTrivialSolutionRule.hpp"
#include "../TrivialSolver.hpp"

solver::DebugTrivialSolutionRule::DebugTrivialSolutionRule() = default;

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

    this->instanceCopy = std::make_shared<graph::Instance>(instance->size());
    for(const auto& f_ptr : *instance)
    {
        instanceCopy->push_back(f_ptr);
    }

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

    instance->erase(instance->begin());
    for(const auto& f_ptr : *instanceCopy)
    {
        instance->push_back(f_ptr);
    }
}

std::shared_ptr<solver::AbstractRule> solver::DebugTrivialSolutionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<DebugTrivialSolutionRule>(instance));
}