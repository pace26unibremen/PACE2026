#include "ChainReductionRule.hpp"

#include <algorithm>
#include <forward_list>
#include <unordered_map>
#include <utility>
#include <iostream>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) :
        AbstractRule(instance, context,true)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = std::move(chainWithTrees);
    rootsT1Indices = {};
    rootsT2Indices = {};

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

void solver::ChainReductionRule::storeRootNodes(const std::shared_ptr<graph::Forest>& forest)
{
    std::vector<int> indicesList;
    indicesList.reserve(2);

    int leftPos = -1;
    int rightPos = -1;

    for (int i = 0; i < forest->Roots().size(); i++)
    {
        if (forest->Roots()[i] != nullptr)
        {
            for (int j = 0; j < forest->Nodes().size(); j++)
            {
                if (&forest->Nodes()[j] == forest->Roots()[i]->leftChild)
                    leftPos = j;
                if (&forest->Nodes()[j] == forest->Roots()[i]->rightChild)
                    rightPos = j;
            }
            indicesList = {leftPos, rightPos};

            if (forest == chainWithTrees.second.front())
            {
                rootsT1Indices.first.push_back(indicesList);
                rootsT1Indices.second.push_back(forest->Roots()[i]->subtreeTerminals);
            }
            if (forest == chainWithTrees.second.back())
            {
                rootsT2Indices.first.push_back(indicesList);
                rootsT2Indices.second.push_back(forest->Roots()[i]->subtreeTerminals);
            }
        }
        else break;
    }
}

void solver::ChainReductionRule::storeEditedNodeIndices(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest)
{
    auto sizeOfNodes = forest->Nodes().size();

    std::vector<int> indicesList;
    indicesList.reserve(4);

    int nodePos = 0;
    for (int i = 0; i < sizeOfNodes; i++)
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

    for (int i = 0; i < sizeOfNodes; i++)
    {
        if (&forest->Nodes()[i] == node->parent)
        {
            parentPos = i;
        }
    }
    for (int i = 0; i < sizeOfNodes; i++)
    {if (&forest->Nodes()[i] == node->sibling)
        {
            siblingPos = i;
        }
    }
    for (int i = 0; i < sizeOfNodes; i++)
    {
        if (&forest->Nodes()[i] == node->leftChild)
        {
            leftPos = i;
        }
    }
    for (int i = 0; i < sizeOfNodes; i++)
    {
        if (&forest->Nodes()[i] == node->rightChild)
        {
            rightPos = i;
        }
    }

    indicesList = {parentPos, siblingPos, leftPos, rightPos};
    if (forest == chainWithTrees.second.front())
    {
        deletedNodesT1Indices.first.at(nodePos) = indicesList;
        deletedNodesT1Indices.second.at(nodePos) = forest->Nodes()[nodePos].subtreeTerminals;
    }
    else
    {
        deletedNodesT2Indices.first.at(nodePos) = indicesList;
        deletedNodesT2Indices.second.at(nodePos) = forest->Nodes()[nodePos].subtreeTerminals;
    }

}

