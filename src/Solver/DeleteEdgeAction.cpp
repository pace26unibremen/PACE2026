
#include "DeleteEdgeAction.hpp"

#include <algorithm>
#include <utility>

using namespace graph;
using namespace solver;

solver::DeleteEdgeAction::DeleteEdgeAction(int childIndex, const std::shared_ptr<graph::Forest>& forest) :
        forest(forest),
        childIndex(childIndex)
{}

void solver::DeleteEdgeAction::doAction()
{
    siblingIndex = forest->Nodes()[childIndex].siblingIndex;
    parentIndex = forest->Nodes()[childIndex].parentIndex;
    leftIndex = forest->Nodes()[parentIndex].firstChildIndex;
    rightIndex = forest->Nodes()[parentIndex].secondChildIndex;

    auto leftRootIterator = std::find(forest->RootIndices().begin(), forest->RootIndices().end(), parentIndex);
    leftRoot_RootsIndex = std::distance(forest->RootIndices().begin(), leftRootIterator);
    parentIsRoot = (leftRootIterator != forest->RootIndices().end());

    if (parentIsRoot)
    {
        doParentIsRoot();
    }
    else
    {
        doParentIsInner();
    }

    // can be removed if new isValid is active
    Node& parent = forest->Nodes()[parentIndex];
    parentCopy = parent;
    parent.firstChildIndex = -1;
    parent.secondChildIndex = -1;
    parent.parentIndex = -1;
    parent.siblingIndex = -1;
}

void DeleteEdgeAction::undoAction()
{
    // can be removed if new isValid is active
    forest->Nodes()[parentIndex] = parentCopy;
    //

    if (parentIsRoot)
    {
        undoParentIsRoot();
    }
    else
    {
        undoParentIsInner();
    }
}

void DeleteEdgeAction::doParentIsRoot()
{
    // first new root is always at same position than the parent
    forest->RootIndices()[leftRoot_RootsIndex] = leftIndex;

    // position of second new root is somewhere after the old parent
    auto rightRoot_Iterator =
        std::lower_bound(forest->RootIndices().begin() + leftRoot_RootsIndex, forest->RootIndices().end(), rightIndex,
                         [&](const int& a, const int& b) {
                             const Node& an = forest->Nodes()[a];
                             const Node& bn = forest->Nodes()[b];
                             return an.hasSmallestTerminal(bn);
                         });
    rightRoot_RootsIndex = std::distance(forest->RootIndices().begin(), rightRoot_Iterator);

    forest->RootIndices().insert(rightRoot_Iterator, rightIndex);

    forest->Nodes()[siblingIndex].siblingIndex = -1;
    forest->Nodes()[siblingIndex].parentIndex = -1;
    forest->Nodes()[childIndex].siblingIndex = -1;
    forest->Nodes()[childIndex].parentIndex = -1;
}

void DeleteEdgeAction::undoParentIsRoot()
{
    forest->Nodes()[siblingIndex].siblingIndex = childIndex;
    forest->Nodes()[siblingIndex].parentIndex = parentIndex;
    forest->Nodes()[childIndex].siblingIndex = siblingIndex;
    forest->Nodes()[childIndex].parentIndex = parentIndex;

    forest->RootIndices()[leftRoot_RootsIndex] = parentIndex;
    forest->RootIndices().erase(forest->RootIndices().begin() + rightRoot_RootsIndex);
}

