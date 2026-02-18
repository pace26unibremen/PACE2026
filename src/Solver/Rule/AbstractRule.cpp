#include "AbstractRule.hpp"

solver::AbstractRule::AbstractRule(const std::shared_ptr<graph::Instance>& instance,
                                   const std::shared_ptr<Context>& context) :
    instance(instance),
    context(context)
{}

bool solver::AbstractRule::IsApplied() const
{
    return this->isApplied;
}

std::shared_ptr<std::list<std::shared_ptr<solver::AbstractRule>>> solver::AbstractRule::NextRuleSuggestion()
{
    return nullptr;
}
