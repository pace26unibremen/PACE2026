#include "EqualForestsRule.hpp"

#include <algorithm>
#include <unordered_set>

solver::EqualForestsRule::EqualForestsRule(const std::shared_ptr<graph::Instance>& instance,
                                           const std::unordered_set<std::shared_ptr<graph::Forest>>& toBeRemoved) :
        toBeRemoved(toBeRemoved)
{
    this->instance = instance;
}

void solver::EqualForestsRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("EqualForestsRule : apply : rule was already applied");
    }
    isApplied = true;
    instanceBackUp = *instance;
    instance->erase(std::remove_if(instance->begin(), instance->end(),
                                   [&](const std::shared_ptr<graph::Forest>& f) { return toBeRemoved.contains(f); }),
                    instance->end());
}

void solver::EqualForestsRule::unapply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("EqualForestsRule : unapply : rule is not applied");
    }
    isApplied = false;
    *instance = instanceBackUp;
}


std::shared_ptr<solver::AbstractRule>
solver::EqualForestsRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    auto toBeRemoved = std::unordered_set<std::shared_ptr<graph::Forest>>();
    for (unsigned int i = 0; i < instance->size(); i++)
    {
        if (toBeRemoved.contains(instance->at(i)))
        {
            continue;
        }
        for (unsigned int j = i; j < instance->size(); j++)
        {
            if (instance->at(i) == instance->at(j))
            {
                toBeRemoved.insert(instance->at(j));
            }
        }
    }

    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<EqualForestsRule>(instance, toBeRemoved));
}
