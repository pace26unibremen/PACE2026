#include "DecoupleSubtreeAction.hpp"

#include <algorithm>

solver::DecoupleSubtreeAction::DecoupleSubtreeAction(graph::Node* subtreeRoot, unsigned int newLabel,
                                                     const std::shared_ptr<graph::Forest>& forest) :
        forest(forest),
        subtreeRoot(subtreeRoot),
        newNode(*subtreeRoot),
        newLabel(newLabel),
        smallestLabelOfSubtree(smallestLabelOfSubtree = subtreeRoot->smallestTerminal())
{}

void propagateXORLabelUp(const std::vector<u_int64_t>& labels, graph::Node* start)
{
    // Update subtreeTerminals up the tree, which now has child and it's subtrees removed
    const unsigned int subtreeTerminalsSize = start->subtreeTerminals.size();
    graph::Node* traversedNode = start;

    // Traverse up to the root, updating subtreeTerminals and sorting children
    while (traversedNode != nullptr)
    {
        // Update subtreeTerminals by removing child's terminals
        for (unsigned int i = 0; i < subtreeTerminalsSize; i++)
        {
            // Use XOR to remove terminals
            traversedNode->subtreeTerminals[i] ^= labels[i];
        }

        // sort children
        graph::Node* l = traversedNode->leftChild;
        graph::Node* r = traversedNode->rightChild;

        if (l and r and r->hasSmallestTerminal(l))
        {
            traversedNode->leftChild = r;
            traversedNode->rightChild = l;
        }
        traversedNode = traversedNode->parent;
    }
}


void solver::DecoupleSubtreeAction::doAction()
{
    // adjust edges of the new terminal (the subtreeRoot) and the new root of the subtree (newNode)
    // note that newNode is a copy of subtreeNode (Constructor)
    subtreeRoot->leftChild = nullptr;
    subtreeRoot->rightChild = nullptr;

    newNode.parent = nullptr;
    newNode.leftChild->parent = &newNode;
    newNode.rightChild->parent = &newNode;

    // update subtreeTerminals vector
    std::vector<u_int64_t> labels = subtreeRoot->subtreeTerminals;
    labels[(newLabel - 1) / 64] ^= (uint64_t) 1 << (newLabel -1) % 64;

    propagateXORLabelUp(labels, subtreeRoot);

    forest->LabelToTerminal()[newLabel] = subtreeRoot;
    forest->TerminalToLabel()[subtreeRoot] = newLabel;

    // we need to insert the new root at the correct index, and we may also need to reposition the og root of the tree
    auto root_Iterator = std::find_if(forest->Roots().begin(), forest->Roots().end(),
                     [&](const graph::Node* r) { return subtreeRoot->hasSubsetTerminals(r); });
    // find og node of the tree
    int indexOfTreeRoot = std::distance(forest->Roots().begin(), root_Iterator);
    graph::Node* root = forest->Roots()[indexOfTreeRoot];
    // check how to insert
    bool newRootIsHigher = root->hasSmallestTerminal(&newNode);
    if(newRootIsHigher)
    {
        indexOfOldRoot = indexOfTreeRoot;
        const auto newRoot_Iterator =
            std::lower_bound(forest->Roots().begin() + indexOfTreeRoot, forest->Roots().end(), &newNode,
                             [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
        forest->Roots().insert(newRoot_Iterator, &newNode);
        indexOfNewRoot = std::distance(forest->Roots().begin(), newRoot_Iterator);
    }
    else
    {
        forest->Roots()[indexOfTreeRoot] = &newNode;
        indexOfNewRoot = indexOfTreeRoot;

        const auto oldRoot_Iterator =
                std::lower_bound(forest->Roots().begin() + indexOfTreeRoot, forest->Roots().end(), root,
                [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
        forest->Roots().insert(oldRoot_Iterator, root);
        indexOfOldRoot = std::distance(forest->Roots().begin(), oldRoot_Iterator);
    }
}

void solver::DecoupleSubtreeAction::undoAction()
{
    // the root of the decoupled subtree may have changed
    // we assume here that the position in the roots-vector is the same
    // and update newNode as root of decoupled subtree
    newNode = *forest->Roots()[indexOfNewRoot];

    // undo edges
    subtreeRoot->leftChild = newNode.leftChild;
    subtreeRoot->rightChild = newNode.rightChild;
    subtreeRoot->leftChild->parent = subtreeRoot;
    subtreeRoot->rightChild->parent = subtreeRoot;

    newNode.parent = subtreeRoot->parent;

    // update subtreeTerminals vector
    std::vector<u_int64_t> labels = newNode.subtreeTerminals;
    labels[(newLabel - 1) / 64] ^= (uint64_t) 1 << (newLabel -1) % 64;

    propagateXORLabelUp(labels, subtreeRoot);

    forest->LabelToTerminal().erase(newLabel);
    forest->TerminalToLabel().erase(subtreeRoot);


    // undo roots
    auto oldRoot = forest->Roots()[indexOfOldRoot];
    bool newRootIsHigher = oldRoot->hasSmallestTerminal(&newNode);
    if(newRootIsHigher)
    {
        forest->Roots().erase(forest->Roots().begin() + indexOfNewRoot);
    }
    else
    {
        forest->Roots()[indexOfNewRoot] = oldRoot;
        forest->Roots().erase(forest->Roots().begin() + indexOfOldRoot);
    }
}
