
#include "DeleteEdgeAction.hpp"

#include <utility>
#include <algorithm>

using namespace graph;
using namespace solver;

solver::DeleteEdgeAction::DeleteEdgeAction(int childIndex, const std::shared_ptr<graph::Forest>& forest) :
    forest(forest),
    childIndex(childIndex)
{}

void solver::DeleteEdgeAction::doAction()
{
    Node& child = forest->Nodes()[childIndex];
    Node& sibling = forest->Nodes()[child.siblingIndex];
    Node& parent = forest->Nodes()[child.parentIndex];

    auto it = std::find(
        forest->RootIndices().begin(),
        forest->RootIndices().end(),
       child.parentIndex);

    // case 1: parent is root
    if(it != forest->RootIndices().end())
    {
        // first new root is always at same position than the parent
        *it = parent.firstChildIndex;

        // position of second new root is somewhere after the old parent
        auto it2 = std::lower_bound(
            it, forest->RootIndices().end(), parent.secondChildIndex,
            [&](const int& a, const int& b)
            {
                const Node& an = forest->Nodes()[a];
                const Node& bn = forest->Nodes()[b];
                return an.hasSmallestTerminal(bn);
            }
        );
        forest->RootIndices().insert(it2, parent.secondChildIndex);

        sibling.parentIndex = -1;
        sibling.siblingIndex = -1;
    }
    // case 2: parent is inner node
    else
    {
        Node& grandParent = forest->Nodes()[parent.parentIndex];
        if (grandParent.firstChildIndex == child.parentIndex)
        {
            grandParent.firstChildIndex = child.siblingIndex;
        }
        else
        {
            grandParent.secondChildIndex = child.siblingIndex;
        }
        sibling.parentIndex = parent.parentIndex;
        sibling.siblingIndex = parent.siblingIndex;
        forest->Nodes()[parent.siblingIndex].siblingIndex = child.siblingIndex;

        const unsigned int subtreeTerminalsSize = child.subtreeTerminals.size();
        int traverseUpIndex = child.parentIndex;
        int rootIndex;
        while(traverseUpIndex >= 0)
        {
            Node& traversedNode = forest->Nodes()[traverseUpIndex];
            rootIndex = traverseUpIndex;
            for (unsigned int i = 0; i < subtreeTerminalsSize; i++)
            {
                traversedNode.subtreeTerminals[i] ^= child.subtreeTerminals[i];
            }
            traverseUpIndex = traversedNode.parentIndex;
            // sort children
            const Node& l = forest->Nodes()[traversedNode.firstChildIndex];
            const Node& r = forest->Nodes()[traversedNode.secondChildIndex];
            if(r.hasSmallestTerminal(l))
            {
                std::swap(traversedNode.firstChildIndex, traversedNode.secondChildIndex);
            }
        }

        auto itRoot = std::find(forest->RootIndices().begin(), forest->RootIndices().end(),rootIndex);
        auto rootNode = forest->Nodes()[*itRoot];

        if(rootNode.hasSmallestTerminal(child))
        {
            auto itNewRoot =
                std::lower_bound(itRoot, forest->RootIndices().end(), childIndex, [&](const int& a, const int& b)
                                 {
                                     const Node& an = forest->Nodes()[a];
                                     const Node& bn = forest->Nodes()[b];
                                     return an.hasSmallestTerminal(bn);
                                 });
            forest->RootIndices().insert(itNewRoot, childIndex);
        }
        else
        {
            *itRoot = childIndex;
            auto itRootNewPosition =
                std::lower_bound(itRoot, forest->RootIndices().end(), rootIndex, [&](const int& a, const int& b)
                                 {
                                     const Node& an = forest->Nodes()[a];
                                     const Node& bn = forest->Nodes()[b];
                                     return an.hasSmallestTerminal(bn);
                                 });
            forest->RootIndices().insert(itRootNewPosition, rootIndex);
        }
    }
    // clean up refs
    child.siblingIndex = -1;
    child.parentIndex = -1;
    parent.firstChildIndex = -1;
    parent.secondChildIndex = -1;
    parent.parentIndex = -1;
    parent.siblingIndex = -1;
}

void DeleteEdgeAction::undoAction()
{
    throw std::invalid_argument("not implemented");
}
