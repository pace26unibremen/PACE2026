//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

#include <iostream>

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    const std::pair<graph::Node*,std::vector<std::shared_ptr<graph::Forest>>>& nodeAndTrees) :
        AbstractRule(instance, context, true)
{
    this->nodeAndTrees = nodeAndTrees;
    std::cout << "Terminal.at 1" << std::endl;
    for (const auto& terminal : nodeAndTrees.second.front()->TerminalToLabel())
    {
        if (terminal.first == nodeAndTrees.first)
        {
            this->nodeLabel = terminal.second;
            break;
        }
    }
    //this->nodeLabel = nodeAndTrees.second.front()->Terminals().at(nodeAndTrees.first);
    changes = std::stack<solver::AbstractAction>{};
}

solver::RuleReturnCode solver::ThreeTwoChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

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

    return nullptr;
}

std::string solver::ThreeTwoChainReductionRule::name() const
{
    return "ThreeTwoChainReductionRule";
}
