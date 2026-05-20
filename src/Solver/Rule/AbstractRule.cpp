#include "AbstractRule.hpp"

solver::AbstractRule::AbstractRule(const std::shared_ptr<graph::Instance>& instance,
                                   const std::shared_ptr<Context>& context,
                                   bool isReduction) :
    instance(instance),
    context(context),
    isReduction(isReduction)
{}

bool solver::AbstractRule::IsApplied() const
{
    return this->isApplied;
}

bool solver::AbstractRule::IsReduction() const
{
    return this->isReduction;
}

std::shared_ptr<std::list<std::shared_ptr<solver::AbstractRule>>> solver::AbstractRule::NextRuleSuggestion()
{
    return nullptr;
}
