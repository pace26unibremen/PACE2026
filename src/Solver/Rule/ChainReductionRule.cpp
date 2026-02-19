//
// Created by Andre on 11.12.2025.
//

#include "ChainReductionRule.h"

#include "../Action/DeleteNodeActionInChains.h"
#include <iostream>
#include <algorithm>
#include <stack>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) : AbstractRule(instance,context)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = chainWithTrees;
    changes = std::stack<solver::DeleteNodeActionInChains>();

}

//Chain def:Let T be a rooted phylogenetic X-tree, and let C =
//(x1, x2, . . . , xn) be a sequence of elements in X with n ≥ 2.
//X:= Set of labels, bijectively used on leaves in the tree.

//We say that C is an n-chain (or short chain) of T
//if the parent of x1 coincides with the parent of x2
//or the parent of x2 is the parent of the parent of x1,
//and, for each i ∈ {3, 4, . . . , n},
//the parent of xi is the parent of the parent of xi−1.


int solver::ChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;
    //Fetch Nodes
    std::vector<graph::Node> treeOneNodes = chainWithTrees.second.front()->Nodes();
    std::vector<graph::Node> treeTwoNodes = chainWithTrees.second.back()->Nodes();

    //x4+
    //For n ≥ 4, let C = (x1, x2, . . . , xn) be a
    //maximal n-chain that is common to T and T' . Then set
    //S = T |X \ {x4, x5, . . . , xn} and S'= T'|X \ {x4, x5, . . . , xn}.
    for (int index = chainWithTrees.first.size()-1 ; index >= 3; index--)
    {   graph::Node* NodeInChainT1 = chainWithTrees.first.at(index).at(0);
        graph::Node* NodeInChainT2 = chainWithTrees.first.at(index).at(1);

        changes.emplace(NodeInChainT1,chainWithTrees.second.front());
        changes.top().doAction();
        changes.emplace(NodeInChainT2,chainWithTrees.second.back());
        changes.top().doAction();
    }

    return 0;
}

void solver::ChainReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
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
        std::cout << "T1:" << std::endl;
        T1->write(std::cout);

        for (const auto& T2 : *instance)
        {   //DEBUG
            std::cout << "T2:" << std::endl;
            T2->write(std::cout);

            if (T1 != T2)
            {
                //Fetch Leaves - Terminal Index to Label
                std::unordered_map<graph::Node*,unsigned int> termIndexTreeOne = T1->Terminals();
                std::unordered_map<graph::Node*,unsigned int> termIndexTreeTwo = T2->Terminals();

                //For all Terminals...
                for (const auto& terminalT1 : termIndexTreeOne)
                {  //T1
                    //DEBUG
                    std::cout << "terminalT1: " << terminalT1.second << std::endl;

                    //Determine parent
                    graph::Node* parentT1 = terminalT1.first->parent;

                    //Determine if sibling exists
                    graph::Node* siblingT1 = terminalT1.first->sibling;

                    // -> x1, x2 and x3 for case 1 for chain def known for T1


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
                        std::cout << "terminalT2: " << terminalT2.second << std::endl;
                        //Determine Structure
                        graph::Node* parentT2 = terminalT2.first->parent;
                        graph::Node* siblingT2 = terminalT2.first->sibling;

                        // -> x1,x2,x3 for case 1 known for T2

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
                        if (siblingT2 != nullptr && siblingT1 != nullptr &&
                           (siblingT1->parent == parentT1) && (siblingT2->parent == parentT2))
                        {
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
                            {   //Sibling is in fact a terminal
                                if (case1T1Sibling != nullptr && case1T2Sibling != nullptr
                                   && case1T1Parent != nullptr && case1T2Parent != nullptr
                                   && case1T1Sibling->leftChild == nullptr && case1T1Sibling->rightChild == nullptr
                                   && case1T2Sibling->leftChild == nullptr && case1T2Sibling->rightChild == nullptr)
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
                        else if (case2SiblingIsOnLeftSideT1 == case2SiblingIsOnLeftSideT2 &&
                            case2SiblingIsOnLeftSideT1 != -1 && case2SiblingIsOnLeftSideT2 != -1 &&
                          siblingT1->parent == parentOfParentT1 && siblingT2->parent == parentOfParentT2 &&
                          not case1applied)
                        {
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
                            {   //Siblings exist
                                if (case2T1Sibling != nullptr && case2T2Sibling != nullptr
                                    && case2T1Parent != nullptr && case2T2Parent != nullptr
                                    && case2T1Sibling->leftChild == nullptr && case2T1Sibling->rightChild == nullptr
                                    && case2T2Sibling->leftChild == nullptr && case2T2Sibling->rightChild == nullptr)
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

    return std::dynamic_pointer_cast<AbstractRule>(
                    std::make_shared<ChainReductionRule>(instance,chainWithTrees,context));

}


std::string solver::ChainReductionRule::name() const
{
    return "ChainReductionRule";
}

