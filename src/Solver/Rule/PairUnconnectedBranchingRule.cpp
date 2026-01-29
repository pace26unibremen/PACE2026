#include "PairUnconnectedBranchingRule.hpp"

#include <cassert>

// 1. label1
// 2. label2
// 3. list of forests that are affected
typedef std::tuple<
    unsigned int,
    unsigned int,
    std::list<std::shared_ptr<graph::Forest>>>
    context;

solver::PairUnconnectedBranchingRule::PairUnconnectedBranchingRule(const std::shared_ptr<graph::Instance>& instance,
                                                           const context& context) :
        label1(get<0>(context)),
        label2(get<1>(context)),
        forestsConnectedLabels(get<2>(context))
{
    this->instance = instance;
    this->changes = std::stack<solver::DeleteEdgeAction>();
}


void solver::PairUnconnectedBranchingRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("PairUnconnectedBranchingRule : apply : rule is not applied");
    }
    if (this->branch >= 2)
    {
        throw std::invalid_argument("PairUnconnectedBranchingRule : apply : all branches are already visited");
    }
    isApplied = true;
    branch++;

    switch (branch)
    {
        case 1:
            for(const auto& f : forestsConnectedLabels)
            {
                auto t1Index = f->LabelToTerminalIndex()[label1];
                changes.emplace(t1Index, f);
                changes.top().doAction();
            }
            break;
        case 2:
            for(const auto& f : forestsConnectedLabels)
            {
                auto t2Index = f->LabelToTerminalIndex()[label2];
                changes.emplace(t2Index, f);
                changes.top().doAction();
            }
            break;
        default:
            assert(false);
    }
}

void solver::PairUnconnectedBranchingRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("PairUnconnectedBranchingRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}


std::shared_ptr<solver::AbstractRule>
solver::PairUnconnectedBranchingRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    context c = context();
    get<0>(c) = 0;
    get<1>(c) = 0;

    auto f = instance->at(0);
    for (const auto& [label, index] : f->LabelToTerminalIndex())
    {
        const auto& t = f->Nodes()[index];
        if (t.siblingIndex != -1 and f->Terminals().contains(t.siblingIndex))
        {
            get<0>(c) = label;
            get<1>(c) = f->Nodes()[t.siblingIndex].smallestTerminal();
            get<2>(c).push_back(f);
            break;
        }
    }

    if (get<0>(c) == 0)
    {
        // we have a better rule for this case
        return nullptr;
    }

    bool existsUnconnectedPair = false;
    for (unsigned int i = 1; i < instance->size(); i++)
    {
        auto fi = instance->at(i);
        auto t1Index = fi->LabelToTerminalIndex()[get<0>(c)];
        auto t2Index = fi->LabelToTerminalIndex()[get<1>(c)];
        auto rootIndex = fi->rootIndexOf(t1Index);
        if (not fi->Nodes()[t2Index].hasSubsetTerminals(fi->Nodes()[rootIndex]))
        {
            existsUnconnectedPair = true;
        }
        else
        {
            get<2>(c).push_back(fi);
        }
    }

    if (not existsUnconnectedPair)
    {
        return nullptr;
    }

    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<PairUnconnectedBranchingRule>(instance, c));
}

std::string solver::PairUnconnectedBranchingRule::name() const
{
    return "PairUnconnectedBranchingRule";
}

bool solver::PairUnconnectedBranchingRule::isFullyExplored() const
{
    return branch >= 2;
}