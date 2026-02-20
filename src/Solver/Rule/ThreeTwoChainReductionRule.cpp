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
    if (this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    return 0;
}

void solver::ThreeTwoChainReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;



}

std::shared_ptr<solver::AbstractRule>
solver::ThreeTwoChainReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                 const std::shared_ptr<Context>& context)
{
    std::vector<std::vector<graph::Node*>> chainT1;
    std::vector<std::vector<graph::Node*>> chainT2;
    std::vector<std::shared_ptr<graph::Forest>> Trees;
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainInTree;

    for (const auto& T1 : *instance)
    {
        //Let (x1, x2, x3) be a pendant 3-chain
        //of T. If (xi , x3) is a pendant 2-chain in T' with xi ∈ {x1, x2},
        //then set S = T |X \ {x j} and S' = T'|X \ {x j} with {xi , x j} =
        //{x1, x2}
        //Pendant -> Parent of x1 and x2 is the same reference -> Solely Case 1 of ChainReductionRule


        for (const auto& T2 : *instance)
        {

            if (T1 != T2)
            {
                //T1: T in def
                for (const auto& terminalT1 : T1->Terminals())
                {
                    //Find a 3-Pendant chain

                    //T2: T' in def
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
