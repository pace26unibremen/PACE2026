//
// Created by Andre on 11.12.2025.
//

#include "ChainReductionRule.hpp"

#include "../Action/DeleteNodeActionInChains.hpp"
#include <algorithm>
#include <iostream>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) : AbstractRule(instance,context)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = chainWithTrees;
    changes = {nullptr,nullptr};
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
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    if (chainWithTrees.second.size() == 2 && chainWithTrees.first.size() >= 4)
    {
        //Acquire the notes required
        graph::Node* bottomT1 = chainWithTrees.first[3].front();
        graph::Node* bottomT2 = chainWithTrees.first[3].back();
        graph::Node* topT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
        graph::Node* topT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

        graph::Node chainBottomCutoffT1;
        graph::Node chainBottomCutoffT2;
        graph::Node chainTopCutoffT1;
        graph::Node chainTopCutoffT2;

        //T1
        for (auto& node : chainWithTrees.second.front()->Nodes())
        {
            //Remove x3's parent connection to x4
            if (&node == bottomT1)
            {
                graph::Node* bottomNodeInT1 = &node;
                bottomNodeInT1->parent = &chainTopCutoffT1;
                chainBottomCutoffT1 = *bottomNodeInT1;
            }

            //Remove x4's parent connection to x3
            if (&node == bottomT1->parent)
            {
                if (node.leftChild == bottomT1)
                {
                    node.leftChild = nullptr;
                }
                else if (node.rightChild == bottomT1)
                {
                    node.rightChild = nullptr;
                }
            }

            //Exclusive argument, if left then changes[0] is nullptr, if right then topT1->parent is nullptr;
            //Remove xn's parent connction to the last chain element
            if (&node == topT1->parent || &node == changes[0])

            {
                if (node.leftChild == topT1)
                {
                    node.leftChild = &chainBottomCutoffT1;
                }
                else if (node.rightChild == topT1)
                {
                    node.rightChild = &chainBottomCutoffT1;
                }
                if (changes[0] == nullptr)
                {
                    chainTopCutoffT1 = node;
                }
                else
                {
                    chainTopCutoffT1 = *changes[0];
                }
            }

            //Remove xn's parent connection to his parent
            if (&node == topT1)
            {
                graph::Node* topNodeinT1 = &node;
                changes[0] = (topNodeinT1->parent);
                topNodeinT1->parent = nullptr;
            }
        }

        //T2
        for (auto& node : chainWithTrees.second.back()->Nodes())
        {
            //Remove x3's parent connection to x4
            if (&node == bottomT2)
            {
                graph::Node* bottomNodeInT2 = &node;
                bottomNodeInT2->parent = &chainTopCutoffT2;
                chainBottomCutoffT2 = *bottomNodeInT2;
            }
            //Remove x4's parent connection to x3
            if (&node == bottomT2->parent)
            {
                if (node.leftChild == bottomT2)
                {
                    node.leftChild = nullptr;
                }
                else if (node.rightChild == bottomT2)
                {
                    node.rightChild = nullptr;
                }
            }

            //Exclusive argument, if left then changes[0] is nullptr, if right then topT1->parent is nullptr;
            //Remove xn's parent connction to the last chain element
            if (&node == topT2->parent || &node == changes[1])
            {
                if (node.leftChild == topT2)
                {
                    node.leftChild = &chainBottomCutoffT2;
                }
                else if (node.rightChild == topT2)
                {
                    node.rightChild = &chainBottomCutoffT2;
                }
                if (changes[1] == nullptr)
                {
                    chainTopCutoffT2 = node;
                }
                else
                {
                    chainTopCutoffT2 = *changes[1];
                }
            }
            //Remove xn's connection to his parent
            if (&node == topT2)
            {
                graph::Node* topNodeinT2 = &node;
                changes[1] = (topNodeinT2->parent);
                topNodeinT2->parent = nullptr;
            }
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

    //Find nodes that belong to chain
    graph::Node* bottomT1 = chainWithTrees.first[3].front();
    graph::Node* bottomT2 = chainWithTrees.first[3].back();
    graph::Node* topT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
    graph::Node* topT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

    //x3 to x4 and x4 to x3 for T1
    for (auto& node : chainWithTrees.second.front()->Nodes())
    {
        if(&node == bottomT1)
        {
            for (auto& node2 : chainWithTrees.second.front()->Nodes())
            {
                if (&node2 == chainWithTrees.first[4].front())
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
        if ( &node == changes[0])
        {
            for (auto& node2 : chainWithTrees.second.front()->Nodes())
            {
                node.parent = &node2;
                if (node2.leftChild == nullptr)
                {
                    node2.leftChild = &node;
                    changes[0] = nullptr;
                    break;
                }
                if (node2.rightChild == nullptr)
                {
                    node2.rightChild = &node;
                    changes[0] = nullptr;
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
                if (&node2 == chainWithTrees.first[4].back())
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
        if ( &node == changes[1])
        {
            for (auto& node2 : chainWithTrees.second.back()->Nodes())
            {
                node.parent = &node2;
                if (node2.leftChild == nullptr)
                {
                    node2.leftChild = &node;
                    changes[1] = nullptr;
                    break;
                }
                if (node2.rightChild == nullptr)
                {
                    node2.rightChild = &node;
                    changes[1] = nullptr;
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

                    //anti single vertex check for terminalT2
                    if (parentT1 != nullptr )
                    {
                        //Determine x3 for case 2 for chain def in T1
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

                                //Case 2 structure check
                                int case2SiblingIsOnLeftSideT2 = -1;
                                if (parentOfParentT2 != nullptr)
                                {
                                    if (parentOfParentT2->leftChild== siblingOfParentT2)
                                    {
                                        case2SiblingIsOnLeftSideT2 = 1;
                                    }
                                    else
                                    {
                                        case2SiblingIsOnLeftSideT2= 0;
                                    }
                                }

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
                                    std::vector<std::vector<graph::Node*>> chainT1T2 = {{terminalT1.first,terminalT2.first},
                                        {siblingT1,siblingT2},{parentT1,parentT2}};

                                    //Chain collection bool
                                    bool case1check = true;

                                    //Determine x4 for T1, T2
                                    graph::Node* case1T1Parent = chainT1T2.back().front()->parent;
                                    graph::Node* case1T2Parent = chainT1T2.back().back()->parent;

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
                                                //T2     structure c heck
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

                                    if (chainT1T2.size() >= 4)
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
                                //the same as the index for the parent of the parent on both trees.
                                else if(case2SiblingIsOnLeftSideT1 == case2SiblingIsOnLeftSideT2 &&
                                    case2SiblingIsOnLeftSideT1 != -1 && case2SiblingIsOnLeftSideT2 != -1 &&
                                  siblingT1->parent == parentOfParentT1 && siblingT2->parent == parentOfParentT2 &&
                                  not case1applied)
                                {
                                    //std::cout << "case 2" << std::endl;
                                    //Chain for both trees, x1-x3
                                    //Technically parent of x1 is ignored but, well, it gets carried in-between x1 and x3 so w/e lol

                                    std::vector<std::vector<graph::Node*>> chainT1T2 = {{terminalT1.first,terminalT2.first},
                                    {siblingOfParentT1,siblingOfParentT2},
                                    {parentOfParentT1,parentOfParentT2}};

                                    graph::Node* case2T1Parent = chainT1T2.back().front()->parent;
                                    graph::Node* case2T2Parent = chainT1T2.back().back()->parent;

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

                                    if (chainT1T2.size() >= 4)
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

