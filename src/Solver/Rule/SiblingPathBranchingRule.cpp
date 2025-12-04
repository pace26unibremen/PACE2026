#include "SiblingPathBranchingRule.hpp"

#include <cassert>
#include <list>

// 1. label1
// 2. label2
// 3. map shared_ptr forest to the list of the cut-away subtrees on the path
typedef std::tuple<unsigned int, unsigned int, std::unordered_map<std::shared_ptr<graph::Forest>, std::list<int>>>
    context;

solver::SiblingPathBranchingRule::SiblingPathBranchingRule(const std::shared_ptr<graph::Instance>& instance,
                                                           const context& context) :
        label1(get<0>(context)),
        label2(get<1>(context)),
        forestToPathDeletions(get<2>(context))
{
    this->instance = instance;
    this->changes = std::stack<solver::DeleteEdgeAction>();
}


void solver::SiblingPathBranchingRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("SiblingPathBranchingRule : apply : rule is not applied");
    }
    if (this->branch >= 3)
    {
        throw std::invalid_argument("SiblingPathBranchingRule : apply : all branches are already visited");
    }
    isApplied = true;
    branch++;

    switch (branch)
    {
        case 1:
            for(const auto& f : *instance)
            {
                auto t1Index = f->LabelToTerminalIndex()[label1];
                const auto& t1 = f->Nodes()[t1Index];
                if(t1.parentIndex != -1)
                {
                    changes.emplace(DeleteEdgeAction(t1Index,f));
                    changes.top().doAction();
                }
            }
            break;
        case 2:
            for(const auto& f : *instance)
            {
                auto t2Index = f->LabelToTerminalIndex()[label2];
                const auto& t2 = f->Nodes()[t2Index];
                if(t2.parentIndex != -1)
                {
                    changes.emplace(DeleteEdgeAction(t2Index,f));
                    changes.top().doAction();
                }
            }
            break;
        case 3:
            for(const auto& f : *instance)
            {
                for(auto pathDel : this->forestToPathDeletions[f])
                {
                    changes.emplace(DeleteEdgeAction(pathDel,f));
                    changes.top().doAction();
                }
            }
            break;
        default:
            assert(false);
    }
}

void solver::SiblingPathBranchingRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("SiblingPathBranchingRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}


std::shared_ptr<solver::AbstractRule>
solver::SiblingPathBranchingRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
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
            get<2>(c).emplace(f, std::list<int>());
            break;
        }
    }

    if (get<0>(c) == 0)
    {
        // we have a better rule for this case
        return nullptr;
    }

    // if there is a path in at least on instance, where something can be deleted
    bool existNonTrivialPath = false;

    for (unsigned int i = 1; i < instance->size(); i++)
    {
        auto fi = instance->at(i);
        auto t1Index = fi->LabelToTerminalIndex()[get<0>(c)];
        auto t2Index = fi->LabelToTerminalIndex()[get<1>(c)];
        auto rootIndex = fi->rootIndexOf(t1Index);
        if (not fi->Nodes()[t2Index].hasSubsetTerminals(fi->Nodes()[rootIndex]))
        {
            // we have a better rule for this case
            return nullptr;
        }

        // moving upwards from t1 to lca - and collect all cut-away subtrees
        auto cutAwaySubtrees = std::list<int>();
        int lcaIndex = 0;
        const auto& t2 = fi->Nodes()[t2Index];
        int it = t1Index;
        while (true)
        {
            const auto& itNode = fi->Nodes()[it];
            assert(itNode.parentIndex != -1);
            if (t2.hasSubsetTerminals(fi->Nodes()[itNode.parentIndex]))
            {
                lcaIndex = itNode.parentIndex;
                break;
            }
            cutAwaySubtrees.push_back(itNode.siblingIndex);
            it = itNode.parentIndex;
        }

        // moving upwards from t2 to lca - and collect all cut-away subtrees
        it = t2Index;
        while (true)
        {
            const auto& itNode = fi->Nodes()[it];
            assert(itNode.parentIndex != -1);
            if (itNode.parentIndex == lcaIndex)
            {
                break;
            }
            cutAwaySubtrees.push_back(itNode.siblingIndex);
            it = itNode.parentIndex;
        }

        if (cutAwaySubtrees.size() >= 1)
        {
            // actually the case == 1 result in another, better branching rule
            existNonTrivialPath = true;
        }
        get<2>(c).emplace(fi, std::move(cutAwaySubtrees));
    }

    if (not existNonTrivialPath)
    {
        return nullptr;
    }
    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<SiblingPathBranchingRule>(instance, c));
}

bool solver::SiblingPathBranchingRule::isFullyExplored() const
{
    return branch >= 3;
}
