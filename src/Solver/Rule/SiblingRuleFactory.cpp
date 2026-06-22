#include "SiblingRuleFactory.hpp"

#include "BRule.hpp"
#include "ReverseBRule.hpp"
#include "TwoBRule.hpp"
#include "ACBranchingRule.hpp"
#include "ABCBranchingRule.hpp"
#include "EqualPairReductionRule.hpp"

std::pair<unsigned int, unsigned int> solver::SiblingRuleFactory::getSiblings(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<solver::Context>& context)
{
    const auto& f = instance->at(0);
    for (const auto& label : context->heuristicLabelOrder)
    {
        if (not f->LabelToTerminal().contains(label))
        {
            continue;
        }
        const auto& node = f->LabelToTerminal().at(label);

        if (node->sibling != nullptr and f->TerminalToLabel().contains(node->sibling))
        {
            return {f->TerminalToLabel().at(node), f->TerminalToLabel().at(node->sibling)};
        }
    }
    return {0,0};
}

std::shared_ptr<solver::AbstractRule> solver::SiblingRuleFactory::basicRules(const std::shared_ptr<graph::Instance>& instance,
                                                                             const std::shared_ptr<solver::Context>& context)
{
    const auto& [aLabel, cLabel] = getSiblings(instance, context);
    auto forestWithBNodes = std::list<std::pair<std::shared_ptr<graph::Forest>, std::list<graph::Node*>>>();

    if (aLabel * cLabel == 0)
    {
        // there is no sibling pair in f0
        return nullptr;
    }

    // if the siblings from f0 are siblings in every other forest.
    bool siblings = true;

    for (unsigned int i = 1; i < instance->size(); i++)
    {
        const auto& fi = instance->at(i);
        const auto& aNode = fi->LabelToTerminal().at(aLabel);
        const auto& cNode = fi->LabelToTerminal().at(cLabel);

        if (aNode->sibling == cNode)
        {
            continue;
        }
        siblings = false;

        const auto& aRoot = fi->rootOf(aNode);
        if (not cNode->hasSubsetTerminals(aRoot))
        {
            return std::make_shared<ACBranchingRule>(instance, context, aLabel, cLabel);
        }

        // moving upwards from aNode to lca - and collect b-nodes
        auto bNodes = std::list<graph::Node*>();
        graph::Node* lca;
        graph::Node* it = aNode;
        while (true)
        {
            if (cNode->hasSubsetTerminals(it->parent))
            {
                lca = it->parent;
                break;
            }
            bNodes.push_back(it->sibling);
            it = it->parent;
        }

        // moving upwards from cNode to lca - and collect b-nodes
        it = cNode;
        while (true)
        {
            if (it->parent == lca)
            {
                break;
            }
            bNodes.push_back(it->sibling);
            it = it->parent;
        }

        if (not bNodes.empty())
        {
            forestWithBNodes.emplace_back(fi, bNodes);
        }
    }

    if (siblings)
    {
        return std::make_shared<EqualPairReductionRule>(instance, context, aLabel, cLabel);
    }

    return std::make_shared<ABCBranchingRule>(instance, context, aLabel, cLabel, forestWithBNodes);
}
