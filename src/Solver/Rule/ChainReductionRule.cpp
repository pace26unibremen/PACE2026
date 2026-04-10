#include "ChainReductionRule.hpp"

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
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    //Repeat check for safety, latter >= 2 means that chain was indeed longer then 3 elements, as x1 and x2 are never
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


        //REMOVAL OF EDGES

        //Remove the edge between x3 and x4, replace with edge between x3 and topChainT1Parent
        bottomT1->parent = topChainT1Parent;
        bottomT2->parent = topChainT2Parent;

        //Create edge between x3 and xn's parent
        // topChainT1Parent->leftChild == topChainT1 ? topChainT1Parent->leftChild = bottomT1 : topChainT1Parent->rightChild
        // = bottomT1;

        if (topChainT1Parent->leftChild == topChainT1)
        {
            topChainT1Parent->leftChild = bottomT1;
            topChainT1Parent->rightChild->sibling = bottomT1;
            bottomT1->sibling = topChainT1Parent->rightChild;
        }
        else
        {
            topChainT1Parent->rightChild = bottomT1;
            topChainT1Parent->leftChild->sibling = bottomT1;
            bottomT1->sibling = topChainT1Parent->leftChild;
        }

        if (topChainT2Parent->leftChild == topChainT2)
        {
            topChainT2Parent->leftChild = bottomT2;
            topChainT2Parent->rightChild->sibling = bottomT2;
            bottomT2->sibling = topChainT2Parent->rightChild;
        }
        else
        {
            topChainT2Parent->rightChild = bottomT2;
            topChainT2Parent->leftChild->sibling = bottomT2;
            bottomT2->sibling = topChainT2Parent->leftChild;
        }
        // topChainT2Parent->leftChild == topChainT2 ? topChainT2Parent->leftChild = bottomT2 :
        // topChainT2Parent->rightChild = bottomT2;

        //Update x4 to reflect x3 change
        bottomChainT1->leftChild == bottomT1 ? bottomChainT1->leftChild = nullptr : bottomChainT1->rightChild = nullptr;

        bottomChainT2->leftChild == bottomT1 ? bottomChainT2->leftChild = nullptr : bottomChainT2->rightChild = nullptr;

        //Remove edge between xn and its parent and update parent accordingly with x3 being it's new child.
        topChainT1->parent = nullptr;
        topChainT2->parent = nullptr;


        //REMOVAL OF SIBLING RELATIONSHIPS

        //Remove sibling connection between xn and xn's parents terminal
        topChainT1->sibling->sibling = nullptr;
        topChainT1->sibling = nullptr;

        topChainT2->sibling->sibling = nullptr;
        topChainT2->sibling = nullptr;

        //Remove sibling connection between x3's parent and x4
        if (bottomChainT1->leftChild == nullptr)
        {
            bottomChainT1->rightChild->sibling->sibling = nullptr;
            bottomChainT1->rightChild->sibling = nullptr;
            // bottomT1->sibling = topChainT1Parent->rightChild;
            // topChainT1Parent->rightChild->sibling = bottomT1;
        }
        else
        {
            bottomChainT1->leftChild->sibling->sibling = nullptr;
            bottomChainT1->leftChild->sibling = nullptr;
            // bottomT1->sibling = topChainT1Parent->leftChild;
            // topChainT1Parent->leftChild->sibling = bottomT1;
        }

        if (bottomChainT2->leftChild == nullptr)
        {
            bottomChainT2->rightChild->sibling->sibling = nullptr;
            bottomChainT2->rightChild->sibling = nullptr;
            // bottomT2->sibling = topChainT2Parent->rightChild;
            // topChainT2Parent->rightChild->sibling = bottomT2;
        }
        else
        {
            bottomChainT2->leftChild->sibling->sibling = nullptr;
            bottomChainT2->leftChild->sibling = nullptr;
            // bottomT2->sibling = topChainT2Parent->leftChild;
            // topChainT2Parent->leftChild->sibling = bottomT2;
        }
        //Now: x4 to xn isolated from x1-x3 and xn's parent to root by both edges and sibling-relationship.
        //What remains now is to remove the edges between x4 to xn and their parents, turning them to single vertex trees,
        //And to remove the parent nodes out of the trees entirely...

        for (int i = chainWithTrees.first.size()-1; i > 0; i--)
        {
            //Every element is a parent -> Terminal
            // If the left side is the parent of the child
            //For T1
            if (i != 1)
            {
                if (chainWithTrees.first[i].front()->leftChild == chainWithTrees.first[i-1].front())
                {
                    // std::cout << "Case 1, if 1" << std::endl;
                    //->Right side is the terminal
                    graph::Node* terminal = chainWithTrees.first[i].front()->rightChild;

                    //Remove the edge between these two on the side of the terminal
                    terminal->parent = nullptr;


                    // //Delete the parent node out of the list of Nodes -> Address still in ChainWithTrees for unapply.
                    // auto parentPos = std::find(chainWithTrees.second.front()->Nodes().begin(),
                    //         chainWithTrees.second.front()->Nodes().end(), test);
                    // chainWithTrees.second.front()->Nodes().erase(parentPos);

                    //Remove the sibling connection
                    terminal->sibling->sibling = nullptr;
                    terminal->sibling = nullptr;

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.front()->Roots().emplace_back(terminal);
                }
                else if (chainWithTrees.first[i].front()->rightChild == chainWithTrees.first[i-1].front())
                {
                    // std::cout << "Case 1, if 2" << std::endl;
                    //->Left side is the terminal
                    graph::Node* terminal = chainWithTrees.first[i].front()->leftChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    // chainWithTrees.first[i].front()->leftChild = nullptr;
                    //Remove parent out of Nodes in T1
                    // auto parentPos = std::find(chainWithTrees.second.front()->Roots().begin(),
                    //         chainWithTrees.second.front()->Roots().end(), chainWithTrees.first[i].front());
                    // chainWithTrees.second.front()->Roots().erase(parentPos);

                    //Remove the sibling connection
                    terminal->sibling->sibling = nullptr;
                    terminal->sibling = nullptr;

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.front()->Roots().emplace_back(terminal);
                }

                //For T2
                if (chainWithTrees.first[i].back()->leftChild == chainWithTrees.first[i-1].back())
                {
                    // std::cout << "Case 2, if 1" << std::endl;
                    //->Right side is the terminal
                    graph::Node* terminal = chainWithTrees.first[i].back()->rightChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    //Remove parent out of Nodes in T2
                    // chainWithTrees.first[i].back()->rightChild = nullptr;
                    // auto parentPos = std::find(chainWithTrees.second.back()->Roots().begin(),
                    //         chainWithTrees.second.back()->Roots().end(), chainWithTrees.first[i].back());
                    // chainWithTrees.second.back()->Roots().erase(parentPos);

                    //Remove the sibling connection
                    terminal->sibling->sibling = nullptr;
                    terminal->sibling = nullptr;

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.back()->Roots().emplace_back(terminal);
                }
                else if (chainWithTrees.first[i].back()->rightChild == chainWithTrees.first[i-1].back())
                {
                    // std::cout << "Case 2, if 2" << std::endl;
                    //->Left side is the terminal
                    graph::Node* terminal = chainWithTrees.first[i].back()->leftChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    //Remove parent out of Nodes in T2
                    // chainWithTrees.first[i].back()->leftChild = nullptr;
                    // auto parentPos = std::find(chainWithTrees.second.back()->Roots().begin(),
                    //         chainWithTrees.second.back()->Roots().end(), chainWithTrees.first[i].back());
                    // chainWithTrees.second.back()->Roots().erase(parentPos);

                    //Remove the sibling connection
                    terminal->sibling->sibling = nullptr;
                    terminal->sibling = nullptr;

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.back()->Roots().emplace_back(terminal);
                }
            }
            else
            {
                //T1, i = 1
                if (chainWithTrees.first[i].front()->leftChild == nullptr)
                {
                    graph::Node* terminal = chainWithTrees.first[i].front()->rightChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    //Remove parent out of Nodes in T1
                    // chainWithTrees.first[i].front()->rightChild = nullptr;
                    // auto parentPos = std::find(chainWithTrees.second.front()->Roots().begin(),
                    //         chainWithTrees.second.front()->Roots().end(), chainWithTrees.first[i].front());
                    // chainWithTrees.second.front()->Roots().erase(parentPos);

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.front()->Roots().emplace_back(terminal);
                }
                else
                {
                    graph::Node* terminal = chainWithTrees.first[i].front()->leftChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    //Remove parent out of Nodes in T1
                    // chainWithTrees.first[i].front()->leftChild = nullptr;
                    // auto parentPos = std::find(chainWithTrees.second.front()->Roots().begin(),
                    //         chainWithTrees.second.front()->Roots().end(), chainWithTrees.first[i].front());
                    // chainWithTrees.second.front()->Roots().erase(parentPos);


                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.front()->Roots().emplace_back(terminal);
                }
                //T2, i = 1 -> x4
                if (chainWithTrees.first[i].back()->leftChild == nullptr)
                {
                    graph::Node* terminal = chainWithTrees.first[i].back()->rightChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    //Remove parent out of Nodes in T2
                    // chainWithTrees.first[i].back()->rightChild = nullptr;
                    // auto parentPos = std::find(chainWithTrees.second.back()->Roots().begin(),
                    //         chainWithTrees.second.back()->Roots().end(), chainWithTrees.first[i].back());
                    // chainWithTrees.second.back()->Roots().erase(parentPos);

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.back()->Roots().emplace_back(terminal);
                }
                else
                {
                    graph::Node* terminal = chainWithTrees.first[i].back()->leftChild;

                    //Remove the edge between these two
                    terminal->parent = nullptr;

                    //Remove parent out of Nodes in T2
                    // chainWithTrees.first[i].back()->leftChild = nullptr;
                    // auto parentPos = std::find(chainWithTrees.second.back()->Roots().begin(),
                    //         chainWithTrees.second.back()->Roots().end(), chainWithTrees.first[i].back());
                    // chainWithTrees.second.back()->Roots().erase(parentPos);

                    //Add it as a root node, as it's a single vertex tree
                    chainWithTrees.second.back()->Roots().emplace_back(terminal);
                }
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
    graph::Node* bottomT1 = chainWithTrees.first[0].front();
    graph::Node* bottomT2 = chainWithTrees.first[0].back();

    //x3 to x4 and x4 to x3 for T1
    for (auto& node : chainWithTrees.second.front()->Nodes())
    {
        if(&node == bottomT1)
        {
            for (auto& node2 : chainWithTrees.second.front()->Nodes())
            {
                if (&node2 == chainWithTrees.first[1].front())
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
                if (&node2 == chainWithTrees.first[1].back())
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

    if (instance->size() == 2 && instance->front() != instance->back())
    {
        std::shared_ptr<graph::Forest> T1 = instance->front();
        std::shared_ptr<graph::Forest> T2 = instance->back();

        //Fetch Leaves - Terminal Index to Label
        std::unordered_map<graph::Node*,unsigned int> termIndexTreeOne = T1->TerminalToLabel();
        std::unordered_map<graph::Node*,unsigned int> termIndexTreeTwo = T2->TerminalToLabel();

        //For all Terminals...
        for (auto& terminalT1 : termIndexTreeOne)
        {   //T1

            //Determine parent
            graph::Node* parentT1 = terminalT1.first->parent;

            // -> x1, x2 for case 1 for chain def known for T1

            //anti single vertex check for terminalT1
            if (parentT1 != nullptr )
            {
                //Determine x3 and x2 for case 1 for chain in T1
                graph::Node* parentOfParentT1 = parentT1->parent;

                if (not chain.empty())
                {
                    break;
                }

                for (auto& terminalT2 : termIndexTreeTwo)
                {   //T2
                    //std::cout << "terminalT2: " << terminalT2.second << std::endl;
                    //Determine Structure
                    graph::Node* parentT2 = terminalT2.first->parent;

                    // -> x1,x2,x3 for case 1 known for T2

                    //Anti single vertex check for terminalT2
                    if (parentT2 != nullptr)
                    {
                        //Determine x3 for case 2 for chain def in T2
                        graph::Node* parentOfParentT2 = parentT2->parent;

                        //Case 1: if the parent of x1 coincides with the parent of x2 for both trees T1 and T2
                        if (terminalT1.first->parent->leftChild != nullptr
                        && terminalT1.first->parent->rightChild != nullptr
                        && T1->TerminalToLabel().contains(terminalT1.first->parent->leftChild)
                        && T1->TerminalToLabel().contains(terminalT1.first->parent->rightChild)

                        && terminalT2.first->parent->leftChild != nullptr
                        && terminalT2.first->parent->rightChild != nullptr
                        && T2->TerminalToLabel().contains(terminalT2.first->parent->leftChild)
                        && T2->TerminalToLabel().contains(terminalT2.first->parent->rightChild)

                        && terminalT1.first->parent->parent != nullptr && terminalT2.first->parent->parent != nullptr

                        && (terminalT1.first->parent->parent->leftChild == terminalT1.first->parent
                        || terminalT1.first->parent->parent->rightChild == terminalT1.first->parent)
                        && (T1->TerminalToLabel().contains(terminalT1.first->parent->parent->leftChild)
                        || T1->TerminalToLabel().contains(terminalT1.first->parent->parent->rightChild))

                        && (terminalT2.first->parent->parent->leftChild == terminalT2.first->parent
                        || terminalT2.first->parent->parent->rightChild == terminalT2.first->parent)
                        && (T2->TerminalToLabel().contains(terminalT2.first->parent->parent->leftChild)
                        || T2->TerminalToLabel().contains(terminalT2.first->parent->parent->rightChild))

                        )
                        {
                            //Chain for both trees
                            std::vector<std::vector<graph::Node*>> chainT1T2 = {};

                            //Chain collection bool
                            bool case1check = true;

                            //Determine x4 for T1, T2
                            graph::Node* case1T1Parent = parentT1->parent;
                            graph::Node* case1T2Parent = parentT2->parent;

                            //Determine Sibling of x3
                            graph::Node* case1T1Sibling;
                            graph::Node* case1T2Sibling;

                            //T1 structure check, determining sibling of x3 -> Supposed terminal
                            if (case1T1Parent->leftChild == parentT1)
                            {
                                case1T1Sibling = case1T1Parent->rightChild;
                            }
                            else
                            {
                                case1T1Sibling = case1T1Parent->leftChild;
                            }
                            //T2 structure c heck
                            if (case1T2Parent->leftChild == parentT2)
                            {
                                case1T2Sibling = case1T2Parent->rightChild;
                            }
                            else
                            {
                                case1T2Sibling = case1T2Parent->leftChild;
                            }

                            //Determine x4,... of chain
                            while (case1check)
                            {
                                //Sibling is in fact a terminal...
                                if (case1T1Sibling != nullptr && case1T2Sibling != nullptr
                                && case1T1Parent != nullptr && case1T2Parent != nullptr

                                && case1T1Sibling->leftChild == nullptr && case1T1Sibling->rightChild == nullptr
                                && case1T2Sibling->leftChild == nullptr && case1T2Sibling->rightChild == nullptr

                                //and is not attached to root
                                && std::find(T1->Roots().begin(), T1->Roots().end(), case1T1Parent)
                                == T1->Roots().end()
                                && std::find(T2->Roots().begin(), T2->Roots().end(), case1T2Parent)
                                == T2->Roots().end()

                                //and the current looked at parent isn't a or the root node of the forest
                                && case1T1Parent != *T1->Roots().end()
                                && case1T2Parent != *T2->Roots().end()
                                )
                                {
                                    //Push the identified chain element into the chain list
                                    chainT1T2.push_back({case1T1Parent,case1T2Parent});

                                    //Shift to next parent above the now newest chain element
                                    case1T1Parent = chainT1T2.back().front()->parent;
                                    case1T2Parent = chainT1T2.back().back()->parent;

                                    //Update sibling to that of the new parent node to be looked at
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


                                        if (case1T2Parent->leftChild == chainT1T2.back().back())
                                        {
                                            case1T2Sibling = case1T2Parent->rightChild;
                                        }
                                        else
                                        {
                                            case1T2Sibling = case1T2Parent->leftChild;
                                        }
                                    }
                                    else //Security false setting in case
                                    {
                                        case1check = false;
                                    }
                                }
                                // We're either at the root node or the chain ends
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
                                //Create the connected pair between chain and trees
                                chain = chainT1T2;
                                chainTrees.emplace_back(T1);
                                chainTrees.emplace_back(T2);
                                chainWithTrees.first = chain;
                                chainWithTrees.second = chainTrees;
                                return std::dynamic_pointer_cast<AbstractRule>(
                                std::make_shared<ChainReductionRule>(instance,chainWithTrees,context));
                            }

                        }
                    }
                    if (not chain.empty()) break;
                }
            }
            if (not chain.empty())
            {
                break;
            }
        }
    }

    //When no chain is found
    return nullptr;
}
std::string solver::ChainReductionRule::name() const
{
    return "ChainReductionRule";
}

