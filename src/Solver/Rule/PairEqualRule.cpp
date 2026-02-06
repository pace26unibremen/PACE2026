#include "PairEqualRule.hpp"

#include <cassert>

solver::PairEqualRule::PairEqualRule(const std::shared_ptr<graph::Instance>& instance,
                                     const std::unordered_map<std::shared_ptr<graph::Forest>, graph::Node*>& forestToSubtree) :
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
    auto forestToSubtree = std::unordered_map<std::shared_ptr<graph::Forest>, graph::Node*>();

    unsigned int label1 = 0;
    unsigned int label2 = 0;

    auto f = instance->at(0);
    for (const auto& [label, node] : f->LabelToTerminal())
    {
        if (node->sibling != nullptr and f->Terminals().contains(node->sibling))
        {
            label1 = label;
            label2 = node->sibling->smallestTerminal();
            forestToSubtree.emplace(f, node->parent);
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
        auto t1 = fi->LabelToTerminal()[label1];
        auto t2 = fi->LabelToTerminal()[label2];
        if ((t1->parent == nullptr) or (t1->parent != t2->parent))
        {
            // we can not apply this rule for label1, label2
            return nullptr;
        }
        forestToSubtree.emplace(fi, t1->parent);
    }

    return std::dynamic_pointer_cast<AbstractRule>(std::make_shared<PairEqualRule>(instance, forestToSubtree));
}

std::string solver::PairEqualRule::name() const
{
    return "PairEqualRule";
}
