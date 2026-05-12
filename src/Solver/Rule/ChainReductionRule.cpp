#include "ChainReductionRule.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>
solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) :
        AbstractRule(instance, context,true)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = std::move(chainWithTrees);
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

bool solver::ChainReductionRule::isNodeInNodeVector(const graph::Node* node, const std::vector<graph::Node*>& list)
{
    bool check = false;
    for(int i = 0; i < list.size(); i++)
    {
       if (node == list[i])
       {
           check = true;
           break;
       }
    }
    return check;
}
void solver::ChainReductionRule::storeNodeIndices(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest)
{
    std::vector<int> indicesList;
    indicesList.reserve(4);

    int nodePos = 0;
    for (int i = 0; i < forest->Nodes().capacity(); i++)
    {
        if (&forest->Nodes()[i] == node)
        {
            nodePos = i;
            break;
        }
    }

    int parentPos = -1;
    int siblingPos = -1;
    int leftPos = -1;
    int rightPos = -1;

    for (int i = 0; i < forest->Nodes().capacity(); i++)
    {
        if (&forest->Nodes()[i] == node->parent)
        {
            parentPos = i;
        }
    }
    for (int i = 0; i < forest->Nodes().capacity(); i++)
    {
        if (&forest->Nodes()[i] == node->sibling)
        {
            siblingPos = i;
        }
    }
    for (int i = 0; i < forest->Nodes().capacity(); i++)
    {
        if (&forest->Nodes()[i] == node->leftChild)
        {
            leftPos = i;
        }
    }
    for (int i = 0; i < forest->Nodes().capacity(); i++)
    {
        if (&forest->Nodes()[i] == node->rightChild)
        {
            rightPos = i;
        }
    }

    indicesList = {parentPos, siblingPos, leftPos, rightPos};
    if (forest == chainWithTrees.second.front())
    {
        deletedNodesT1Indices.at(nodePos) = indicesList;
    }
    else
    {
        deletedNodesT2Indices.at(nodePos) = indicesList;
    }
}

void solver::ChainReductionRule::storeNode(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest)
{
    if (forest == chainWithTrees.second.front())
    {
        for (int index = 0; index < forest->Nodes().capacity(); index++)
        {
            if (&forest->Nodes()[index] == node)
            {
                deletedNodesT1[index].leftChild = node->leftChild;
                deletedNodesT1[index].rightChild = node->rightChild;
                deletedNodesT1[index].parent = node->parent;
                deletedNodesT1[index].sibling = node->sibling;
                break;
            }
        }
    }
    else if (forest == chainWithTrees.second.back())
    {
        for (int index = 0; index < forest->Nodes().capacity(); index++)
        {
            if (&forest->Nodes()[index] == node)
            {
                deletedNodesT2[index].leftChild = node->leftChild;
                deletedNodesT2[index].rightChild = node->rightChild;
                deletedNodesT2[index].parent = node->parent;
                deletedNodesT2[index].sibling = node->sibling;
                break;
            }
        }
    }
    else
    {
        throw std::invalid_argument("ChainReductionRule : storeNode : forest is not within chainWithTrees");
    }
}

