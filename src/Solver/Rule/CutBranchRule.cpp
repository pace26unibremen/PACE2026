#include "CutBranchRule.hpp"

solver::CutBranchRule::CutBranchRule(const std::shared_ptr<graph::Instance>& instance,
                                     const std::shared_ptr<Context>& context) :
    AbstractRule(instance, context)
{}

int solver::CutBranchRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("CutBranchRule : apply : rule was already applied");
    }
    isApplied = true;
    return 2;
}

void solver::CutBranchRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("CutBranchRule : unapply : rule is not applied");
    }
    isApplied = false;
}

std::shared_ptr<solver::AbstractRule>
solver::CutBranchRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                    const std::shared_ptr<Context>& context)
{
    for (const auto& f : *instance)
    {
        if (f->Roots().size() >=  context->bestSolutionSize)
        {
            return std::make_shared<solver::CutBranchRule>(instance, context);
        }
    }

    return nullptr;
}

std::string solver::CutBranchRule::name() const
{
    return "CutBranchRule";
}
