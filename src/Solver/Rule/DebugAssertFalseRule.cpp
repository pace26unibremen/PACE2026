#include "DebugAssertFalseRule.hpp"

solver::DebugAssertFalseRule::DebugAssertFalseRule(const std::shared_ptr<graph::Instance>& instance)
{
    this->instance = instance;
}

void solver::DebugAssertFalseRule::apply()
{
    graph::WriteInstance(instance, pathToWriteInstance);
    throw std::runtime_error("DebugAssertFalseRule : apply : rule triggered");
}

void solver::DebugAssertFalseRule::unapply() {}

std::shared_ptr<solver::AbstractRule> solver::DebugAssertFalseRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<DebugAssertFalseRule>(instance));
}