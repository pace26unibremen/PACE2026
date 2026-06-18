//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    unsigned bLabel) :
        AbstractRule(instance, context, false), bLabel(bLabel)
{}

solver::RuleReturnCode solver::ThreeTwoChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    for (const auto& forest : *instance)
    {
        auto bNode = forest->LabelToTerminal().at(bLabel);
        if (not bNode->parent) continue;
        changes.emplace(bNode, forest);
        changes.top().doAction();
    }

    return solver::RuleReturnCode::Continue;

}

void solver::ThreeTwoChainReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}

graph::Node* solver::ThreeTwoChainReductionRule::isA3Path(const graph::Node* aNode, const graph::Node* cNode)
{
    if (aNode->parent && aNode->parent->parent && aNode->parent->parent == cNode->parent)
    {
        return aNode->sibling;
    }
    else if (cNode->parent && cNode->parent->parent && cNode->parent->parent == aNode->parent)
    {
        return cNode->sibling;
    }
    else return nullptr;

}

std::shared_ptr<solver::AbstractRule>
solver::ThreeTwoChainReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                 const std::shared_ptr<Context>& context)
{
    unsigned b = 0;

    unsigned a = 0;
    unsigned c = 0;

    auto f = instance->at(0);
    for (const auto& [label, node] : f->LabelToTerminal())
    {
        if (node->sibling != nullptr and f->TerminalToLabel().contains(node->sibling))
        {
            a = label;
            c = f->TerminalToLabel().at(node->sibling);
            break;
        }
    }

    if (a == 0)
    {
        // At this point if true then we only have single vertex trees
        return nullptr;
    }

    for (unsigned int i = 1; i < instance->size(); i++)
    {
        auto fi = instance->at(i);
        auto aNode = fi->LabelToTerminal()[a];
        auto cNode = fi->LabelToTerminal()[c];

        if (aNode->sibling == cNode) continue;

        auto bNode = isA3Path(aNode, cNode);

        // PairPathBranchingRule
        if (not bNode) return nullptr;

        if (fi->TerminalToLabel().contains(bNode))
        {
            auto biLabel = fi->TerminalToLabel().at(bNode);

            if (b == 0)
            {
                b = biLabel;
            }
            else if (b == biLabel)
            {
                continue;
            }
            else // b != bLabel
            {
                return nullptr;
            }
        }
        else
        {
            // bNode is not a terminal
            return nullptr;
        }
    }

    if (b == 0)
    {
        //PairEqualRule
        return nullptr;
    }
    return std::make_shared<ThreeTwoChainReductionRule>(instance, context, b);
}

std::string solver::ThreeTwoChainReductionRule::name() const
{
    return "ThreeTwoChainReductionRule";
}
