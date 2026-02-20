//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    const std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>>& chainInTree) :
        AbstractRule(instance, context)
{
    this->chainInTree = chainInTree;
    changes = std::stack<solver::AbstractAction>{};
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
    std::vector<std::vector<graph::Node*>> chain;
    std::vector<std::shared_ptr<graph::Forest>> Trees;
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainInTree;

    for (const auto& T1 : *instance)
    {
        for (const auto& T2 : *instance)
        {

            if (T1 != T2)
            {
                //T1
                for (const auto& terminalT1 : T1->Terminals())
                {
                    //T2
                    for(const auto& terminalT2 : T2->Terminals())
                    {

                    }
                }
            }
        }
    }




   return std::make_shared<ThreeTwoChainReductionRule>(instance, context,chainInTree);
}

std::string solver::ThreeTwoChainReductionRule::name() const
{
    return "ThreeTwoChainReductionRule";
}
