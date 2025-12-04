
#include "SingleVertexTreePropagationRule.hpp"

solver::SingleVertexTreePropagationRule::SingleVertexTreePropagationRule(
    const std::shared_ptr<graph::Instance>& instance, const std::unordered_set<unsigned int>& labelsToBeReduced) :
        labelsToBeReduced(labelsToBeReduced)
{
    this->instance = instance;
    changes = std::stack<DeleteEdgeAction>();
}

void solver::SingleVertexTreePropagationRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("SingleVertexTreePropagationRule : apply : rule was already applied");
    }
    isApplied = true;

    for (const auto& f_ptr : *instance)
    {
        for (unsigned int label : labelsToBeReduced)
        {
            int terminalIndex = f_ptr->LabelToTerminalIndex()[label];
            const graph::Node& terminal = f_ptr->Nodes()[terminalIndex];
            if (terminal.parentIndex != -1)
            {
                changes.emplace(terminalIndex, f_ptr);
                changes.top().doAction();
            }
        }
    }
}

void solver::SingleVertexTreePropagationRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("EqualForestsRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}

std::shared_ptr<solver::AbstractRule>
solver::SingleVertexTreePropagationRule::isApplicable(const std::shared_ptr<graph::Instance>& instance)
{
    auto labelsToBeReduced = std::unordered_set<unsigned int>();

    for (unsigned int label = 1; label < instance->at(0)->LabelToTerminalIndex().size() + 1; label++)
    {
        bool anySingleVertexTree = false;
        bool anyNotSingleVertexTree = false;

        for (const auto& f_ptr : *instance)
        {
            const int& terminalIndex = f_ptr->LabelToTerminalIndex()[label];
            const graph::Node& terminal = f_ptr->Nodes()[terminalIndex];
            if (terminal.parentIndex != -1)
            {
                anyNotSingleVertexTree = true;
            }
            else
            {
                anySingleVertexTree = true;
            }
        }
        if (anySingleVertexTree and anyNotSingleVertexTree)
        {
            labelsToBeReduced.insert(label);
        }
    }

    if(labelsToBeReduced.empty())
    {
        return nullptr;
    }
    return std::dynamic_pointer_cast<AbstractRule>(
        std::make_shared<SingleVertexTreePropagationRule>(instance, labelsToBeReduced));
}