void solver::ChainReductionRule::removeConnectionOfTerminalNode(graph::Node* node, std::shared_ptr<graph::Forest>& forest)
{
    //If left child is terminal
    if (node->leftChild->leftChild == nullptr && node->leftChild->rightChild == nullptr)
    {
        //alter terminal to be alone
        graph::Node* terminal = node->leftChild;
        storeEditedNodeIndices(terminal, forest);

        terminal->parent = nullptr;
        if (terminal->sibling != nullptr)
        {
            terminal->sibling->sibling = nullptr;

        }
        terminal->sibling = nullptr;
        //->Single vertex terminal
        forest->Roots().push_back(terminal);

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
        storeEditedNodeIndices(terminal,forest);

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

std::vector<uint64_t> solver::ChainReductionRule::eraseTerminals(std::vector<uint64_t> target, std::vector<uint64_t> toBeErased)
{
    for (int i = 0; i < target.size(); i++)
    {
        if (target[i] == toBeErased[i] && target[i] != 0)
        {
            target[i] = 0;
        }
    }
    return target;
}

void solver::ChainReductionRule::updateSubtreeTerminals()
{
    //x3
    graph::Node* t1BeginNode = chainWithTrees.first[0].front();
    graph::Node* t2BeginNode = chainWithTrees.first[0].back();

    //xn
    graph::Node* t1EndNode = chainWithTrees.first[chainWithTrees.first.size()-1].front();
    graph::Node* t2EndNode = chainWithTrees.first[chainWithTrees.first.size()-1].back();

    //Determine the sum of the subtree terminal numbers that are part of the chain.
    std::vector<uint64_t> subtreeTerminalsT1 = t1EndNode->subtreeTerminals;
    std::vector<uint64_t> subtreeTerminalsT2 = t2EndNode->subtreeTerminals;
    //Delete the terminal markers that are in x3, as to not remove them.
    subtreeTerminalsT1 = eraseTerminals(subtreeTerminalsT1, t1BeginNode->subtreeTerminals);
    subtreeTerminalsT2 = eraseTerminals(subtreeTerminalsT2, t2BeginNode->subtreeTerminals);

    //Nodes to be reached until root
    graph::Node* parentOfEndNodeT1 = t1EndNode->parent;
    graph::Node* parentOfEndNodeT2 = t2EndNode->parent;

    while (parentOfEndNodeT1->parent != nullptr && parentOfEndNodeT1->sibling != nullptr)
    {
        parentOfEndNodeT1->subtreeTerminals =
            eraseTerminals(parentOfEndNodeT1->subtreeTerminals, subtreeTerminalsT1);

        parentOfEndNodeT1 = parentOfEndNodeT1->parent;
    }
    //Ends when parendOfEndNodeT1 is root of t1 -> apply once more
    parentOfEndNodeT1->subtreeTerminals =
        eraseTerminals(parentOfEndNodeT1->subtreeTerminals, subtreeTerminalsT1);

    while (parentOfEndNodeT2->parent != nullptr && parentOfEndNodeT2->sibling != nullptr)
    {
        parentOfEndNodeT2->subtreeTerminals =
            eraseTerminals(parentOfEndNodeT2->subtreeTerminals, subtreeTerminalsT2);

        parentOfEndNodeT2 = parentOfEndNodeT2->parent;
    }
    //Ends when parendOfEndNodeT1 is root of t2 -> apply once more
    parentOfEndNodeT2->subtreeTerminals =
        eraseTerminals(parentOfEndNodeT2->subtreeTerminals, subtreeTerminalsT2);

    //-> for all xn to root, the subtreeTerminal bitmask has been updated.
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
    //Respectively, this needs to happen to the other side of the edge, i.e. for x4 it must be done on x3,
    //and on xn+1 for xn, as it's two-sided.
    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    //Set the allowed memory of the set of to be deleted Nodes
    //First for all nodes
    deletedNodesT1Indices.first.resize(chainWithTrees.second.front()->Nodes().capacity());
    deletedNodesT2Indices.first.resize(chainWithTrees.second.back()->Nodes().capacity());
    //Then for all possible subtreeTerminals
    deletedNodesT1Indices.second.resize(chainWithTrees.second.front()->Nodes().capacity());
    deletedNodesT2Indices.second.resize(chainWithTrees.second.back()->Nodes().capacity());

    //Repeat check for safety, latter >= 2 means that chain was indeed longer then 3 elements, as x1 and x2 are never
    //stored due to being irrelevant
    if (chainWithTrees.second.size() == 2 && chainWithTrees.first.size() >= 2)
    {
        //Store all node connections by their index.
        for (int index = 0; index < chainWithTrees.second.front()->Nodes().size(); index++)
        {
            storeEditedNodeIndices(&chainWithTrees.second.front()->Nodes()[index], chainWithTrees.second.front());

        }

        for (int index = 0; index < chainWithTrees.second.back()->Nodes().size(); index++)
        {
            storeEditedNodeIndices(&chainWithTrees.second.back()->Nodes()[index], chainWithTrees.second.back());
        }

        //Store current Root Nodes
        storeRootNodes(chainWithTrees.second.front());
        storeRootNodes(chainWithTrees.second.back());

        // Acquire the notes required
        // Fetch x3
        graph::Node* bottomT1 = chainWithTrees.first[0].front();
        graph::Node* bottomT2 = chainWithTrees.first[0].back();

        //Fetch xn's parent out of tree.
        graph::Node* topChainT1Parent = chainWithTrees.first[chainWithTrees.first.size()-1].front()->parent;
        graph::Node* topChainT2Parent = chainWithTrees.first[chainWithTrees.first.size()-1].back()->parent;

        //Address the now seperated chain in T1 and T2 through the removal of the parent nodes of the terminals as well
        //as setting the terminals to be single tree vertices.

        //For xn to root, remove the terminal markers within their respective subtreeTerminals bitmask.
        updateSubtreeTerminals();

        for (int i = chainWithTrees.first.size()-1; i > 0; i--)
        {
            //Every element of chainWithTrees.first is a parent of a terminal. Index cycles through each.
            removeConnectionOfTerminalNode(chainWithTrees.first[i].front(), chainWithTrees.second.front());
            chainWithTrees.first[i].front()->subtreeTerminals = {};
            removeConnectionOfTerminalNode(chainWithTrees.first[i].back(), chainWithTrees.second.back());
            chainWithTrees.first[i].back()->subtreeTerminals = {};
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

    std::vector<graph::Node*> rootsT1 = chainWithTrees.second.front()->Roots();
    std::vector<graph::Node*> rootsT2 = chainWithTrees.second.back()->Roots();
    if (not this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    //For all potentially edited nodes out of T1, restore their old connections to other nodes by index
    for (int index = 0; index < deletedNodesT1Indices.first.size(); index++)
    {
        if (deletedNodesT1Indices.first[index].empty() == false)
        {
            //parent
            if (deletedNodesT1Indices.first[index][0] != -1)
            {
                nodesT1[index].parent = &nodesT1.at(deletedNodesT1Indices.first[index][0]);
            }
            //sibling
            if (deletedNodesT1Indices.first[index][1] != -1)
            {
                nodesT1[index].sibling = &nodesT1.at(deletedNodesT1Indices.first[index][1]);
            }
            //left
            if (deletedNodesT1Indices.first[index][2] != -1)
            {
                nodesT1[index].leftChild = &nodesT1.at(deletedNodesT1Indices.first[index][2]);
            }
            //right
            if (deletedNodesT1Indices.first[index][3] != -1)
            {
                nodesT1[index].rightChild = &nodesT1.at(deletedNodesT1Indices.first[index][3]);
            }

            nodesT1[index].subtreeTerminals = deletedNodesT1Indices.second[index];
        }
    }
    //For all potentially edited nodes out of T2, restore their old connections to other nodes by index
    for (int index = 0; index < deletedNodesT2Indices.first.size(); index++)
    {
        if (deletedNodesT2Indices.first[index].empty() == false)
        {
            //parent
            if (deletedNodesT2Indices.first[index][0] != -1)
            {
                nodesT2[index].parent = &nodesT2.at(deletedNodesT2Indices.first[index][0]);
            }
            //sibling
            if (deletedNodesT2Indices.first[index][1] != -1)
            {
                nodesT2[index].sibling = &nodesT2.at(deletedNodesT2Indices.first[index][1]);
            }
            //left
            if (deletedNodesT2Indices.first[index][2] != -1)
            {
                nodesT2[index].leftChild = &nodesT2.at(deletedNodesT2Indices.first[index][2]);
            }
            //right
            if (deletedNodesT2Indices.first[index][3] != -1)
            {
                nodesT2[index].rightChild = &nodesT2.at(deletedNodesT2Indices.first[index][3]);
            }

            nodesT2[index].subtreeTerminals = deletedNodesT2Indices.second[index];
        }
    }

    //Restore all root nodes in T1
    for (int index = 0; index < rootsT1Indices.first.size(); index++)
    {
        if (rootsT1Indices.first[index][0] != -1)
            rootsT1[index]->leftChild = &nodesT1.at(rootsT1Indices.first[index][0]);
        if (rootsT1Indices.first[index][1] != -1)
            rootsT1[index]->rightChild = &nodesT1.at(rootsT1Indices.first[index][1]);
    }
    //Remove all unnecessary root node entries?
    rootsT1.resize(rootsT1Indices.first.size());

    //Restore all root nodes in T2
    for (int index = 0; index < rootsT2Indices.first.size(); index++)
    {
        if (rootsT2Indices.first[index][0] != -1)
            rootsT2[index]->leftChild = &nodesT2.at(rootsT2Indices.first[index][0]);
        if (rootsT2Indices.first[index][1] != -1)
            rootsT2[index]->rightChild = &nodesT2.at(rootsT2Indices.first[index][1]);
    }
    rootsT2.resize(rootsT2Indices.first.size());

}

int solver::ChainReductionRule::identifyDistanceToRoot(graph::Node* node, std::shared_ptr<graph::Forest>& forest)
{
        int counter = 0;
        graph::Node* current = node;
        if (isNodeInNodeVector(node, forest->Roots())) return 0;
        while (true)
        {
            current = current->parent;
            counter++;
            if (isNodeInNodeVector(current, forest->Roots())) return counter;
        }
}

std::vector<int> solver::ChainReductionRule::structureToRoot(graph::Node* node, std::shared_ptr<graph::Forest>& forest)
{
    std::vector<int> result = {};
    graph::Node* current = node;
    graph::Node* parent = current->parent;
    while (parent!=nullptr)
    {
        if (parent->leftChild == current) result.push_back(0);
        else if (parent->rightChild == current) result.push_back(1);
        current = current->parent;
        parent = parent->parent;
    }
    return result;
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
        std::unordered_map<graph::Node*,unsigned int>& termIndexTreeOne = T1->TerminalToLabel();
        std::unordered_map<graph::Node*,unsigned int>& termIndexTreeTwo = T2->TerminalToLabel();

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
                        std::vector structureToRootT1 = structureToRoot(terminalT1.first, T1);
                        std::vector structureToRootT2 = structureToRoot(terminalT2.first,T2);

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

                        && identifyDistanceToRoot(terminalT1.first,T1) == identifyDistanceToRoot(terminalT2.first,T2)
                        && structureToRootT1 == structureToRootT2)
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

                        && identifyDistanceToRoot(terminalT1.first,T1) == identifyDistanceToRoot(terminalT2.first,T2)
                        && structureToRootT1 == structureToRootT2
                        )
                        {
                            //std::cout << "case 2" << std::endl;
                            //Chain for both trees, parent nodes of x1, x2, x3
                            std::vector<std::vector<graph::Node*>> chainT1T2 = {
                                {parentOfParentT1->parent,parentOfParentT2->parent}};

                            graph::Node* case2T1Parent = parentOfParentT1->parent;
                            graph::Node* case2T2Parent = parentOfParentT2->parent;

                            graph::Node* case2T1Sibling = nullptr;
                            graph::Node* case2T2Sibling = nullptr;
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

