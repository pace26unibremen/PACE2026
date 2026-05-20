//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

#include <iostream>

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    const std::pair<graph::Node*,std::vector<std::shared_ptr<graph::Forest>>>& nodeAndTrees) :
        AbstractRule(instance, context, true)
{
    this->nodeAndTrees = nodeAndTrees;
    std::cout << "Terminal.at 1" << std::endl;
    for (const auto& terminal : nodeAndTrees.second.front()->TerminalToLabel())
    {
        if (terminal.first == nodeAndTrees.first)
        {
            this->nodeLabel = terminal.second;
            break;
        }
    }
    //this->nodeLabel = nodeAndTrees.second.front()->Terminals().at(nodeAndTrees.first);
    changes = std::stack<solver::AbstractAction>{};
}

solver::RuleReturnCode solver::ThreeTwoChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    for (const auto& tree : nodeAndTrees.second)
    {
        graph::Node* node = nodeAndTrees.first;
        graph::Node* parent = node->parent;
        //Delete node out of nodes
        if (parent->leftChild == node)
        {
            parent->leftChild = nullptr;
        }
        else
        {
            parent->rightChild = nullptr;
        }

        //Delete out of element lists
        for (int index=0; index < tree->Nodes().size() ; index++)
        {   //
            graph::Node* currentNode = &tree->Nodes().at(index);
            if (currentNode == node)
            {
                // tree->Nodes().erase(tree->Nodes().begin()+index-1);
                // break;
            }
        }

        for (const auto& terminal : tree->TerminalToLabel())
        {
            if (terminal.first == node)
            {
                // tree->TerminalToLabel().erase(terminal.first);
                // break;
            }
        }

        tree->TerminalToLabel().erase(node);

        //Da node Terminal ist -> Keine Kinder, Keine Wiederverknüpfung.
    }

    return RuleReturnCode::Continue;
}

void solver::ThreeTwoChainReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    for (const auto& tree : nodeAndTrees.second)
    {
        graph::Node* node = nodeAndTrees.first;
        graph::Node* parent = node->parent;
        if (parent->leftChild == nullptr)
        {
            parent->leftChild = node;
        }
        else if (parent->rightChild == nullptr)
        {
            parent->rightChild = node;
        }
        else
        {
            parent->leftChild = node;
        }

        tree->Nodes().emplace_back(*node);
        tree->TerminalToLabel().emplace(node,nodeLabel);
        tree->LabelToTerminal().emplace(nodeLabel, node);
    }

}

