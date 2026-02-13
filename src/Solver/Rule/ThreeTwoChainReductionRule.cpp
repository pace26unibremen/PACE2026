//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context):
                        AbstractRule(instance,context)
{

}

int solver::ThreeTwoChainReductionRule::apply()
{

    return 0;
}

void solver::ThreeTwoChainReductionRule::unapply()
{

}

std::shared_ptr<solver::AbstractRule>
solver::ThreeTwoChainReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                 const std::shared_ptr<Context>& context)
{
    for (const auto& T1 : *instance)
    {
        for (const auto& T2 : *instance)
        {
            if (T1 != T2)
            {
                //T1
                for (const auto& terminalT1 : T1->Terminals())
                {

                }
            }
        }
    }




   // return std::make_shared<ThreeTwoChainReductionRule>(instance, context);
}

std::string solver::ThreeTwoChainReductionRule::name() const
{
    return "ThreeTwoChainReductionRule";
}