void solver::ChainReductionRule::removeConnectionOfTerminalNode(graph::Node* node, std::shared_ptr<graph::Forest>& forest)
{
    //If left child is terminal
    if (node->leftChild->leftChild == nullptr && node->leftChild->rightChild == nullptr)
    {
        //alter terminal to be alone
        graph::Node* terminal = node->leftChild;
        storeNodeIndices(terminal, forest);

        terminal->parent = nullptr;
        if (terminal->sibling != nullptr)
        {
            terminal->sibling->sibling = nullptr;

        }
        terminal->sibling = nullptr;
        //->Single vertex terminal

        //now alter param node
        if (node->rightChild != nullptr) node->rightChild->parent = nullptr;
        node->rightChild = nullptr;

        node->leftChild = nullptr;

        node->parent = nullptr;

        if (node->sibling != nullptr)
        {
            node->sibling->sibling = nullptr;
        }
        node->sibling = nullptr;
    }
    //else if right child is terminal
    else if (node->rightChild->leftChild == nullptr && node->rightChild->rightChild == nullptr)
    {
        //alter terminal to be alone
        graph::Node* terminal = node->rightChild;
        storeNodeIndices(terminal,forest);

        terminal->parent = nullptr;
        if (terminal->sibling != nullptr)
        {
            terminal->sibling->sibling = nullptr;

        }
        terminal->sibling = nullptr;

        //now alter param node
        if (node->leftChild != nullptr) node->leftChild->parent = nullptr;
        node->leftChild = nullptr;

        node->rightChild = nullptr;

        node->parent = nullptr;

        if (node->sibling != nullptr)
        {
            node->sibling->sibling = nullptr;
        }
        node->sibling = nullptr;
    }
    else
    {
        throw std::invalid_argument("ChainReductionRule : removeConnectionOfTerminalNode : "
                                    "node parameter did not have a terminal as a child node");
    }
}



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

    //Set the allowed memory of the set of to be deleted Nodes
    deletedNodesT1.resize(chainWithTrees.second.front()->Nodes().capacity());
    deletedNodesT2.resize(chainWithTrees.second.back()->Nodes().capacity());
    deletedNodesT1Indices.resize(chainWithTrees.second.front()->Nodes().capacity());
    deletedNodesT2Indices.resize(chainWithTrees.second.back()->Nodes().capacity());

    //Repeat check for safety, latter >= 2 means that chain was indeed longer then 3 elements, as x1 and x2 are never
    //stored due to being irrelevant
    if (chainWithTrees.second.size() == 2 && chainWithTrees.first.size() >= 2)
    {
        //Store all node connections by their index.
        for (int index = 0; index < chainWithTrees.second.front()->Nodes().capacity(); index++)
        {
            storeNodeIndices(&chainWithTrees.second.front()->Nodes()[index], chainWithTrees.second.front());

        }
        for (int index = 0; index < chainWithTrees.second.back()->Nodes().capacity(); index++)
        {
            storeNodeIndices(&chainWithTrees.second.back()->Nodes()[index], chainWithTrees.second.back());
        }
        // Acquire the notes required
        // Fetch x3
        graph::Node* bottomT1 = chainWithTrees.first[0].front();
        graph::Node* bottomT2 = chainWithTrees.first[0].back();

        //Fetch xn's parent out of tree.
        graph::Node* topChainT1Parent = chainWithTrees.first[chainWithTrees.first.size()-1].front()->parent;
        graph::Node* topChainT2Parent = chainWithTrees.first[chainWithTrees.first.size()-1].back()->parent;

        //Address the now seperated chain in T1 and T2 through the removal of the parent nodes of the terminals as well
        //as setting the terminals to be single tree vertices.

        for (int i = chainWithTrees.first.size()-1; i > 0; i--)
        {
            //Every element of chainWithTrees.first is a parent of a terminal. Index cycles through each.
            removeConnectionOfTerminalNode(chainWithTrees.first[i].front(), chainWithTrees.second.front());
            removeConnectionOfTerminalNode(chainWithTrees.first[i].back(), chainWithTrees.second.back());
        }

        //Connect the two parts of the now seperated trees
        bottomT1->parent = topChainT1Parent;
        bottomT2->parent = topChainT2Parent;

        //Connect the sibling connections between top's terminal and bottom node parts
        //Due to removeConnectionOfTerminalNode: The top Chain node side's most upper node has no parent ref
        //-> Other side still has it.
        if (topChainT1Parent->leftChild->parent == topChainT1Parent)
        {
            bottomT1->sibling = topChainT1Parent->leftChild;
            topChainT1Parent->rightChild = bottomT1;
            topChainT1Parent->leftChild->sibling = bottomT1;
        }
        else if (topChainT1Parent->rightChild->parent == topChainT1Parent)
        {
            bottomT1->sibling = topChainT1Parent->rightChild;
            topChainT1Parent->leftChild = bottomT1;
            topChainT1Parent->rightChild->sibling = bottomT1;
        }

        if (topChainT2Parent->leftChild->parent == topChainT2Parent)
        {
            bottomT2->sibling = topChainT2Parent->leftChild;
            topChainT2Parent->rightChild = bottomT2;
            topChainT2Parent->leftChild->sibling = bottomT2;
        }
        else if (topChainT2Parent->rightChild->parent == topChainT2Parent)
        {
            bottomT2->sibling = topChainT2Parent->rightChild;
            topChainT2Parent->leftChild = bottomT2;
            topChainT2Parent->rightChild->sibling = bottomT2;
        }
    }
    return RuleReturnCode::Continue;
}

