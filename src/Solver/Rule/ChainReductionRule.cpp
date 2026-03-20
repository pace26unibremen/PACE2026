#include "ChainReductionRule.hpp"

#include "../Action/DeleteNodeActionInChains.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) :
        AbstractRule(instance, context)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = std::move(chainWithTrees);
    parentToXN = {nullptr, nullptr};
    chainNodes = {};
    chainLabels = {};
}

//Chain def:Let T be a rooted phylogenetic X-tree, and let C =
//(x1, x2, . . . , xn) be a sequence of elements in X with n ≥ 2.
//X:= Set of labels, bijectively used on leaves in the tree.

//We say that C is an n-chain (or short chain) of T
//if the parent of x1 coincides with the parent of x2
//or the parent of x2 is the parent of the parent of x1,
//and, for each i ∈ {3, 4, . . . , n},
//the parent of xi is the parent of the parent of xi−1.

solver::RuleReturnCode solver::ChainReductionRule::apply()
{
    //Goal: Deleting edges from the first and last chain node element to its not-terminal child and parent respectively
    //Needs to do:
    //Identify nodes within chainWithTrees that fit the description
    //If they're the first chain node, then remove the edge to his not-terminal child
    //  This is always there given the chain definition
    //Else if it's the last node in the list, then remove his edge to the parent of his.
    // Always there if the chain exists. At most the root of the tree.
    //Respectively, this needs to happen to the other side of the edge, i.e for x4 it must be done on x3,
    //and on xn+1 for xn, as it's two sided.
    // Forest needs to be writable -> Removal of nodes within chain out of Nodes(), Terminals() and LabelToTerminal
    // -> Storage is required -> protected class wide variables for those.
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    //Repeat check for safety, latter >= 2 means that chain was indeed larger then 3 elements, as x1 and x2 are never
    //stored due to being irrelevant
    if (chainWithTrees.second.size() == 2 && chainWithTrees.first.size() >= 2)
    {
        //Acquire the notes required
        //Fetch x3
        graph::Node* bottomT1 = chainWithTrees.first[0].front();
        graph::Node* bottomT2 = chainWithTrees.first[0].back();

        //x4
        graph::Node* bottomChainT1 = chainWithTrees.first[1].front();
        graph::Node* bottomChainT2 = chainWithTrees.first[1].back();

        //Fetch xn
        graph::Node* topChainT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
        graph::Node* topChainT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

        //Fetch xn's parent out of tree.
        graph::Node* topChainT1Parent = chainWithTrees.first[chainWithTrees.first.size()-1].front()->parent;
        graph::Node* topChainT2Parent = chainWithTrees.first[chainWithTrees.first.size()-1].back()->parent;

        //Remove the edge between x3 and x4, replace with edge between x3 and topChainT1Parent
        bottomT1->parent = topChainT1Parent;
        bottomT2->parent = topChainT2Parent;

        //Update x4 to reflect x3 change
        if (bottomChainT1->leftChild == bottomT1)
        {
            bottomChainT1->leftChild = nullptr;
        }
        else if (bottomChainT1->rightChild == bottomT1)
        {
            bottomChainT1->rightChild = nullptr;
        }

        if (bottomChainT2->leftChild == bottomT2)
        {
            bottomChainT2->leftChild = nullptr;
        }
        else if (bottomChainT2->rightChild == bottomT2)
        {
            bottomChainT2->rightChild = nullptr;
        }

        //Remove edge between xn and its parent and update parent accordingly with x3 being it's new child.
        topChainT1->parent = nullptr;
        topChainT2->parent = nullptr;

        if (topChainT1Parent->leftChild == topChainT1)
        {
            topChainT1Parent->leftChild = bottomT1;
        }
        else if (topChainT1Parent->rightChild == topChainT1)
        {
            topChainT1Parent->rightChild = bottomT1;
        }

        if (topChainT2Parent->leftChild == topChainT2)
        {
            topChainT2Parent->leftChild = bottomT2;
        }
        else if (topChainT2Parent->rightChild == topChainT2)
        {
            topChainT2Parent->rightChild = bottomT2;
        }
#ifdef DEBUG_IMAGE_VIEW_GRAPH
        T1->renderImage();
#endif
#ifdef DEBUG_IMAGE_VIEW_GRAPH
        T2->renderImage();
#endif
        //Now, remove labels, terminals and nodes out of the two trees that are within the relevant section of the chain
        //,while storing them within this instance for unapply.
        // //Storage?
        // graph::Node chainBottomCutoffT1;
        // graph::Node chainBottomCutoffT2;
        // graph::Node chainTopCutoffT1;
        // graph::Node chainTopCutoffT2;
        //
        // //T1
        // for (auto node : chainWithTrees.second.front()->Nodes())
        // {
        //     //Remove x3's parent connection to x4
        //     if (&node == bottomT1)
        //     {
        //         node.parent = topT1->parent;
        //         // graph::Node* bottomNodeInT1 = &node;
        //         // bottomNodeInT1->parent = topT1->parent;
        //     }
        //
        //     //Remove x4's parent connection to x3
        //     if (&node == bottomT1->parent)
        //     {
        //         if (node.leftChild == bottomT1)
        //         {
        //             node.leftChild = nullptr;
        //         }
        //         else if (node.rightChild == bottomT1)
        //         {
        //             node.rightChild = nullptr;
        //         }
        //     }
        //
        //     //Exclusive argument, if left then changes[0] is nullptr, if right then topT1->parent is nullptr;
        //     //Remove xn's parent connction to the last chain element
        //     if (&node == topT1->parent || &node == parentToXN.first)
        //
        //     {
        //         if (node.leftChild == topT1)
        //         {
        //             node.leftChild = &chainBottomCutoffT1;
        //         }
        //         else if (node.rightChild == topT1)
        //         {
        //             node.rightChild = &chainBottomCutoffT1;
        //         }
        //         if (parentToXN.first == nullptr)
        //         {
        //             chainTopCutoffT1 = node;
        //         }
        //         else
        //         {
        //             chainTopCutoffT1 = *parentToXN.first;
        //         }
        //     }
        //
        //     //Remove xn's parent connection to his parent
        //     if (&node == topT1)
        //     {
        //         graph::Node* topNodeinT1 = &node;
        //         parentToXN.first = (topNodeinT1->parent);
        //         topNodeinT1->parent = nullptr;
        //     }
        // }
        //
        // //T2
        // for (auto& node : chainWithTrees.second.back()->Nodes())
        // {
        //     //Remove x3's parent connection to x4
        //     if (&node == bottomT2)
        //     {
        //         graph::Node* bottomNodeInT2 = &node;
        //         bottomNodeInT2->parent = topT2->parent;
        //     }
        //     //Remove x4's parent connection to x3
        //     if (&node == bottomT2->parent)
        //     {
        //         if (node.leftChild == bottomT2)
        //         {
        //             node.leftChild = nullptr;
        //         }
        //         else if (node.rightChild == bottomT2)
        //         {
        //             node.rightChild = nullptr;
        //         }
        //     }
        //
        //     //Exclusive argument, if left then changes[0] is nullptr, if right then topT1->parent is nullptr;
        //     //Remove xn's parent connction to the last chain element
        //     if (&node == topT2->parent || &node == parentToXN.second)
        //     {
        //         if (node.leftChild == topT2)
        //         {
        //             node.leftChild = &chainBottomCutoffT2;
        //         }
        //         else if (node.rightChild == topT2)
        //         {
        //             node.rightChild = &chainBottomCutoffT2;
        //         }
        //         if (parentToXN.second == nullptr)
        //         {
        //             chainTopCutoffT2 = node;
        //         }
        //         else
        //         {
        //             chainTopCutoffT2 = *parentToXN.second;
        //         }
        //     }
        //     //Remove xn's connection to his parent
        //     if (&node == topT2)
        //     {
        //         graph::Node* topNodeinT2 = &node;
        //         parentToXN.second = (topNodeinT2->parent);
        //         topNodeinT2->parent = nullptr;
        //     }
        // }
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

    //Find nodes that belong to chain
    graph::Node* bottomT1 = chainWithTrees.first[1].front();
    graph::Node* bottomT2 = chainWithTrees.first[1].back();
    graph::Node* topT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
    graph::Node* topT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

    //x3 to x4 and x4 to x3 for T1
    for (auto& node : chainWithTrees.second.front()->Nodes())
    {
        if(&node == bottomT1)
        {
            for (auto& node2 : chainWithTrees.second.front()->Nodes())
            {
                if (&node2 == chainWithTrees.first[2].front())
                {
                    node.parent = &node2;
                    if (node2.leftChild == nullptr)
                    {
                        node2.leftChild = &node;
                        break;
                    }
                    if (node2.rightChild == nullptr)
                    {
                        node2.rightChild = &node;
                        break;
                    }
                }
            }
        }
        //Top node of chain connected to his parent again
        if ( &node == parentToXN.first)
        {
            for (auto& node2 : chainWithTrees.second.front()->Nodes())
            {
                node.parent = &node2;
                if (node2.leftChild == nullptr)
                {
                    node2.leftChild = &node;
                    parentToXN.first = nullptr;
                    break;
                }
                if (node2.rightChild == nullptr)
                {
                    node2.rightChild = &node;
                    parentToXN.first = nullptr;
                    break;
                }
            }
        }
    }
    //T2, both x3<->x4 and top chain <-> his parent
    for (auto& node : chainWithTrees.second.back()->Nodes())
    {
        if(&node == bottomT2)
        {
            for (auto& node2 : chainWithTrees.second.back()->Nodes())
            {
                if (&node2 == chainWithTrees.first[2].back())
                {
                    node.parent = &node2;
                    if (node2.leftChild == nullptr)
                    {
                        node2.leftChild = &node;
                        break;
                    }
                    if (node2.rightChild == nullptr)
                    {
                        node2.rightChild = &node;
                        break;
                    }
                }
            }
        }
        if ( &node == parentToXN.second)
        {
            for (auto& node2 : chainWithTrees.second.back()->Nodes())
            {
                node.parent = &node2;
                if (node2.leftChild == nullptr)
                {
                    node2.leftChild = &node;
                    parentToXN.second = nullptr;
                    break;
                }
                if (node2.rightChild == nullptr)
                {
                    node2.rightChild = &node;
                    parentToXN.second = nullptr;
                    break;
                }
            }
        }
    }
}

std::shared_ptr<solver::AbstractRule>
solver::ChainReductionRule::isApplicable(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context)
{
    //First found Chain in both trees
    std::vector<std::vector<graph::Node*>> chain;
    std::vector<std::shared_ptr<graph::Forest>> chainTrees;
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees;

    for (const auto& T1 : *instance)
    {   //DEBUG
        //std::cout << "T1:" << std::endl;
        //T1->write(std::cout);

        for (const auto& T2 : *instance)
        {   //DEBUG
            //std::cout << "T2:" << std::endl;
            //T2->write(std::cout);

            if (T1 != T2)
            {
                //std::cout << "equal check" << std::endl;
                //Fetch Leaves - Terminal Index to Label
                std::unordered_map<graph::Node*,unsigned int> termIndexTreeOne = T1->TerminalToLabel();
                std::unordered_map<graph::Node*,unsigned int> termIndexTreeTwo = T2->TerminalToLabel();

                //For all Terminals...
                for (const auto& terminalT1 : termIndexTreeOne)
                {  //T1
                    //DEBUG
                    //std::cout << "terminalT1: " << terminalT1.second << std::endl;

                    //Determine parent
                    graph::Node* parentT1 = terminalT1.first->parent;

                    //Determine if sibling exists
                    graph::Node* siblingT1 = terminalT1.first->sibling;

                    // -> x1, x2 for case 1 for chain def known for T1

                    //anti single vertex check for terminalT1
                    if (parentT1 != nullptr )
                    {
                        //Determine x3 and x2 for case 1 for chain in T1
                        graph::Node* parentOfParentT1 = parentT1->parent;
                        graph::Node* siblingOfParentT1 = parentT1->sibling;

                        //Case 2 structure check
                        int case2SiblingIsOnLeftSideT1 = -1;

                        if (parentOfParentT1 != nullptr)
                        {
                            if (parentOfParentT1->leftChild == siblingOfParentT1)
                            {
                                case2SiblingIsOnLeftSideT1 = 1;
                            }
                            else
                            {
                                case2SiblingIsOnLeftSideT1= 0;
                            }
                        }

                        if (not chain.empty())
                        {
                            break;
                        }

                        for (const auto& terminalT2 : termIndexTreeTwo)
                        {   //T2
                            //std::cout << "terminalT2: " << terminalT2.second << std::endl;
                            //Determine Structure
                            graph::Node* parentT2 = terminalT2.first->parent;
                            graph::Node* siblingT2 = terminalT2.first->sibling;

                            // -> x1,x2,x3 for case 1 known for T2

                            //Anti single vertex check for terminalT2
                            if (parentT2 != nullptr)
                            {
                                //Determine x3 for case 2 for chain def in T2
                                graph::Node* parentOfParentT2 = parentT2->parent;
                                graph::Node* siblingOfParentT2 = parentT2->sibling;

                                bool case1applied = false;

                                //Case 1: if the parent of x1 coincides with the parent of x2
                                if (terminalT1.first->parent->leftChild != nullptr
                                    && terminalT1.first->parent->rightChild != nullptr
                                    && T1->TerminalToLabel().contains(terminalT1.first->parent->leftChild)
                                    && T1->TerminalToLabel().contains(terminalT1.first->parent->rightChild)
                                    && terminalT2.first->parent->leftChild != nullptr
                                    && terminalT2.first->parent->rightChild != nullptr
                                    && T2->TerminalToLabel().contains(terminalT2.first->parent->leftChild)
                                    && T2->TerminalToLabel().contains(terminalT2.first->parent->rightChild)
                                )
                                {
                                    //std::cout << "case 1" << std::endl;
                                    //Chain for both trees, x1-x3
                                    std::vector<std::vector<graph::Node*>> chainT1T2 = {{parentT1,parentT2}};

                                    //Chain collection bool
                                    bool case1check = true;

                                    //Determine x4 for T1, T2
                                    graph::Node* case1T1Parent = parentT1;
                                    graph::Node* case1T2Parent = parentT2;

                                    //Determine Sibling of x3
                                    graph::Node* case1T1Sibling;
                                    graph::Node* case1T2Sibling;

                                    if (case1T1Parent != nullptr && case1T2Parent != nullptr)
                                    {
                                        //T1 structure check, determining sibling of x3 -> Supposed terminal
                                        if (case1T1Parent->leftChild == chainT1T2.back().front())
                                        {
                                            case1T1Sibling = case1T1Parent->rightChild;
                                        }
                                        else
                                        {
                                            case1T1Sibling = case1T1Parent->leftChild;
                                        }
                                        //T2 structure c heck
                                        if (case1T2Parent->leftChild == chainT1T2.back().back())
                                        {
                                            case1T2Sibling = case1T2Parent->rightChild;
                                        }
                                        else
                                        {
                                            case1T2Sibling = case1T2Parent->leftChild;
                                        }
                                    }
                                    //Determine x4,... of chain
                                    while (case1check)
                                    {   //std::cout << chainT1T2.size() << std::endl;
                                        //Sibling is in fact a terminal and is not attached to root
                                        if (case1T1Sibling != nullptr && case1T2Sibling != nullptr
                                           && case1T1Parent != nullptr && case1T2Parent != nullptr
                                           && case1T1Sibling->leftChild == nullptr && case1T1Sibling->rightChild == nullptr
                                           && case1T2Sibling->leftChild == nullptr && case1T2Sibling->rightChild == nullptr
                                           && std::find(T1->Roots().begin(), T1->Roots().end(), case1T1Parent)
                                           == T1->Roots().end()
                                           && std::find(T2->Roots().begin(), T2->Roots().end(), case1T2Parent)
                                           == T2->Roots().end()
                                           && case1T1Parent != *T1->Roots().end()
                                           && case1T2Parent != *T2->Roots().end()
                                           )
                                        {
                                            chainT1T2.push_back({case1T1Parent,case1T2Parent});
                                            case1T1Parent = chainT1T2.back().front()->parent;
                                            case1T2Parent = chainT1T2.back().back()->parent;
                                            //T1 structure check
                                            if (case1T1Parent != nullptr && case1T2Parent != nullptr)
                                            {
                                                if (case1T1Parent->leftChild == chainT1T2.back().front())
                                                {
                                                    case1T1Sibling = case1T1Parent->rightChild;
                                                }
                                                else
                                                {
                                                    case1T1Sibling = case1T1Parent->leftChild;
                                            }
                                                //T2 structure c heck
                                                if (case1T2Parent->leftChild == chainT1T2.back().back())
                                                {
                                                    case1T2Sibling = case1T2Parent->rightChild;
                                                }
                                                else
                                                {
                                                    case1T2Sibling = case1T2Parent->leftChild;
                                                }

                                                case1applied = true;
                                            }
                                            else
                                            {
                                                case1check = false;
                                            }
                                        }
                                        else case1check = false;
                                    }

                                    if (chainT1T2.size() >= 2)
                                    {
                                        chain = chainT1T2;
                                        chainTrees.emplace_back(T1);
                                        chainTrees.emplace_back(T2);
                                        chainWithTrees.first = chain;
                                        chainWithTrees.second = chainTrees;
                                        return std::dynamic_pointer_cast<AbstractRule>(
                                        std::make_shared<ChainReductionRule>(instance,chainWithTrees,context));
                                    }
                                }
                                //Case 2: or the parent of x2 is the parent of the parent of x1, i.e parent index of sibling is
                                //the same as the index for the parent of the parent on both trees, as well as for x3 and x2
                                //Only one terminal for each parent node must exist
                                else if (
                                // Parent has indeed only one terminal on both trees
                                    ((parentT1->leftChild == terminalT1.first && parentT1->rightChild == nullptr) ||
                                    (parentT1->rightChild == terminalT1.first && parentT1->leftChild == nullptr))

                                    && ((parentT2->leftChild == terminalT2.first && parentT2->rightChild == nullptr) ||
                                    (parentT2->rightChild == terminalT2.first && parentT2->leftChild == nullptr))
                                // Parent of Parent of first terminal exists and...
                                    && parentOfParentT1 != nullptr && parentOfParentT2 != nullptr
                                // parent of Parent has one terminal x2 on both trees
                                    && ((parentOfParentT1->leftChild == parentT1
                                        && T1->TerminalToLabel().contains(parentOfParentT1->rightChild))
                                        ||(parentOfParentT1->rightChild == parentT1
                                        && T1->TerminalToLabel().contains(parentOfParentT1->leftChild)))

                                        && ((parentOfParentT2->leftChild == parentT2
                                            && T2->TerminalToLabel().contains(parentOfParentT2->rightChild))
                                        ||(parentOfParentT2->rightChild == parentT2
                                            && T2->TerminalToLabel().contains(parentOfParentT2->leftChild)))
                                //Parent of Parent of x2 exists on both trees and...
                                    && parentOfParentT1->parent != nullptr && parentOfParentT2->parent != nullptr
                                // Has a terminal on the other child side
                                    && ((parentOfParentT1->parent->leftChild == parentOfParentT1
                                        && T1->TerminalToLabel().contains(parentOfParentT1->parent->rightChild))
                                        || (parentOfParentT1->parent->rightChild == parentOfParentT1
                                        && T1->TerminalToLabel().contains(parentOfParentT1->parent->leftChild)))

                                        &&((parentOfParentT2->parent->leftChild == parentOfParentT2
                                        && T2->TerminalToLabel().contains(parentOfParentT2->parent->rightChild))
                                        || (parentOfParentT2->parent->rightChild == parentOfParentT2
                                        && T2->TerminalToLabel().contains(parentOfParentT2->parent->leftChild)))
                                        )
                                {
                                    //std::cout << "case 2" << std::endl;
                                    //Chain for both trees, parent nodes of x1, x2, x3
                                    std::vector<std::vector<graph::Node*>> chainT1T2 = {
                                    {parentOfParentT1->parent,parentOfParentT2->parent}};

                                    graph::Node* case2T1Parent = parentOfParentT1->parent;
                                    graph::Node* case2T2Parent = parentOfParentT2->parent;

                                    graph::Node* case2T1Sibling;
                                    graph::Node* case2T2Sibling;
                                    if (case2T1Parent != nullptr && case2T2Parent != nullptr)
                                    {
                                        //T1 structure check
                                        if ( case2T1Parent->leftChild == chainT1T2.back().front())
                                        {
                                            case2T1Sibling = case2T1Parent->rightChild;
                                        }
                                        else
                                        {
                                            case2T1Sibling = case2T1Parent->leftChild;
                                        }
                                        //T2 structure c heck
                                        if (case2T2Parent->leftChild == chainT1T2.back().back())
                                        {
                                            case2T2Sibling = case2T2Parent->rightChild;
                                        }
                                        else
                                        {
                                            case2T2Sibling = case2T2Parent->leftChild;
                                        }

                                    }
                                    //Chain collection bool
                                    bool case2check = true;

                                    //Determine x4,... of chain
                                    while (case2check)
                                    {
                                        //std::cout << chainT1T2.size() << std::endl;
                                        //Siblings exist
                                        if (case2T1Sibling != nullptr && case2T2Sibling != nullptr
                                            && case2T1Parent != nullptr && case2T2Parent != nullptr
                                            && case2T1Sibling->leftChild == nullptr && case2T1Sibling->rightChild == nullptr
                                            && case2T2Sibling->leftChild == nullptr && case2T2Sibling->rightChild == nullptr
                                            && std::find(T1->Roots().begin(), T1->Roots().end(), case2T1Parent)
                                           == T1->Roots().end()
                                           && std::find(T2->Roots().begin(), T2->Roots().end(), case2T2Parent)
                                           == T2->Roots().end()
                                        )
                                        {
                                            chainT1T2.push_back({case2T1Parent,case2T2Parent});
                                            case2T1Parent = chainT1T2.back().front()->parent;
                                            case2T2Parent = chainT1T2.back().back()->parent;

                                            if (case2T1Parent != nullptr && case2T2Parent != nullptr)
                                            {
                                                //T1 structure check
                                                if (case2T1Parent->leftChild == chainT1T2.back().front())
                                                {
                                                    case2T1Sibling = case2T1Parent->rightChild;
                                                }
                                                else
                                                {
                                                    case2T1Sibling = case2T1Parent->leftChild;
                                                }
                                                //T2 structure c heck
                                                if (case2T2Parent->leftChild == chainT1T2.back().back())
                                                {
                                                    case2T2Sibling = case2T1Parent->rightChild;
                                                }
                                                else
                                                {
                                                    case2T2Sibling = case2T2Parent->leftChild;
                                                }
                                            }
                                            else
                                            {
                                                case2check = false;
                                            }
                                        }
                                        else case2check = false;
                                    }

                                    if (chainT1T2.size() >= 2)
                                    {
                                        chain = chainT1T2;
                                        chainTrees.emplace_back(T1);
                                        chainTrees.emplace_back(T2);
                                        chainWithTrees.first = chain;
                                        chainWithTrees.second = chainTrees;
                                        return std::dynamic_pointer_cast<AbstractRule>(
                                        std::make_shared<ChainReductionRule>(instance,chainWithTrees,context));
                                    }

                                }
                                if (not chain.empty())
                                {
                                  break;
                                }
                            }
                        }
                    }
                }
            }
            if (not chain.empty())
            {
                break;
            }
        }
        if(not chain.empty())
        {
            break;
        }
    }
    //when no chain was found:

    return nullptr;

}


std::string solver::ChainReductionRule::name() const
{
    return "ChainReductionRule";
}