void DeleteEdgeAction::doParentIsInner()
{
    Node& child = forest->Nodes()[childIndex];
    Node& parent = forest->Nodes()[parentIndex];
    Node& sibling = forest->Nodes()[siblingIndex];
    Node& grandParent = forest->Nodes()[parent.parentIndex];

    if (grandParent.firstChildIndex == parentIndex)
    {
        grandParent.firstChildIndex = siblingIndex;
    }
    else
    {
        grandParent.secondChildIndex = siblingIndex;
    }

    sibling.parentIndex = parent.parentIndex;
    sibling.siblingIndex = parent.siblingIndex;
    forest->Nodes()[parent.siblingIndex].siblingIndex = siblingIndex;

    const unsigned int subtreeTerminalsSize = child.subtreeTerminals.size();
    int traverseUpIndex = child.parentIndex;
    int rootIndex;
    while (traverseUpIndex >= 0)
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
        if (r.hasSmallestTerminal(l))
        {
            std::swap(traversedNode.firstChildIndex, traversedNode.secondChildIndex);
        }
    }

    auto leftRoot_Iterator = std::find(forest->RootIndices().begin(), forest->RootIndices().end(), rootIndex);
    leftRoot_RootsIndex = std::distance(forest->RootIndices().begin(), leftRoot_Iterator);
    auto rootNode = forest->Nodes()[*leftRoot_Iterator];

    if (rootNode.hasSmallestTerminal(child))
    {
        auto rightRoot_Iterator = std::lower_bound(leftRoot_Iterator, forest->RootIndices().end(), childIndex,
                                                   [&](const int& a, const int& b) {
                                                       const Node& an = forest->Nodes()[a];
                                                       const Node& bn = forest->Nodes()[b];
                                                       return an.hasSmallestTerminal(bn);
                                                   });
        rightRoot_RootsIndex = std::distance(forest->RootIndices().begin(), rightRoot_Iterator);
        forest->RootIndices().insert(rightRoot_Iterator, childIndex);
    }
    else
    {
        *leftRoot_Iterator = childIndex;
        auto rightRoot_Iterator = std::lower_bound(leftRoot_Iterator, forest->RootIndices().end(), rootIndex,
                                                   [&](const int& a, const int& b) {
                                                       const Node& an = forest->Nodes()[a];
                                                       const Node& bn = forest->Nodes()[b];
                                                       return an.hasSmallestTerminal(bn);
                                                   });
        rightRoot_RootsIndex = std::distance(forest->RootIndices().begin(), rightRoot_Iterator);
        forest->RootIndices().insert(rightRoot_Iterator, rootIndex);
    }

    child.siblingIndex = -1;
    child.parentIndex = -1;
}

void DeleteEdgeAction::undoParentIsInner()
{
    Node& child = forest->Nodes()[childIndex];
    Node& parent = forest->Nodes()[parentIndex];
    Node& sibling = forest->Nodes()[siblingIndex];
    Node& grandParent = forest->Nodes()[parent.parentIndex];

    if (grandParent.firstChildIndex == siblingIndex)
    {
        grandParent.firstChildIndex = parentIndex;
    }
    else
    {
        grandParent.secondChildIndex = parentIndex;
    }

    sibling.parentIndex = parentIndex;
    sibling.siblingIndex = childIndex;
    child.siblingIndex = siblingIndex;
    child.parentIndex = parentIndex;
    forest->Nodes()[parent.siblingIndex].siblingIndex = parentIndex;

    const unsigned int subtreeTerminalsSize = child.subtreeTerminals.size();
    int traverseUpIndex = child.parentIndex;
    int rootIndex;
    while (traverseUpIndex >= 0)
    {
        Node& traversedNode = forest->Nodes()[traverseUpIndex];
        rootIndex = traverseUpIndex;
        for (unsigned int i = 0; i < subtreeTerminalsSize; i++)
        {
            traversedNode.subtreeTerminals[i] |= child.subtreeTerminals[i];
        }
        traverseUpIndex = traversedNode.parentIndex;
        // sort children
        const Node& l = forest->Nodes()[traversedNode.firstChildIndex];
        const Node& r = forest->Nodes()[traversedNode.secondChildIndex];
        if (r.hasSmallestTerminal(l))
        {
            std::swap(traversedNode.firstChildIndex, traversedNode.secondChildIndex);
        }
    }

    forest->RootIndices()[leftRoot_RootsIndex] = rootIndex;
    forest->RootIndices().erase(forest->RootIndices().begin() + rightRoot_RootsIndex);
}