void solver::ChainReductionRule::unapply()
{
    std::vector<graph::Node>& nodesT1 = chainWithTrees.second.front()->Nodes();
    std::vector<graph::Node>& nodesT2 = chainWithTrees.second.back()->Nodes();

    if (not this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    for (int index = 0; index < deletedNodesT1Indices.capacity(); index++)
    {
        if (deletedNodesT1Indices[index].empty() == false)
        {
            //parent
            if (deletedNodesT1Indices[index][0] != -1)
            {
                nodesT1[index].parent = &nodesT1.at(deletedNodesT1Indices[index][0]);
            }
            //sibling
            if (deletedNodesT1Indices[index][1] != -1)
            {
                nodesT1[index].sibling = &nodesT1.at(deletedNodesT1Indices[index][1]);
            }
            //left
            if (deletedNodesT1Indices[index][2] != -1)
            {
                nodesT1[index].leftChild = &nodesT1.at(deletedNodesT1Indices[index][2]);
            }
            //right
            if (deletedNodesT1Indices[index][3] != -1)
            {
                nodesT1[index].rightChild = &nodesT1.at(deletedNodesT1Indices[index][3]);
            }
        }
    }

    for (int index = 0; index < deletedNodesT2Indices.capacity(); index++)
    {
        if (deletedNodesT2Indices[index].empty() == false)
        {
            //parent
            if (deletedNodesT2Indices[index][0] != -1)
            {
                nodesT2[index].parent = &nodesT2.at(deletedNodesT2Indices[index][0]);
            }
            //sibling
            if (deletedNodesT2Indices[index][1] != -1)
            {
                nodesT2[index].sibling = &nodesT2.at(deletedNodesT2Indices[index][1]);
            }
            //left
            if (deletedNodesT2Indices[index][2] != -1)
            {
                nodesT2[index].leftChild = &nodesT2.at(deletedNodesT2Indices[index][2]);
            }
            //right
            if (deletedNodesT2Indices[index][3] != -1)
            {
                nodesT2[index].rightChild = &nodesT2.at(deletedNodesT2Indices[index][3]);
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

                        && terminalT1.first->parent->parent != nullptr
                        && terminalT2.first->parent->parent != nullptr

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
                            std::vector<std::vector<graph::Node*>> chainT1T2 = {{parentT1,parentT2}};

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

                                //and the current parent node isn't a root node
                                && not isNodeInNodeVector(case1T1Parent, T1->Roots())
                                && not isNodeInNodeVector(case1T2Parent, T2->Roots())

                                // && std::find(T1->Roots().begin(), T1->Roots().end(), case1T1Parent)
                                // == T1->Roots().end()
                                // && std::find(T2->Roots().begin(), T2->Roots().end(), case1T2Parent)
                                // == T2->Roots().end()
                                //
                                // //and the current looked at parent isn't a or the root node of the forest
                                // && case1T1Parent != *T1->Roots().end()
                                // && case1T2Parent != *T2->Roots().end()
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
                                //Siblings and Parent exist
                                if (case2T1Sibling != nullptr && case2T2Sibling != nullptr
                                && case2T1Parent != nullptr && case2T2Parent != nullptr
                                //Sibling (Terminal) is a terminal
                                && case2T1Sibling->leftChild == nullptr && case2T1Sibling->rightChild == nullptr
                                && case2T2Sibling->leftChild == nullptr && case2T2Sibling->rightChild == nullptr
                                //Current parent node isnt a root node
                                && not isNodeInNodeVector(case2T1Parent, T1->Roots())
                                && not isNodeInNodeVector(case2T2Parent, T2->Roots())
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

