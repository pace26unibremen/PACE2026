#include "DebugAssertFalseRule.hpp"

solver::DebugAssertFalseRule::DebugAssertFalseRule(const std::shared_ptr<graph::Instance>& instance,
                                                   const std::shared_ptr<Context>& context)
{
    this->instance = instance;
    this->context = context;
}

void solver::DebugAssertFalseRule::apply()
{
    graph::WriteInstance(instance, pathToWriteInstance);
    throw std::runtime_error("DebugAssertFalseRule : apply : rule triggered");
}

void solver::DebugAssertFalseRule::unapply() {}

std::shared_ptr<solver::AbstractRule>
solver::DebugAssertFalseRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                           const std::shared_ptr<Context>& context)
{
    return std::make_shared<DebugAssertFalseRule>(instance, context);
}

std::string solver::DebugAssertFalseRule::name() const
{
    return "DebugAssertFalseRule";
}