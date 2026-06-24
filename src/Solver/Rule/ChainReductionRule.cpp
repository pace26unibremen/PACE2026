#include "ChainReductionRule.hpp"

#include "../Action/AddEdgeAction.hpp"
#include "../Action/DeleteEdgeAction.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>

#include <assert.h>

solver::ChainReductionRule::ChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
    const std::shared_ptr<Context>& context
    ) :
        AbstractRule(instance, context, true),
        addedEdgeT1(nullptr,nullptr,nullptr),
        addedEdgeT2(nullptr,nullptr,nullptr)
{
    //Copying of the Trees maybe irrelevant when doing this without const params. Not sure.
    this->chainWithTrees = std::move(chainWithTrees);
    rootsT1Indices = {};
    rootsT2Indices = {};
    addedEdgeT1;
    addedEdgeT2;
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

void solver::ChainReductionRule::removeNodeOutOfRoot(graph::Node* node, std::vector<graph::Node*>& roots)
{
    // const auto RootIterator = std::ranges::find(chainWithTrees.second.front()->Roots(), topChainT1Parent->rightChild);
    // auto RootsIndex = std::distance(chainWithTrees.second.front()->Roots().begin(), RootIterator);
    // chainWithTrees.second.front()->Roots().erase(chainWithTrees.second.front()->Roots().begin() + RootsIndex);

    const auto RootIterator = std::ranges::find(roots, node);
    auto RootsIndex = std::distance(roots.begin(), RootIterator);
    roots.erase(roots.begin() + RootsIndex);
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

        graph::Node* topChainT1 = chainWithTrees.first[chainWithTrees.first.size()-1].front();
        graph::Node* topChainT2 = chainWithTrees.first[chainWithTrees.first.size()-1].back();

        //Fetch xn's parent out of tree.
        graph::Node* topChainT1Parent = topChainT1->parent;
        graph::Node* topChainT2Parent = topChainT2->parent;

        topOfChainT1 = topChainT1;
        topOfChainT2 = topChainT2;

        std::ranges::borrowed_iterator_t<std::vector<graph::Node*>&> aboveChainT1;
        std::ranges::borrowed_iterator_t<std::vector<graph::Node*>&> aboveChainT2;
        std::ptrdiff_t rootEntryIndexT1;
        std::ptrdiff_t rootEntryIndexT2;

        //If Root ended the chain in isApplicable
        if (!topChainT1Parent->parent && !topChainT1Parent->sibling)
        {
            aboveChainT1 = std::ranges::find(chainWithTrees.second.front()->Roots(), topChainT1Parent);
            rootEntryIndexT1 = std::distance(chainWithTrees.second.front()->Roots().begin(), aboveChainT1);
        }
        if (!topChainT2Parent->parent && !topChainT2Parent->sibling)
        {
            aboveChainT2 =  std::ranges::find(chainWithTrees.second.back()->Roots(), topChainT2Parent);
            rootEntryIndexT2 = std::distance(chainWithTrees.second.back()->Roots().begin(), aboveChainT2);
        }


        //Remove edge from the bottom of the to be removed part
        changes.emplace(bottomT1,chainWithTrees.second.front());
        changes.top().doAction();

        changes.emplace(bottomT2, chainWithTrees.second.back());
        changes.top().doAction();

        //Remove edge from top of chain to its parent
        changes.emplace(topChainT1,chainWithTrees.second.front());
        changes.top().doAction();

        changes.emplace(topChainT2, chainWithTrees.second.back());
        changes.top().doAction();


        if (topChainT1Parent->leftChild == topChainT1)
        {
            //Connect bottomT1 with topChainT1Parent
            //Bottom
            bottomT1->parent = topChainT1Parent;
            bottomT1->sibling = topChainT1Parent->rightChild;
            //Top
            topChainT1Parent->leftChild = bottomT1;

            removeNodeOutOfRoot(bottomT1,chainWithTrees.second.front()->Roots());

            //If the other child got turned into a single vertex tree
            if (!topChainT1Parent->rightChild->parent && !topChainT1Parent->rightChild->sibling)
            {
                //Delete it out of there
                topChainT1Parent->rightChild->parent = topChainT1Parent;
                topChainT1Parent->rightChild->sibling = bottomT1;
                removeNodeOutOfRoot(topChainT1Parent->rightChild, chainWithTrees.second.front()->Roots());

            }
            //Store pos of topChain, as it does match the pos for its parent if it's a root
            const auto topChainPosIterator = std::ranges::find(chainWithTrees.second.front()->Roots(), topChainT1);
            auto topChainIndex = std::distance(chainWithTrees.second.front()->Roots().begin(), topChainPosIterator);

            //Remove topChainT1 from the roots
            removeNodeOutOfRoot(topChainT1,chainWithTrees.second.front()->Roots());

            //Update subtree for topChainT1Parent and above
            auto newSubtreeTerminals = bottomT1->subtreeTerminals;
            for (int i = 0; i < newSubtreeTerminals.size(); i++)
            {
                newSubtreeTerminals[i] |= topChainT1Parent->rightChild->subtreeTerminals[i];
            }
            topChainT1Parent->subtreeTerminals = newSubtreeTerminals;

            //All nodes above topChainT1Parent
            if (topChainT1Parent->parent && topChainT1Parent->sibling)
            {
                newSubtreeTerminals = topChainT1Parent->subtreeTerminals;
                auto previous = topChainT1Parent;
                auto current = topChainT1Parent->parent;
                while (current->parent)
                {
                    if (current->leftChild == previous)
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->rightChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->leftChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    previous = previous->parent;
                    newSubtreeTerminals = previous->subtreeTerminals;
                    current = current->parent;
                }
            }
            else //topChainT1Parent was root! -> Enter into Root
            {
                chainWithTrees.second.front()->
                Roots().insert(chainWithTrees.second.front()->Roots().begin() + topChainIndex, topChainT1Parent);
            }

        }
        else // right side is topChain
        {
            bottomT1->parent = topChainT1Parent;
            bottomT1->sibling = topChainT1Parent->leftChild;
            topChainT1Parent->rightChild = bottomT1;

            removeNodeOutOfRoot(bottomT1,chainWithTrees.second.front()->Roots());

            //If the other child got turned into a single vertex tree
            if (!topChainT1Parent->leftChild->parent && !topChainT1Parent->leftChild->sibling)
            {
                //Delete it out of the Root and connect it
                topChainT1Parent->leftChild->parent = topChainT1Parent;
                topChainT1Parent->leftChild->sibling = bottomT1;
                removeNodeOutOfRoot(topChainT1Parent->leftChild, chainWithTrees.second.front()->Roots());

            }
            //Store pos of topChain, as it does match the pos for its parent if it's a root
            const auto topChainPosIterator = std::ranges::find(chainWithTrees.second.front()->Roots(), topChainT1);
            auto topChainIndex = std::distance(chainWithTrees.second.front()->Roots().begin(), topChainPosIterator);

            //Remove topChainT1 from the roots
            removeNodeOutOfRoot(topChainT1,chainWithTrees.second.front()->Roots());

            //Update subtree for topChainT1Parent and above
            auto newSubtreeTerminals = bottomT1->subtreeTerminals;
            for (int i = 0; i < newSubtreeTerminals.size(); i++)
            {
                newSubtreeTerminals[i] |= topChainT1Parent->leftChild->subtreeTerminals[i];
            }
            topChainT1Parent->subtreeTerminals = newSubtreeTerminals;

            //All nodes above topChainT1Parent
            if (topChainT1Parent->parent && topChainT1Parent->sibling)
            {
                newSubtreeTerminals = topChainT1Parent->subtreeTerminals;
                auto previous = topChainT1Parent;
                auto current = topChainT1Parent->parent;
                while (current->parent)
                {
                    if (current->leftChild == previous)
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->rightChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->leftChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    previous = previous->parent;
                    newSubtreeTerminals = previous->subtreeTerminals;
                    current = current->parent;
                }
            }
            else //topChainT1Parent was root! -> Enter into Root
            {
                chainWithTrees.second.front()->
                Roots().insert(chainWithTrees.second.front()->Roots().begin() + topChainIndex, topChainT1Parent);
            }
        }

        if (topChainT2Parent->leftChild == topChainT2)
        {
            //Connect bottomT1 with topChainT1Parent
            //Bottom
            bottomT2->parent = topChainT2Parent;
            bottomT2->sibling = topChainT2Parent->rightChild;
            //Top
            topChainT2Parent->leftChild = bottomT2;

            removeNodeOutOfRoot(bottomT2,chainWithTrees.second.back()->Roots());

            //If the other child got turned into a single vertex tree
            if (!topChainT2Parent->rightChild->parent && !topChainT2Parent->rightChild->sibling)
            {
                //Delete it out of there
                topChainT2Parent->rightChild->parent = topChainT2Parent;
                topChainT2Parent->rightChild->sibling = bottomT2;
                removeNodeOutOfRoot(topChainT2Parent->rightChild, chainWithTrees.second.back()->Roots());

            }
            //Store pos of topChain, as it does match the pos for its parent if it's a root
            const auto topChainPosIterator = std::ranges::find(chainWithTrees.second.back()->Roots(), topChainT2);
            auto topChainIndex = std::distance(chainWithTrees.second.back()->Roots().begin(), topChainPosIterator);

            //Remove topChainT1 from the roots
            removeNodeOutOfRoot(topChainT2,chainWithTrees.second.back()->Roots());

            //Update subtree for topChainT1Parent and above
            auto newSubtreeTerminals = bottomT2->subtreeTerminals;
            for (int i = 0; i < newSubtreeTerminals.size(); i++)
            {
                newSubtreeTerminals[i] |= topChainT2Parent->rightChild->subtreeTerminals[i];
            }
            topChainT2Parent->subtreeTerminals = newSubtreeTerminals;

            //All nodes above topChainT1Parent
            if (topChainT2Parent->parent && topChainT2Parent->sibling)
            {
                newSubtreeTerminals = topChainT2Parent->subtreeTerminals;
                auto previous = topChainT2Parent;
                auto current = topChainT2Parent->parent;
                while (current->parent)
                {
                    if (current->leftChild == previous)
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->rightChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->leftChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    previous = previous->parent;
                    newSubtreeTerminals = previous->subtreeTerminals;
                    current = current->parent;
                }
            }
            else //topChainT2Parent was root! -> Enter into Root
            {
                chainWithTrees.second.back()->
                Roots().insert(chainWithTrees.second.back()->Roots().begin() + topChainIndex, topChainT2Parent);
            }
        }
        else // right side is topChain
        {
            //Connect bottomT1 with topChainT1Parent
            //Bottom
            bottomT2->parent = topChainT2Parent;
            bottomT2->sibling = topChainT2Parent->leftChild;
            //Top
            topChainT2Parent->rightChild = bottomT2;

            removeNodeOutOfRoot(bottomT2,chainWithTrees.second.back()->Roots());

            //If the other child got turned into a single vertex tree
            if (!topChainT2Parent->leftChild->parent && !topChainT2Parent->leftChild->sibling)
                {
                    //Delete it out of there
                    topChainT2Parent->leftChild->parent = topChainT2Parent;
                    topChainT2Parent->leftChild->sibling = bottomT2;
                    removeNodeOutOfRoot(topChainT2Parent->leftChild, chainWithTrees.second.back()->Roots());

                }
            //Store pos of topChain, as it does match the pos for its parent if it's a root
            const auto topChainPosIterator = std::ranges::find(chainWithTrees.second.back()->Roots(), topChainT2);
            auto topChainIndex = std::distance(chainWithTrees.second.back()->Roots().begin(), topChainPosIterator);

            //Remove topChainT1 from the roots
            removeNodeOutOfRoot(topChainT2,chainWithTrees.second.back()->Roots());

            //Update subtree for topChainT1Parent and above
            auto newSubtreeTerminals = bottomT2->subtreeTerminals;
            for (int i = 0; i < newSubtreeTerminals.size(); i++)
            {
                    newSubtreeTerminals[i] |= topChainT2Parent->leftChild->subtreeTerminals[i];
            }
            topChainT2Parent->subtreeTerminals = newSubtreeTerminals;

            //All nodes above topChainT1Parent
            if (topChainT2Parent->parent && topChainT2Parent->sibling)
            {
                newSubtreeTerminals = topChainT2Parent->subtreeTerminals;
                auto previous = topChainT2Parent;
                auto current = topChainT2Parent->parent;
                while (current->parent)
                {
                    if (current->leftChild == previous)
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->rightChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < newSubtreeTerminals.size(); i++)
                        {
                            newSubtreeTerminals[i] |= current->leftChild->subtreeTerminals[i];
                            current->subtreeTerminals[i] = newSubtreeTerminals[i];
                        }
                    }
                    previous = previous->parent;
                    newSubtreeTerminals = previous->subtreeTerminals;
                    current = current->parent;
                }
            }
            else //topChainT2Parent was root! -> Enter into Root
            {
                chainWithTrees.second.back()->
                Roots().insert(chainWithTrees.second.back()->Roots().begin() + topChainIndex, topChainT2Parent);
            }
        }

        //
        // for (int i = chainWithTrees.first.size()-1; i > -1 ; i--)
        // {
        //     changes.emplace(chainWithTrees.first[i].front(),chainWithTrees.second.front());
        //     changes.top().doAction();
        //
        //     changes.emplace(chainWithTrees.first[i].back(),chainWithTrees.second.back());
        //     changes.top().doAction();
        //
        // }
        //
        //
        // if (chainWithTrees.first[1].front()->leftChild->leftChild == nullptr &&
        //      chainWithTrees.first[1].front()->leftChild->rightChild == nullptr)
        // {
        //     for (int i = chainWithTrees.first.size()-1; i > 0 ; i--)
        //     {
        //         changes.emplace(chainWithTrees.first[i].front()->leftChild,chainWithTrees.second.front());
        //         changes.top().doAction();
        //
        //         changes.emplace(chainWithTrees.first[i].back()->leftChild,chainWithTrees.second.back());
        //         changes.top().doAction();
        //
        //     }
        // }
        // else
        // {
        //     for(int i = chainWithTrees.first.size()-2; i > 0 ; i--)
        //         {
        //             changes.emplace(chainWithTrees.first[i].front()->rightChild,chainWithTrees.second.front());
        //             changes.top().doAction();
        //
        //             changes.emplace(chainWithTrees.first[i].back()->rightChild,chainWithTrees.second.back());
        //             changes.top().doAction();
        //
        //         }
        // }


        //Left side terminal - once one side is determined, it sticks since chain definition
        // if (chainWithTrees.first[1].front()->leftChild->leftChild == nullptr &&
        //     chainWithTrees.first[1].front()->leftChild->rightChild == nullptr)
        // {
        //     for (int i = 1; i < chainWithTrees.first.size(); i++)
        //     {
        //         //Remove terminal
        //         changes.emplace(chainWithTrees.first[i].front()->leftChild, chainWithTrees.second.front());
        //         changes.top().doAction();
        //
        //         //Remove parent of terminal
        //         changes.emplace(chainWithTrees.first[i-1].front(),chainWithTrees.second.front());
        //         changes.top().doAction();
        //     }
        //     for (int i = 1; i < chainWithTrees.first.size(); i++)
        //     {
        //         //Remove terminal
        //         changes.emplace(chainWithTrees.first[i].back()->leftChild, chainWithTrees.second.back());
        //         changes.top().doAction();
        //
        //         //Remove parent of terminal
        //         changes.emplace(chainWithTrees.first[i-1].back(),chainWithTrees.second.back());
        //         changes.top().doAction();
        //     }
        // }
        // else //right side
        // {
        //     for (int i = 1; i < chainWithTrees.first.size(); i++)
        //     {
        //         //Remove terminal
        //         changes.emplace(chainWithTrees.first[i].front()->rightChild, chainWithTrees.second.front());
        //         changes.top().doAction();
        //
        //         //Remove parent of terminal
        //         changes.emplace(chainWithTrees.first[i-1].front(),chainWithTrees.second.front());
        //         changes.top().doAction();
        //     }
        //     for (int i = 1; i < chainWithTrees.first.size(); i++)
        //     {
        //         //Remove terminal
        //         changes.emplace(chainWithTrees.first[i].back()->rightChild, chainWithTrees.second.back());
        //         changes.top().doAction();
        //
        //         //Remove parent of terminal
        //         changes.emplace(chainWithTrees.first[i-1].back(),chainWithTrees.second.back());
        //         changes.top().doAction();
        //     }
        // }

        // addedEdgeT1 = AddEdgeAction(bottomT1,topChainT1Parent, chainWithTrees.second.front());
        // addedEdgeT2 = AddEdgeAction(bottomT2,topChainT2Parent, chainWithTrees.second.back());
        //
        // addedEdgeT1.doAction();
        // addedEdgeT2.doAction();

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
    //
    // addedEdgeT2.undoAction();
    // addedEdgeT1.undoAction();

    while (not changes.empty())
    {
        changes.top().undoAction();
        changes.pop();
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

