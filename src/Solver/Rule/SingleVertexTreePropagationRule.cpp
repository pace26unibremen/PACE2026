#include "SingleVertexTreePropagationRule.hpp"

solver::SingleVertexTreePropagationRule::SingleVertexTreePropagationRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    const std::unordered_set<unsigned int>& labelsToBeReduced) :
        AbstractRule(instance, context, false),
        labelsToBeReduced(labelsToBeReduced)
{}

solver::RuleReturnCode solver::SingleVertexTreePropagationRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("SingleVertexTreePropagationRule : apply : rule is already applied");
    }
    isApplied = true;

    for (const auto& fi : *instance)
    {
        for (unsigned int label : labelsToBeReduced)
        {
            graph::Node* terminal = fi->LabelToTerminal()[label];
            if (terminal->parent != nullptr)
            {
                if (context->protectedEdges.contains(terminal))
                {
                    return RuleReturnCode::CutBranch;
                }
                changes.emplace(terminal, fi, context.get());
                changes.top().doAction();
            }
        }
    }

    return RuleReturnCode::Continue;
}

void solver::SingleVertexTreePropagationRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("SingleVertexTreePropagationRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}

std::shared_ptr<solver::AbstractRule>
solver::SingleVertexTreePropagationRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context)
{
    // Applicability is read from the context's incrementally-maintained
    // single-vertex-tree tracking instead of rescanning the whole instance every
    // solver iteration (see Context::singleVertexMismatchCount). The tracking is
    // kept current by DeleteEdgeAction / CollapseSubtreeAction on do/undo; here we
    // just initialise it once, then the common "nothing to do" case is O(1).
    if (not context->singleVertexTrackingInitialised)
    {
        context->initSingleVertexTracking(instance);
    }

    if (context->singleVertexMismatchCount == 0)
    {
        return nullptr;
    }

    // Materialise the reduction set only now that the rule is known to fire: the
    // labels that are an SVT in some but not all forests.
    auto labelsToBeReduced = std::unordered_set<unsigned int>();
    const unsigned int forests = context->singleVertexNumForests;
    for (unsigned int label = 0; label < context->singleVertexForestCount.size(); ++label)
    {
        const unsigned int count = context->singleVertexForestCount[label];
        if (count > 0 and count < forests)
        {
            labelsToBeReduced.insert(label);
        }
    }
    return std::make_shared<SingleVertexTreePropagationRule>(instance, context, labelsToBeReduced);
}

std::string solver::SingleVertexTreePropagationRule::name() const
{
    return "SingleVertexTreePropagationRule";
}

std::shared_ptr<solver::AbstractRule> solver::SingleVertexTreePropagationRule::clone() const
{
    return std::make_shared<SingleVertexTreePropagationRule>(instance, context, labelsToBeReduced);
}
