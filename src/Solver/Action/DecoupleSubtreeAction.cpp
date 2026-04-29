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

    auto decoupledSubtreePartRoot = *decoupledSubtreePartIt;
    auto parentTreeRoot = *parentTreePartIt;

    // `decoupledSubtreeRoot`: The newly created node stored in this action instance.
    // `decoupledSubtreePartRoot`: The root node that will be coupled (merged) with
    // the terminal with the label `newLabel` (`decouplingPoint`)
    // `parentTreeRoot` The root of the tree that contains the `decouplingPoint`
    //
    // 1. `decoupledSubtreeRoot` may be unequal to `decoupledSubtreePartRoot`
    // 2. `decoupledSubtreeRoot` could be a valid root of a valid tree in the current forest.
    // 3. We need to ensure that after this method call the newly created node (`decoupledSubtreeRoot`)
    //    is not part of the forest anymore, because (probably) the rule and therefore the node
    //    will be garbage collected, which leads to invalid pointers.
    // idea: We just swap `decoupledSubtreePartRoot` and `decoupledSubtreeRoot`
    // but only iff `decoupledSubtree` is an actual root (it might already be not a part of the forest anymore :D)
    //  - `decoupledSubtreePartRoot` needs no valid address after the coupling
    //  - `decoupledSubtreeRoot` gets the valid address of `decoupledSubtreePartRoot` in the node vector of the forest


    auto decoupledSubtreeIt = std::ranges::find(forest->Roots(), &decoupledSubtreeRoot);
    if (decoupledSubtreeIt != forest->Roots().end())
    {
        // decoupledSubtreeRoot is an actual root in the current tree

        // swap the Node objects
        std::swap(*decoupledSubtreePartRoot, decoupledSubtreeRoot);
        // fix pointer of children

        if (decoupledSubtreePartRoot->leftChild)
            decoupledSubtreePartRoot->leftChild->parent = decoupledSubtreePartRoot;
        if (decoupledSubtreePartRoot->rightChild)
            decoupledSubtreePartRoot->rightChild->parent = decoupledSubtreePartRoot;
        if (decoupledSubtreeRoot.leftChild)
            decoupledSubtreeRoot.leftChild->parent = &decoupledSubtreeRoot;
        if (decoupledSubtreeRoot.rightChild)
            decoupledSubtreeRoot.rightChild->parent = &decoupledSubtreeRoot;

        // fix order in root vector
        std::swap(*decoupledSubtreeIt, *decoupledSubtreePartIt);

        // The variable name of the subtree that we will attach to the parent tree
        // should remain `decoupledSubtreePartRoot`
        decoupledSubtreePartRoot = &decoupledSubtreeRoot;

        // fix position of the iterators
        // std::swap(decoupledSubtreeIt, decoupledSubtreePartIt);
    }

    // until here we just collected all necessary nodes etc
    // we still want to couple the subtree part with the smallest label
    // which is now `decoupledSubtreeRoot`
    // we will connect the children of `decoupledSubtreeRoot` with the `decouplingPoint`


    // undo edges
    decouplingPoint->leftChild = decoupledSubtreePartRoot->leftChild;
    decouplingPoint->rightChild = decoupledSubtreePartRoot->rightChild;
    if (decouplingPoint->leftChild)
    {
        decouplingPoint->leftChild->parent = decouplingPoint;
    }
    if (decouplingPoint->rightChild)
    {
        decouplingPoint->rightChild->parent = decouplingPoint;
    }

    // before updating the subtreeTerminal vector, we store which root node has a higher order
    bool decoupledSubtreeIsPartBigger = parentTreeRoot->hasSmallestTerminal(decoupledSubtreePartRoot);

    // update subtreeTerminals vector
    std::vector<u_int64_t> labels = decoupledSubtreePartRoot->subtreeTerminals;
    labels[(newLabel - 1) / 64] ^= (uint64_t) 1 << (newLabel -1) % 64;
    propagateXORLabelUp(labels, decouplingPoint);

    forest->LabelToTerminal().erase(newLabel);
    forest->TerminalToLabel().erase(decouplingPoint);

    // if the `decoupledSubtreePart` is a single-vertex tree, we need to maintain the maps
    if (decoupledSubtreeIt != forest->Roots().end() and forest->TerminalToLabel().contains(decoupledSubtreePartRoot))
    {
        forest->TerminalToLabel()[decouplingPoint] = forest->TerminalToLabel()[decoupledSubtreePartRoot];
        forest->TerminalToLabel().erase(decoupledSubtreePartRoot);

        // it might be necessary to update all labels that maps to `decoupledSubtreePartRoot`
        forest->LabelToTerminal()[forest->TerminalToLabel()[decouplingPoint]] = decouplingPoint;
    }

    // if the `decoupledSubtreePart` is a single-vertex tree, we need to maintain the maps
    if (decoupledSubtreeIt != forest->Roots().end() and forest->TerminalToLabel().contains(*decoupledSubtreeIt))
    {
        forest->TerminalToLabel()[decouplingPoint] = forest->TerminalToLabel()[*decoupledSubtreeIt];
        forest->TerminalToLabel().erase(*decoupledSubtreeIt);

        // it might be necessary to update all labels that maps to `decoupledSubtreePartRoot`
        forest->LabelToTerminal()[forest->TerminalToLabel()[decouplingPoint]] = decouplingPoint;
    }


    // undo roots
    if(decoupledSubtreeIsPartBigger)
    {
        forest->Roots().erase(decoupledSubtreePartIt);
    }
    else
    {
        *decoupledSubtreePartIt = parentTreeRoot;
        forest->Roots().erase(parentTreePartIt);
    }
}
