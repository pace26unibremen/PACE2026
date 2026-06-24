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

std::shared_ptr<solver::AbstractRule>
solver::SiblingRuleFactory::basicRules(const std::shared_ptr<graph::Instance>& instance,
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


std::pair<graph::Node*, graph::Node*> checkBNodes(graph::Node* const & aNode, graph::Node* const & cNode)
{
    if (aNode->parent and aNode->parent->sibling == cNode)
    {
        //   ┌──┴──┐
        // ┌─┴─┐   c
        // a   b
        return {aNode->sibling, nullptr};
    }
    if (cNode->parent and cNode->parent->sibling == aNode)
    {
        //   ┌──┴──┐
        // ┌─┴─┐   a
        // c   b
        return {cNode->sibling, nullptr};
    }
    if (aNode->parent and aNode->parent->parent and aNode->parent->parent->sibling == cNode)
    {
        //      ┌───┴──┐
        //   ┌──┴─┐    c
        // ┌─┴─┐  b2
        // a  b1
        return {aNode->sibling, aNode->parent->sibling};
    }
    if (cNode->parent and cNode->parent->parent and cNode->parent->parent->sibling == aNode)
    {
        //      ┌───┴──┐
        //   ┌──┴─┐    a
        // ┌─┴─┐  b2
        // c  b1
        return {cNode->sibling, cNode->parent->sibling};
    }
    return {nullptr, nullptr};
}

std::shared_ptr<solver::AbstractRule> solver::SiblingRuleFactory::allRules(const std::shared_ptr<graph::Instance>& instance,
                                                                             const std::shared_ptr<solver::Context>& context)
{
    const auto& [aLabel, c1Label] = getSiblings(instance, context);
    auto forestWithBNodes = std::list<std::pair<std::shared_ptr<graph::Forest>, std::list<graph::Node*>>>();

    if (aLabel * c1Label == 0)
    {
        // there is no sibling pair in f0
        return nullptr;
    }

    // c2 is the potential uncle of the sibling pair
    // which corresponds to the c-node for the reversed B rule after Whidden
    // and for the 2B Rule it would correspond to the x-node after Whidden.
    const unsigned int c2Label = [&instance, &aLabel]() -> unsigned int
    {
        const auto& f0 = instance->at(0);
        const auto& aNode = f0->LabelToTerminal().at(aLabel);
        if (f0->TerminalToLabel().contains(aNode->parent->sibling))
        {
            return f0->TerminalToLabel().at(aNode->parent->sibling);
        }
        return 0;
    }();
    unsigned int b1Label = 0;
    unsigned int b2Label = 0;

    // if the siblings from f0 are siblings in every other forest.
    bool siblings = true;

    // if the checked subtree always have c2 has uncle
    // and that a and c1+c2 are separated by the same pendant subtree b1 / b2
    bool twoBRUle = (c2Label != 0);

    // if a and c2 are a sibling pair in at least one forest
    // and the other forests have the same subtree ((a c1) c2) as f0.
    bool reverseB = (c2Label != 0);

    // if a and c1 are separated by one pendant subtree and that this subtree is b1 or b2
    bool bRule = true;

    for (unsigned int i = 1; i < instance->size(); i++)
    {
        const auto& fi = instance->at(i);
        const auto& aNode = fi->LabelToTerminal().at(aLabel);
        const auto& c1Node = fi->LabelToTerminal().at(c1Label);

        // check if we have the same sibling pair in fi
        if (aNode->sibling == c1Node)
        {
            // if 2B rule or reverse B rule are still in the race
            // we need to check that the sibling pair's uncle is c2
            if (twoBRUle or reverseB)
            {
                if ((not fi->TerminalToLabel().contains(aNode->parent->sibling)) or
                    (fi->TerminalToLabel().at(aNode->parent->sibling) != c2Label))
                {
                    twoBRUle = false;
                    reverseB = false;
                }
            }
            // and then we can move to next forest
            continue;
        }
        // From here on, we have the case that a and c1 are no sibling pairs in fi.
        siblings = false;

        // If a and c are in different components in at least one forest, then AC Branching Rule is the only option.
        const auto& aRoot = fi->rootOf(aNode);
        if (not c1Node->hasSubsetTerminals(aRoot))
        {
            return std::make_shared<ACBranchingRule>(instance, context, aLabel, c1Label);
        }

        // reverse B rule: we need to check if a and c2 are a sibling pair
        if (reverseB)
        {
            const auto& c2Node = fi->LabelToTerminal().at(c2Label);
            if (aNode->sibling != c2Node)
            {
                reverseB = false;
            }
        }

        // 2B rule: we need to check that c2 is an uncle of a or c1.
        if (twoBRUle)
        {
            const auto& c2Node = fi->LabelToTerminal().at(c2Label);
            // (not c2 'is uncle of' a) and
            // (not c2 'is uncle of' c1)
            if ((not aNode->parent or aNode->parent->sibling != c2Node) and
                (not c1Node->parent or c1Node->parent->sibling != c2Node))
            {
                twoBRUle = false;
            }
        }

        // B rule and 2B rule: we need to check the b node
        // it follows a long block where b1Label and b2Label are assigned
        const auto& [b1Node, b2Node] = checkBNodes(aNode, c1Node);
        if (not b1Node)
        {
            // current case:
            // a and c1 are connected, but there are more than two subtrees 'between' a and c1
            bRule = false;
            twoBRUle = false;
        }
        else if (not b2Node)
        {
            // current case:
            //   ┌──┴──┐     /   ┌──┴──┐
            // ┌─┴─┐   c1   /  ┌─┴─┐   a
            // a   b1Node  /  c1   b1Node
            // at this point we don't checked anything about the label of the b1 Node

            // fill the b Nodes list for the ACB Branching rule
            forestWithBNodes.push_back({fi,{b1Node}});

            // from here on, we check B rule and 2B rule stuff, so if none of them is possible anymore -> skip
            if (not twoBRUle and not bRule)
            {
                continue;
            }

            if (not fi->TerminalToLabel().contains(b1Node))
            {
                // b1Node is not a terminal
                bRule = false;
                twoBRUle = false;
                continue;
            }

            const auto possibleBLabel = fi->TerminalToLabel().at(b1Node);
            if (possibleBLabel == b1Label or possibleBLabel == b2Label)
            {
                //TODO i think b rule can be generalized accept two different b labels
                if (possibleBLabel == b2Label)  //
                {                               //  remove for generalization
                    bRule = false;              //
                }                               //

                // if the b1 Nodes matches the stored labels for b1 or b2
                continue;
            }
            if (b1Label == 0)
            {
                // if b1 was not assigned until now, we can do this now
                b1Label = possibleBLabel;
                continue;
            }
            if (b2Label == 0)
            {
                // if b2 was not assigned until now, we can do this now
                bRule = false; // TODO  remove for b rule generalization
                b2Label = possibleBLabel;
                continue;
            }
            bRule = false;
            twoBRUle = false;
            continue;
        }
        else
        {
            // fill the b Nodes list for the ACB Branching rule
            forestWithBNodes.push_back({fi,{b1Node, b2Node}});
            // current case:
            //       ┌───┴──┐     /          ┌───┴──┐
            //    ┌──┴─┐    c1   /        ┌──┴─┐    a
            //  ┌─┴─┐  b2       /       ┌─┴─┐  b2
            //  a  b1          /       c1   b1
            // at this point we don't checked anything about the label of the b1 Node

            // b rule can not be applied anymore
            bRule = false;

            // from here on, we only check 2 B rule stuff, so if 2 B is not possible, we can skip this
            if (not twoBRUle)
            {
                continue;
            }

            if (not fi->TerminalToLabel().contains(b1Node) or
                not fi->TerminalToLabel().contains(b2Node))
            {
                // b1Node or b2Node is not a terminal
                twoBRUle = false;
                continue;
            }
            const auto possibleB1Label = fi->TerminalToLabel().at(b1Node);
            const auto possibleB2Label = fi->TerminalToLabel().at(b2Node);

            // check possibleB1Label first:
            if (possibleB1Label == b1Label or possibleB1Label == b2Label)
            {}
            else if (b1Label == 0)
            {
                // if b1 was not assigned until now, we can do this now
                b1Label = possibleB1Label;
            }
            else if (b2Label == 0)
            {
                // if b2 was not assigned until now, we can do this now
                b2Label = possibleB1Label;
            }
            else
            {
                // possibleB1Label didn't match previous assignments for b1Label/b2Label.
                twoBRUle = false;
                continue;
            }

            // check now possibleB2Label:
            if (possibleB2Label == b1Label and possibleB2Label == b2Label)
            {
                continue;
            }
            if (b1Label == 0)
            {
                // if b1 was not assigned until now, we can do this now
                b1Label = possibleB2Label;
                continue;
            }
            if (b2Label == 0)
            {
                // if b2 was not assigned until now, we can do this now
                b2Label = possibleB2Label;
                continue;
            }
            // possibleB1Label didn't match previous assignments for b1/b2.
            twoBRUle = false;
            continue;
        }

        // The current situation in fi is:
        // a and c1 are connected, but there are more than two subtrees 'between' a and c1.
        // (see first condition of b1Label-b2Label block)
        // This is the ABC branching rule case, so we collect all b-nodes between a and c1.
        //
        // moving upwards from aNode to lca - and collect b-nodes
        auto bNodes = std::list<graph::Node*>();
        graph::Node* lca;
        graph::Node* it = aNode;
        while (true)
        {
            if (c1Node->hasSubsetTerminals(it->parent))
            {
                lca = it->parent;
                break;
            }
            bNodes.push_back(it->sibling);
            it = it->parent;
        }

        // moving upwards from cNode to lca - and collect b-nodes
        it = c1Node;
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
        return std::make_shared<EqualPairReductionRule>(instance, context, aLabel, c1Label);
    }
    if (bRule)
    {
        return std::make_shared<BRule>(instance, context, b1Label);
    }
    if (reverseB)
    {
        return std::make_shared<ReverseBRule>(instance, context, c1Label);
    }
    if (twoBRUle)
    {
        // there is an edge case, that we didn't checked until now:
        // b1 or b2 could be c2.
        // This is because when we checked that c2 is an uncle of the subtree
        // we actually checked if it's an uncle of the a-node (logical-) or c1-node
        // regardless which one is the higher node.
        if (b1Label != c2Label and b2Label != c2Label)
        {
            return std::make_shared<TwoBRule>(instance, context, std::pair(b1Label, b2Label));
        }
    }

    return std::make_shared<ABCBranchingRule>(instance, context, aLabel, c1Label, forestWithBNodes);
}
