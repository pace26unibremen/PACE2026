#include "DecoupleSubtreeAction.hpp"

#include <algorithm>

solver::DecoupleSubtreeAction::DecoupleSubtreeAction(graph::Node* decouplingPoint, unsigned int newLabel,
                                                     const std::shared_ptr<graph::Forest>& forest) :
        forest(forest),
        decouplingPoint(decouplingPoint),
        decoupledSubtreeRoot(*decouplingPoint),
        newLabel(newLabel),
        smallestLabelOfSubtree(smallestLabelOfSubtree = decouplingPoint->smallestTerminal())
{}

void solver::DecoupleSubtreeAction::propagateXORLabelUp(const std::vector<u_int64_t>& labels, graph::Node* start)
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
    decouplingPoint->leftChild = nullptr;
    decouplingPoint->rightChild = nullptr;

    decoupledSubtreeRoot.parent = nullptr;
    decoupledSubtreeRoot.sibling = nullptr;
    decoupledSubtreeRoot.leftChild->parent = &decoupledSubtreeRoot;
    decoupledSubtreeRoot.rightChild->parent = &decoupledSubtreeRoot;

    // update subtreeTerminals vector
    std::vector<u_int64_t> labels = decouplingPoint->subtreeTerminals;
    labels[(newLabel - 1) / 64] ^= (uint64_t) 1 << (newLabel -1) % 64;

    propagateXORLabelUp(labels, decouplingPoint);

    forest->LabelToTerminal()[newLabel] = decouplingPoint;
    forest->TerminalToLabel()[decouplingPoint] = newLabel;

    // we need to insert the new root at the correct index, and we may also need to reposition the og root of the tree
    auto root_Iterator = std::find_if(forest->Roots().begin(), forest->Roots().end(),
                     [&](const graph::Node* r) { return decouplingPoint->hasSubsetTerminals(r); });
    // find og node of the tree
    int indexOfParentTreeRoot = std::distance(forest->Roots().begin(), root_Iterator);
    graph::Node* parentTreeRoot = forest->Roots()[indexOfParentTreeRoot];
    // check how to insert
    bool decoupledSubtreeIsBigger = parentTreeRoot->hasSmallestTerminal(&decoupledSubtreeRoot);
    if(decoupledSubtreeIsBigger)
    {
        auto decoupledSubtreeIt =
            std::lower_bound(root_Iterator, forest->Roots().end(), &decoupledSubtreeRoot,
                             [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
        forest->Roots().insert(decoupledSubtreeIt, &decoupledSubtreeRoot);
    }
    else
    {
        forest->Roots()[indexOfParentTreeRoot] = &decoupledSubtreeRoot;
        auto parentTreeIt =
                std::lower_bound(root_Iterator, forest->Roots().end(), parentTreeRoot,
                [&](const graph::Node* a, const graph::Node* b) { return a->hasSmallestTerminal(b); });
        forest->Roots().insert(parentTreeIt, parentTreeRoot);
    }
}

void solver::DecoupleSubtreeAction::undoAction()
{
    // We cannot use assume that the decoupled subtree has not changed.
    // So we couple only the part that contains the smallest terminal of the og subtree.
    auto decoupledSubtreePartIt = std::find_if(forest->Roots().begin(), forest->Roots().end(),[&]
        (const graph::Node* n) {return n->hasTerminal(smallestLabelOfSubtree);});

    // We also cannot assume that the parent tree has not changed,
    // so we search for the root that contains the newLabel (we could also traverse from subtreeRoot)
    auto parentTreePartIt = std::find_if(forest->Roots().begin(), forest->Roots().end(),[&]
        (const graph::Node* n) {return n->hasTerminal(newLabel);});

    decoupledSubtreeRoot = **decoupledSubtreePartIt;
    auto parentTreeRoot = *parentTreePartIt;


    // undo edges
    decouplingPoint->leftChild = decoupledSubtreeRoot.leftChild;
    decouplingPoint->rightChild = decoupledSubtreeRoot.rightChild;
    if (decouplingPoint->leftChild)
    {
        decouplingPoint->leftChild->parent = decouplingPoint;
    }
    if (decouplingPoint->rightChild)
    {
        decouplingPoint->rightChild->parent = decouplingPoint;
    }

    // this should be not necessary, but we just restore the situation
    decoupledSubtreeRoot.parent = decouplingPoint->parent;
    decoupledSubtreeRoot.sibling = decouplingPoint->sibling;

    // update subtreeTerminals vector
    std::vector<u_int64_t> labels = decoupledSubtreeRoot.subtreeTerminals;
    labels[(newLabel - 1) / 64] ^= (uint64_t) 1 << (newLabel -1) % 64;

    propagateXORLabelUp(labels, decouplingPoint);

    forest->LabelToTerminal().erase(newLabel);
    forest->TerminalToLabel().erase(decouplingPoint);


    // undo roots
    bool decoupledSubtreeIsBigger = parentTreeRoot->hasSmallestTerminal(&decoupledSubtreeRoot);
    if(decoupledSubtreeIsBigger)
    {
        forest->Roots().erase(decoupledSubtreePartIt);
    }
    else
    {
        *decoupledSubtreePartIt = parentTreeRoot;
        forest->Roots().erase(parentTreePartIt);
    }
}
