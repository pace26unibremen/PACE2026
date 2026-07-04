#include "CollapseSubtreeAction.hpp"

#include "../Context.hpp"

solver::CollapseSubtreeAction::CollapseSubtreeAction(graph::Node* node, const std::shared_ptr<graph::Forest>& forest,
                                                     solver::Context* svtContext) :
    forest(forest),
    svtContext(svtContext),
    node(node)
{}

void solver::CollapseSubtreeAction::doAction()
{
    leftChild  = node->leftChild;
    rightChild = node->rightChild;

    node->leftChild = nullptr;
    node->rightChild = nullptr;

    unsigned int smallestLabel = 0;
    int k = 0;
    for(uint64_t bitmask : node->subtreeTerminals)
    {
        while (bitmask != 0) {
            unsigned i = __builtin_ctzll(bitmask);
            bitmask &= bitmask - 1;

            unsigned int label = 64 * k + i + 1;
            if(smallestLabel == 0)
            {
                smallestLabel = label;
            }

            graph::Node* oldNode = forest->LabelToTerminal()[label];
            collapsedLabelToTerminals.emplace(label, oldNode);
            forest->LabelToTerminal()[label] = node;
            if (forest->TerminalToLabel().contains(oldNode) and forest->TerminalToLabel().at(oldNode) == label)
            {
                collapsedTerminals.emplace(oldNode, label);
            }
            forest->TerminalToLabel().erase(oldNode);
        }
        k++;
    }
    collapsedLabel = smallestLabel;
    forest->TerminalToLabel().emplace(node, collapsedLabel);

    // If the collapsed node is a root it is now a childless root, i.e. a new
    // single-vertex tree for every label it represents. Register it so the SVT
    // tracking stays current (undoAction reverses exactly this).
    if (svtContext != nullptr and node->parent == nullptr)
    {
        svtContext->adjustSingleVertexForNode(node, +1);
        svtNodeCounted = true;
    }

    #ifdef DEBUG_IMAGE_VIEW_GRAPH
    forest->renderImage();
    #endif
}

void solver::CollapseSubtreeAction::undoAction()
{
    // the node may have changed (e.g., due to cluster reduction)
    node = forest->LabelToTerminal()[collapsedLabel];

    // Reverse the SVT bookkeeping first, while 'node' is still the childless
    // (collapsed) terminal that doAction counted.
    if (svtContext != nullptr and svtNodeCounted)
    {
        svtContext->adjustSingleVertexForNode(node, -1);
        svtNodeCounted = false;
    }

    node->leftChild = leftChild;
    node->rightChild = rightChild;
    leftChild->parent = node;
    rightChild->parent = node;

    for(const auto& [label, oldNode] : collapsedLabelToTerminals)
    {
        forest->LabelToTerminal()[label] = oldNode;
    }
    for ( const auto& [oldNode, label] : collapsedTerminals)
    {
        forest->TerminalToLabel().setLabel(oldNode, label);
    }

    forest->TerminalToLabel().erase(node);

    #ifdef DEBUG_IMAGE_VIEW_GRAPH
    forest->renderImage();
    #endif
}