std::shared_ptr<solver::AbstractRule>
solver::ThreeTwoChainReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                 const std::shared_ptr<Context>& context)
{
    for (const auto& T1 : *instance)
    {
        //Let (x1, x2, x3) be a pendant 3-chain
        //of T. If (xi , x3) is a pendant 2-chain in T' with xi ∈ {x1, x2},
        //then set S = T |X \ {x j} and S' = T'|X \ {x j} with {xi , x j} =
        //{x1, x2}
        //Pendant -> Parent of x1 and x2 is the same reference -> Solely Case 1 of ChainReductionRule
        for (const auto& T2 : *instance)
        {
            if (T1 != T2)
            {
                //T1: T in def
                for (const auto& terminalT1 : T1->TerminalToLabel())
                {
                    //Find a 3-Pendant chain
                    //Anti Single Vertex Check:
                    if (terminalT1.first->parent != nullptr)
                    {
                        //Structure check: Sibling of terminal is also connected to parent as well as also terminal->
                        //pendant.
                        //Case1: x1 and x2 share parent, Sibling of Parent, x3, and Parent share Parent too
                        if (terminalT1.first->sibling != nullptr &&
                            terminalT1.first->sibling->parent == terminalT1.first->parent &&
                            terminalT1.first->parent->sibling != nullptr &&
                            terminalT1.first->parent->parent == terminalT1.first->parent->sibling->parent &&
                            T1->TerminalToLabel().contains(terminalT1.first->sibling) &&
                            T1->TerminalToLabel().contains(terminalT1.first->parent->sibling))
                        {
                            //x1,x2,x3 known: x1 Term, x2 sibling, x3 parent
                            std::cout << "Terminal.at 2" << std::endl;
                            std::vector<unsigned int> chainInT1Ints = {terminalT1.second,
                                T1->TerminalToLabel().at(terminalT1.first->sibling),
                                T1->TerminalToLabel().at(terminalT1.first->parent->sibling)};
                            std::cout << "Terminal.at 3" << std::endl;

                            //If (xi , x3) is a pendant 2-chain in T' with xi ∈ {x1, x2}
                            //Case 1: Sharing same parent. | Currently assuming that labels are shared...
                            //Case 1.1: x1 and x3 in T2 share Parent
                            if(T2->LabelToTerminal().at(chainInT1Ints.front())->parent ==
                                T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first->sibling;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1,T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context,nodeAndTrees);

                            }
                            //Case 1.2: x2 and x3 in T2 share Parent
                            if (T2->LabelToTerminal().at(chainInT1Ints.at(1))->parent ==
                                T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1, T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context, nodeAndTrees);
                            }
                            //Case 2.1: x1 parent and x3 share Parent
                            if (T2->LabelToTerminal().at(chainInT1Ints.front())->parent != nullptr &&
                                     T2->LabelToTerminal().at(chainInT1Ints.front())->parent->parent ==
                                         T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first->sibling;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1,T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context,nodeAndTrees);
                            }
                            //Case 2.2: x2 Parent and x3 share Parent
                            if (T2->LabelToTerminal().at(chainInT1Ints.at(1))->parent != nullptr &&
                                     T2->LabelToTerminal().at(chainInT1Ints.at(1))->parent->parent ==
                                         T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1, T2};
                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context, nodeAndTrees);
                            }
                        }

                        //Case 2: parent of X1 and x2 share parent, parent of x2 and x3 share parent.
                        else if ( terminalT1.first->parent->parent != nullptr &&
                            terminalT1.first->parent->sibling != nullptr &&
                            terminalT1.first->parent->sibling->parent == terminalT1.first->parent->parent &&
                            terminalT1.first->parent->parent->parent != nullptr &&
                            terminalT1.first->parent->parent->sibling != nullptr &&
                            terminalT1.first->parent->parent->sibling->parent ==
                            terminalT1.first->parent->parent->parent &&
                            T1->TerminalToLabel().contains(terminalT1.first->parent->sibling) &&
                            T1->TerminalToLabel().contains(terminalT1.first->parent->parent->sibling)
                            )
                        {
                            std::cout << "Terminal.at 4" << std::endl;
                            std::vector<unsigned int> chainInT1Ints = {terminalT1.second,
                                T1->TerminalToLabel().at(terminalT1.first->parent->sibling),
                                T1->TerminalToLabel().at(terminalT1.first->parent->parent->sibling)};

                            //If (xi , x3) is a pendant 2-chain in T' with xi ∈ {x1, x2}
                            //Case 1: Sharing same parent. | Currently assuming that labels are shared...
                            //Case 1.1: x1 and x3 in T2 share Parent
                            if(T2->LabelToTerminal().at(chainInT1Ints.front())->parent ==
                                T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first->sibling;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1,T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                   nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context,nodeAndTrees);

                            }
                            //Case 1.2: x2 and x3 in T2 share Parent
                            if (T2->LabelToTerminal().at(chainInT1Ints.at(1))->parent ==
                                T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1, T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context, nodeAndTrees);
                            }
                            //Case 2.1: x1 parent and x3 share Parent
                            if (T2->LabelToTerminal().at(chainInT1Ints.front())->parent != nullptr &&
                                     T2->LabelToTerminal().at(chainInT1Ints.front())->parent->parent ==
                                         T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first->sibling;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1,T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context,nodeAndTrees);
                            }
                            //Case 2.2: x2 Parent and x3 share Parent
                            if (T2->LabelToTerminal().at(chainInT1Ints.at(1))->parent != nullptr &&
                                     T2->LabelToTerminal().at(chainInT1Ints.at(1))->parent->parent ==
                                         T2->LabelToTerminal().at(chainInT1Ints.back())->parent)
                            {
                                graph::Node* toBeRemovedNode = terminalT1.first;

                                std::vector<std::shared_ptr<graph::Forest>> forestList = {T1, T2};

                                std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>
                                    nodeAndTrees = {toBeRemovedNode, forestList};

                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context, nodeAndTrees);
                            }
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

std::string solver::ThreeTwoChainReductionRule::name() const
{
    return "ThreeTwoChainReductionRule";
}
