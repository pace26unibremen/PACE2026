#include "PairEqualRule.hpp"

#include <cassert>

solver::PairEqualRule::PairEqualRule(const std::shared_ptr<graph::Instance>& instance,
                                     const std::unordered_map<std::shared_ptr<graph::Forest>, int>& forestToSubtree) :
        forestToSubtree(forestToSubtree)
{
    this->instance = instance;
    this->changes = std::stack<solver::CollapseSubtreeAction>();
}


void solver::PairEqualRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("PairEqualRule : apply : rule is not applied");
    }
    isApplied = true;

    for(const auto& [f,subtree] : forestToSubtree)
    {
        changes.emplace(subtree, f);
        changes.top().doAction();
    }
}

void solver::PairEqualRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("PairEqualRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}


std::shared_ptr<solver::AbstractRule>
solver::PairEqualRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    auto forestToSubtree = std::unordered_map<std::shared_ptr<graph::Forest>, int>();

    unsigned int label1 = 0;
    unsigned int label2 = 0;

    auto f = instance->at(0);
    for (const auto& [label, index] : f->LabelToTerminalIndex())
    {
        const auto& t = f->Nodes()[index];
        if (t.siblingIndex != -1 and f->Terminals().contains(t.siblingIndex))
        {
            label1 = label;
            label2 = f->Nodes()[t.siblingIndex].smallestTerminal();
            forestToSubtree.emplace(f, t.parentIndex);
            break;
        }
    }

    if (label1 == 0)
    {
        // we have a better rule for this case
        return nullptr;
    }

    for (unsigned int i = 1; i < instance->size(); i++)
    {
        auto fi = instance->at(i);
        auto t1Index = fi->LabelToTerminalIndex()[label1];
        auto t2Index = fi->LabelToTerminalIndex()[label2];
        const auto& t1 = fi->Nodes()[t1Index];
        const auto& t2 = fi->Nodes()[t2Index];
        if ((t1.parentIndex == -1) or (t1.parentIndex != t2.parentIndex))
        {
            // we can not apply this rule for label1, label2
            return nullptr;
        }
        forestToSubtree.emplace(fi, t1.parentIndex);
    }

    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<PairEqualRule>(instance, forestToSubtree));
}
