#include "ChainReductionRule.hpp"

#include "../Action/AddEdgeAction.hpp"
#include "../Action/DeleteEdgeAction.hpp"

#include <algorithm>
#include <random>
#include <unordered_map>
#include <utility>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) :
        AbstractRule(instance, context, true)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = std::move(chainWithTrees);
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
    for(auto i : list)
    {
       if (node == i)
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

void solver::ChainReductionRule::storeNodeIndices(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest)
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
        storeNodeIndices(terminal, forest);

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

    if (this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

    if (chainWithTrees.second.size() == 2 && chainWithTrees.first.size() >= 2)
    {
        // Acquire the notes required
        // Fetch x3
        graph::Node* bottomT1 = chainWithTrees.first[0].front();
        graph::Node* bottomT2 = chainWithTrees.first[0].back();

        graph::Node* bottomChainT1 = chainWithTrees.first[1].front();
        graph::Node* bottomChainT2 = chainWithTrees.first[1].back();

        graph::Node* topChainT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
        graph::Node* topChainT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

        //Fetch xn's parent out of tree.
        graph::Node* topChainT1Parent = topChainT1->parent;
        graph::Node* topChainT2Parent = topChainT2->parent;

        auto T1 = chainWithTrees.second.front();
        auto T2 = chainWithTrees.second.back();


        if (topChainT1Parent->leftChild == topChainT1)
        {
            topChainT1Parent->leftChild = bottomT1;
            topChainT1Parent->rightChild->sibling = bottomT1;

            bottomT1->parent = topChainT1Parent;
            bottomT1->sibling = topChainT1Parent->rightChild;

            auto newSubtreeTerminal = bottomT1->subtreeTerminals;

            for (int i = 0; i < newSubtreeTerminal.size(); i++)
            {
                newSubtreeTerminal[i] |= topChainT1Parent->rightChild->subtreeTerminals[i];
            }

            topChainT1Parent->subtreeTerminals = newSubtreeTerminal;

            auto current = topChainT1Parent;
            while (current->parent)
            {
                graph::Node* previous = current;
                current = current->parent;
                newSubtreeTerminal = previous->subtreeTerminals;
                if (current->leftChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->rightChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
                else if (current->rightChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->leftChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
            }
        }

        else
        {
            topChainT1Parent->rightChild = bottomT1;
            topChainT1Parent->leftChild->sibling = bottomT1;

            bottomT1->parent = topChainT1Parent;
            bottomT1->sibling = topChainT1Parent->leftChild;

            auto newSubtreeTerminal = bottomT1->subtreeTerminals;

            for (int i = 0; i < newSubtreeTerminal.size(); i++)
            {
                newSubtreeTerminal[i] |= topChainT1Parent->leftChild->subtreeTerminals[i];
            }

            topChainT1Parent->subtreeTerminals = newSubtreeTerminal;

            auto current = topChainT1Parent;
            while (!current->parent)
            {
                graph::Node* previous = current;
                current = current->parent;
                newSubtreeTerminal = previous->subtreeTerminals;
                if (current->leftChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->rightChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
                else if (current->rightChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->leftChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
            }
        }

        if (topChainT2Parent->leftChild == topChainT2)
        {
            topChainT2Parent->leftChild = bottomT2;
            topChainT2Parent->rightChild->sibling = bottomT2;

            bottomT2->parent = topChainT2Parent;
            bottomT2->sibling = topChainT2Parent->rightChild;

            auto newSubtreeTerminal = bottomT2->subtreeTerminals;

            for (int i = 0; i < newSubtreeTerminal.size(); i++)
            {
                newSubtreeTerminal[i] |= topChainT2Parent->rightChild->subtreeTerminals[i];
            }

            topChainT2Parent->subtreeTerminals = newSubtreeTerminal;

            graph::Node* previous;
            auto current = topChainT2Parent;
            while (current->parent)
            {
                previous = current;
                current = current->parent;
                newSubtreeTerminal = previous->subtreeTerminals;
                if (current->leftChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->rightChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
                else if (current->rightChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->leftChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
            }
        }
        else
        {
            topChainT2Parent->rightChild = bottomT2;
            topChainT2Parent->leftChild->sibling = bottomT2;

            bottomT2->parent = topChainT2Parent;
            bottomT2->sibling = topChainT2Parent->leftChild;

            auto newSubtreeTerminal = bottomT2->subtreeTerminals;

            for (int i = 0; i < newSubtreeTerminal.size(); i++)
            {
                newSubtreeTerminal[i] |= topChainT2Parent->leftChild->subtreeTerminals[i];
            }

            topChainT2Parent->subtreeTerminals = newSubtreeTerminal;

            graph::Node* previous;
            auto current = topChainT2Parent;
            while (!current->parent)
            {
                previous = current;
                current = current->parent;
                newSubtreeTerminal = previous->subtreeTerminals;
                if (current->leftChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->rightChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
                else if (current->rightChild == previous)
                {
                    for (int i = 0; i < newSubtreeTerminal.size(); i++)
                    {
                        newSubtreeTerminal[i] |= current->leftChild->subtreeTerminals[i];
                    }
                    current->subtreeTerminals = newSubtreeTerminal;
                }
            }
        }
        //X4
        if (bottomChainT1->leftChild == bottomT1)
        {
            beOneIfTerminalIsRightSide = 1;

            graph::Node* childT1 = bottomChainT1->rightChild;
            graph::Node* childT2 = bottomChainT2->rightChild;

            bottomChainT1->parent = nullptr;
            bottomChainT2->parent = nullptr;

            bottomChainT1->sibling = nullptr;
            bottomChainT2->sibling = nullptr;

            bottomChainT1->leftChild = nullptr;
            bottomChainT1->rightChild = nullptr;

            bottomChainT2->leftChild = nullptr;
            bottomChainT2->rightChild = nullptr;

            bottomChainT1->subtreeTerminals.clear();
            bottomChainT2->subtreeTerminals.clear();

            const auto newRootIteratorT1 = std::lower_bound(T1->Roots().begin(), T1->Roots().end(), childT1,
                   [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
            T1->Roots().insert(newRootIteratorT1, childT1);

            const auto newRootIteratorT2 = std::lower_bound(T2->Roots().begin(), T2->Roots().end(), childT2,
                     [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b);});
            T2->Roots().insert(newRootIteratorT2,childT2);

            disconnectedTerminalsT1.emplace_back(childT1);
            disconnectedTerminalsT2.emplace_back(childT2);
        }
        else
        {
            beOneIfTerminalIsRightSide = 0;

            graph::Node* childT1 = bottomChainT1->leftChild;
            graph::Node* childT2 = bottomChainT2->leftChild;

            bottomChainT1->parent = nullptr;
            bottomChainT2->parent = nullptr;

            bottomChainT1->sibling = nullptr;
            bottomChainT2->sibling = nullptr;

            bottomChainT1->leftChild = nullptr;
            bottomChainT1->rightChild = nullptr;

            bottomChainT2->leftChild = nullptr;
            bottomChainT2->rightChild = nullptr;

            bottomChainT1->subtreeTerminals.clear();
            bottomChainT2->subtreeTerminals.clear();

            const auto newRootIteratorT1 = std::lower_bound(T1->Roots().begin(), T1->Roots().end(), childT1,
                   [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
            T1->Roots().insert(newRootIteratorT1, childT1);

            const auto newRootIteratorT2 = std::lower_bound(T2->Roots().begin(), T2->Roots().end(), childT2,
                     [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b);});
            T2->Roots().insert(newRootIteratorT2,childT2);

            disconnectedTerminalsT1.emplace_back(childT1);
            disconnectedTerminalsT2.emplace_back(childT2);
        }


        //X5+
        if (beOneIfTerminalIsRightSide == 1)
        {
            for (int i = 2; i < chainWithTrees.first.size(); i++)
            {
                auto nodeT1 = chainWithTrees.first[i].front();
                auto nodeT2 = chainWithTrees.first[i].back();
                auto childT1 = chainWithTrees.first[i].front()->rightChild;
                auto childT2 = chainWithTrees.first[i].back()->rightChild;

                //Remove sibling and parent from the terminal
                childT1->parent = nullptr;
                childT2->parent = nullptr;

                childT1->sibling = nullptr;
                childT2->sibling = nullptr;

                //Add single vertex terminal into Roots now
                const auto newRootIteratorT1 = std::lower_bound(T1->Roots().begin(), T1->Roots().end(), childT1,
                    [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
                T1->Roots().insert(newRootIteratorT1, childT1);

                const auto newRootIteratorT2 = std::lower_bound(T2->Roots().begin(), T2->Roots().end(), childT2,
                         [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b);});
                T2->Roots().insert(newRootIteratorT2,childT2);

                //Remember the reference for unapply
                disconnectedTerminalsT1.emplace_back(childT1);
                disconnectedTerminalsT2.emplace_back(childT2);

                //Remove all edges and the subtree terminals of parent
                nodeT1->parent = nullptr;
                nodeT2->parent = nullptr;

                nodeT1->sibling = nullptr;
                nodeT2->sibling = nullptr;

                nodeT1->leftChild = nullptr;
                nodeT2->leftChild = nullptr;

                nodeT1->rightChild = nullptr;
                nodeT2->rightChild = nullptr;

                nodeT1->subtreeTerminals.clear();
                nodeT2->subtreeTerminals.clear();

            }
        }
        else
        {
            for (int i = 2; i < chainWithTrees.first.size(); i++)
            {
                auto nodeT1 = chainWithTrees.first[i].front();
                auto nodeT2 = chainWithTrees.first[i].back();
                auto childT1 = chainWithTrees.first[i].front()->leftChild;
                auto childT2 = chainWithTrees.first[i].back()->leftChild;

                //Remove sibling and parent from the terminal
                childT1->parent = nullptr;
                childT2->parent = nullptr;

                childT1->sibling = nullptr;
                childT2->sibling = nullptr;

                //Add single vertex terminal into Roots now
                const auto newRootIteratorT1 = std::lower_bound(T1->Roots().begin(), T1->Roots().end(), childT1,
                    [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
                T1->Roots().insert(newRootIteratorT1, childT1);

                const auto newRootIteratorT2 = std::lower_bound(T2->Roots().begin(), T2->Roots().end(), childT2,
                         [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b);});
                T2->Roots().insert(newRootIteratorT2,childT2);

                //Remember the reference for unapply
                disconnectedTerminalsT1.emplace_back(childT1);
                disconnectedTerminalsT2.emplace_back(childT2);

                //Remove all edges and the subtree terminals of parent
                nodeT1->parent = nullptr;
                nodeT2->parent = nullptr;

                nodeT1->sibling = nullptr;
                nodeT2->sibling = nullptr;

                nodeT1->leftChild = nullptr;
                nodeT2->leftChild = nullptr;

                nodeT1->rightChild = nullptr;
                nodeT2->rightChild = nullptr;

                nodeT1->subtreeTerminals.clear();
                nodeT2->subtreeTerminals.clear();
            }
        }
    }


    return RuleReturnCode::Continue;
}

void solver::ChainReductionRule::unapply()
{
    std::vector<graph::Node*> rootsT1 = chainWithTrees.second.front()->Roots();
    std::vector<graph::Node*> rootsT2 = chainWithTrees.second.back()->Roots();
    if (not this->isApplied)
    {
        throw std::invalid_argument("ChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

    // Acquire the notes required
    // Fetch x3
    graph::Node* bottomT1 = chainWithTrees.first[0].front();
    graph::Node* bottomT2 = chainWithTrees.first[0].back();

    graph::Node* topChainT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
    graph::Node* topChainT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

    //Fetch xn's parent out of tree.
    graph::Node* topChainT1Parent = bottomT1->parent;
    graph::Node* topChainT2Parent = bottomT2->parent;

    // bottomT1->parent = bottomChainT1;
    // bottomT2->parent = bottomChainT1;
    //
    // if ( beOneIfTerminalIsRightSide == 0)
    // {
    //     bottomChainT1->leftChild = bottomT1;
    //     bottomChainT2->leftChild = bottomT2;
    // }
    // else
    // {
    //     bottomChainT1->rightChild = bottomT1;
    //     bottomChainT2->rightChild = bottomT2;
    // }

    for (int i = 1; i < chainWithTrees.first.size(); i++)
    {
        auto currentT1 = chainWithTrees.first[i].front();
        auto currentT2 = chainWithTrees.first[i].back();

        if (beOneIfTerminalIsRightSide == 0)
        {
            currentT1->leftChild = disconnectedTerminalsT1[i-1];
            currentT1->leftChild->parent = currentT1;

            currentT1->rightChild = chainWithTrees.first[i-1].front();
            currentT1->rightChild->parent = currentT1;

            currentT1->leftChild->sibling = currentT1->rightChild;
            currentT1->rightChild->sibling = currentT1->leftChild;


            currentT2->leftChild = disconnectedTerminalsT2[i-1];
            currentT2->leftChild->parent = currentT2;

            currentT2->rightChild = chainWithTrees.first[i-1].back();
            currentT2->rightChild->parent = currentT2;

            currentT2->leftChild->sibling = currentT2->rightChild;
            currentT2->rightChild->sibling = currentT2->leftChild;

            //Delete terminal from roots
            auto childTerminalIteratorT1 = std::ranges::find(rootsT1, currentT1->leftChild);
            auto childTerminalIteratorT2 = std::ranges::find(rootsT2, currentT2->leftChild);

            auto childTerminalIndexT1 = std::distance(rootsT1.begin(), childTerminalIteratorT1);
            auto childTerminalIndexT2 = std::distance(rootsT2.begin(), childTerminalIteratorT2);

            rootsT1.erase(rootsT1.begin() + childTerminalIndexT1);
            rootsT2.erase(rootsT2.begin() + childTerminalIndexT2);

            auto newSubtreeForCurrentT1 = currentT1->rightChild->subtreeTerminals;
            auto newSubtreeForCurrentT2 = currentT2->rightChild->subtreeTerminals;

            //Update subtree terminals
            for (int i = 0; i < newSubtreeForCurrentT1.size(); i++)
            {
                newSubtreeForCurrentT1[i] |= currentT1->leftChild->subtreeTerminals[i];
                newSubtreeForCurrentT2[i] |= currentT2->leftChild->subtreeTerminals[i];
            }
            currentT1->subtreeTerminals = newSubtreeForCurrentT1;
            currentT2->subtreeTerminals = newSubtreeForCurrentT2;
        }
        else
        {   //The terminals are rightChilds of the parent nodes in the chain
            currentT1->rightChild = disconnectedTerminalsT1[i-1];
            currentT1->rightChild->parent = currentT1;

            currentT1->leftChild = chainWithTrees.first[i-1].front();
            currentT1->leftChild->parent = currentT1;

            currentT1->rightChild->sibling = currentT1->leftChild;
            currentT1->leftChild->sibling = currentT1->rightChild;


            currentT2->rightChild = disconnectedTerminalsT2[i-1];
            currentT2->rightChild->parent = currentT2;

            currentT2->leftChild = chainWithTrees.first[i-1].back();
            currentT2->leftChild->parent = currentT2;

            currentT2->rightChild->sibling = currentT2->leftChild;
            currentT2->leftChild->sibling = currentT2->rightChild;

            //Delete terminal from roots
            auto childTerminalIteratorT1 = std::ranges::find(rootsT1, currentT1->rightChild);
            auto childTerminalIteratorT2 = std::ranges::find(rootsT2, currentT2->rightChild);

            auto childTerminalIndexT1 = std::distance(rootsT1.begin(), childTerminalIteratorT1);
            auto childTerminalIndexT2 = std::distance(rootsT2.begin(), childTerminalIteratorT2);

            rootsT1.erase(rootsT1.begin() + childTerminalIndexT1);
            rootsT2.erase(rootsT2.begin() + childTerminalIndexT2);

            auto newSubtreeForCurrentT1 = currentT1->leftChild->subtreeTerminals;
            auto newSubtreeForCurrentT2 = currentT2->leftChild->subtreeTerminals;

            //Update subtree terminals
            for (int i = 0; i < newSubtreeForCurrentT1.size(); i++)
            {
                newSubtreeForCurrentT1[i] |= currentT1->rightChild->subtreeTerminals[i];
                newSubtreeForCurrentT2[i] |= currentT2->rightChild->subtreeTerminals[i];
            }
            currentT1->subtreeTerminals = newSubtreeForCurrentT1;
            currentT2->subtreeTerminals = newSubtreeForCurrentT2;
        }
    }
    if (topChainT1Parent->leftChild == bottomT1)
    {
        topChainT1Parent->leftChild = topChainT1;
        topChainT1Parent->rightChild->sibling = topChainT1;
    }
    else
    {
        topChainT1Parent->rightChild = topChainT1;
        topChainT1Parent->leftChild->sibling = topChainT1;
    }
    if (topChainT2Parent->leftChild == bottomT2)
    {
        topChainT2Parent->leftChild = topChainT2;
        topChainT2Parent->rightChild->sibling = topChainT2;
    }
    else
    {
        topChainT2Parent->rightChild = topChainT2;
        topChainT2Parent->leftChild = topChainT2;
    }

    auto newSubtreeBitmask = topChainT1->subtreeTerminals;
    if (topChainT1Parent->leftChild == topChainT1)
    {
        for (int i = 0; i < newSubtreeBitmask.size(); i++)
        {
            newSubtreeBitmask[i] |= topChainT1Parent->rightChild->subtreeTerminals[i];
        }
        topChainT1Parent->subtreeTerminals = newSubtreeBitmask;
    }
    else
    {
        for (int i = 0; i < newSubtreeBitmask.size(); i++)
        {
            newSubtreeBitmask[i] |= topChainT1Parent->leftChild->subtreeTerminals[i];
        }
        topChainT1Parent->subtreeTerminals = newSubtreeBitmask;
    }

    newSubtreeBitmask = topChainT2->subtreeTerminals;

    if (topChainT2Parent->leftChild == topChainT2)
    {
        for (int i = 0; i < newSubtreeBitmask.size(); i++)
        {
            newSubtreeBitmask[i] |= topChainT2Parent->rightChild->subtreeTerminals[i];
        }
        topChainT2Parent->subtreeTerminals = newSubtreeBitmask;
    }
    else
    {
        for (int i = 0; i < newSubtreeBitmask.size(); i++)
        {
            newSubtreeBitmask[i] |= topChainT2Parent->leftChild->subtreeTerminals[i];
        }
        topChainT2Parent->subtreeTerminals = newSubtreeBitmask;
    }
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

std::vector<int> solver::ChainReductionRule::structureToRoot(graph::Node* node)
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

                    //Determine Structure
                    graph::Node* parentT2 = terminalT2.first->parent;

                    // -> x1,x2,x3 for case 1 known for T2

                    //Anti single vertex check for terminalT2
                    if (parentT2 != nullptr)
                    {
                        //Determine x3 for case 2 for chain def in T2
                        graph::Node* parentOfParentT2 = parentT2->parent;
                        std::vector structureToRootT1 = structureToRoot(terminalT1.first);
                        std::vector structureToRootT2 = structureToRoot(terminalT2.first);

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

