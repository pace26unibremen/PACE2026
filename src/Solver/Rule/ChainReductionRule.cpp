#include "ChainReductionRule.hpp"

solver::ChainReductionRule::ChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
                                               const std::shared_ptr<Context>& context,
                                               const std::list<std::pair<unsigned int, unsigned>>& reducedChains) :
        AbstractRule(instance, context, true),
        reducedChains(reducedChains)
{}

solver::RuleReturnCode solver::ChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule is already applied");
    }
    isApplied = true;

    for (const auto& [start, end] : reducedChains)
    {
        for (const auto& forest : *instance)
        {
            changes.emplace(start,end,forest);
            changes.top().doAction();
        }

    }

    return RuleReturnCode::Continue;
}

void solver::ChainReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (!changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
    }
}

std::shared_ptr<solver::AbstractRule>
solver::ChainReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                         const std::shared_ptr<Context>& context)
{
    unsigned int lengthReducedChain = instance->size() + 1;

    std::unordered_set<unsigned int> visitedLabels;
    std::unordered_map<unsigned int, std::list<unsigned int>> startToChain;

    for (const auto& [_, label] : instance->at(0)->TerminalToLabel())
    {
        if (visitedLabels.contains(label))
            continue;

        startToChain.insert({label, {label}});
        visitedLabels.insert(label);

        // if we have a sibling pair with label = l1 in at least one forest
        // then several cases are relevant:
        //
        // the sibling pair (l1,l2), with uncle l3
        //     ┌──┴──┐
        //   ┌─┴─┐   l3
        //  l1   l2
        //
        // case: another sibling pair  | case: chain with l1 /2       |  case: chain l1/l2 and uncle
        //                             |                              |        as sibling pair
        //       ┌──┴──┐               |     ┌──┴──┐                  |        ┌──┴──┐
        //     ┌─┴─┐   x               |   ┌─┴─┐   l3                 |    ┌───┴─┐   x
        //    l1   l2                  | ┌─┴┐  l1/l2                  |  l1/l2   l3
        // -> equal pair rule          |  -> chain (l1, l3, ...)      |  -> chain (l1, l3, ...)
        //
        //  case: chain starting with both
        //        ┌──┴──┐
        //     ┌──┴──┐   l3
        //   ┌─┴─┐   l1/l2
        // ┌─┴┐  l2/l1
        // -> chain (l1, l2, l3, ...)
        //
        // in some of these subvariants of these cases we should also consider B-, RB- and 2B-Rule

        // for now, we exclude the case where a chain starts with a sibling pair
        bool siblingStart = false;
        for (const auto& forest : *instance)
        {
            graph::Node* chainStart = forest->LabelToTerminal().at(label);
            if (forest->TerminalToLabel().contains(chainStart->sibling))
            {
                siblingStart = true;
            }
        }
        if (siblingStart)
            continue;

        unsigned int chainElement = label;

        while (true)
        {
            bool ongoingChain = true;
            unsigned int nextChainElement = 0;

            //     ┌───┴─────┐
            //   ┌─┴─┐      nextChainElement
            //    chainElement

            for (const auto& forest : *instance)
            {
                graph::Node* chainElementNode = forest->LabelToTerminal().at(chainElement);

                // for an ongoing chain we need an uncle
                // and the parent of the uncle shouldn't be a root node
                if (not chainElementNode->parent or                   // guard: we have parent
                    not chainElementNode->parent->sibling or          // guard: the parent has a sibling (uncle)
                    not chainElementNode->parent->parent->parent)     // guard: the grandparent is not a root
                {
                    ongoingChain = false;
                    break;
                }

                // the uncle must be a terminal
                auto uncle = chainElementNode->parent->sibling;
                if (not forest->TerminalToLabel().contains(uncle))
                {
                    ongoingChain = false;
                    break;
                }

                // the uncle must have the same label as the corresponding uncles in the other forests
                unsigned int uncleLabel = forest->TerminalToLabel().at(uncle);
                if (nextChainElement == 0)
                {
                    nextChainElement = uncleLabel;
                }
                else if (nextChainElement != uncleLabel)
                {
                    ongoingChain = false;
                    break;
                }
            }
            if (ongoingChain)
            {
                if (startToChain.contains(nextChainElement))
                {
                    startToChain.at(label).splice(startToChain.at(label).end(), startToChain.at(nextChainElement));
                    startToChain.erase(nextChainElement);
                    break;
                }
                else
                {
                    chainElement = nextChainElement;
                    startToChain.at(label).push_back(chainElement);
                    visitedLabels.insert(chainElement);
                }
            }
            else
                break;
        }
    }

    std::list<std::pair<unsigned int, unsigned int>> reducedChainsStartEnd;

    for (const auto& [_, chain] : startToChain)
    {
        if (chain.size() <= lengthReducedChain)
            continue;

        auto start = chain.begin();
        std::advance(start, lengthReducedChain - 1);
        reducedChainsStartEnd.emplace_back(*start, chain.back());
    }

    if (reducedChainsStartEnd.empty())
        return nullptr;
    return std::make_shared<ChainReductionRule>(instance, context, reducedChainsStartEnd);
}

std::string solver::ChainReductionRule::name() const
{
    return "ChainReductionRule";
}

std::shared_ptr<solver::AbstractRule> solver::ChainReductionRule::clone() const
{
    return std::make_shared<ChainReductionRule>(instance,context,reducedChains);
}
