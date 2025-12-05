#include "CollapseSubtreeAction.hpp"

solver::CollapseSubtreeAction::CollapseSubtreeAction(int nodeIndex, const std::shared_ptr<graph::Forest>& forest) :
    forest(forest),
    nodeIndex(nodeIndex)
{}

void solver::CollapseSubtreeAction::doAction()
{
    graph::Node& node = forest->Nodes()[nodeIndex];
    leftChildIndex  = node.leftChildIndex;
    rightChildIndex = node.rightChildIndex;

    node.leftChildIndex = -1;
    node.rightChildIndex = -1;

    unsigned int smallestLabel = 0;
    int k = 0;
    for(uint64_t bitmask : node.subtreeTerminals)
    {
        while (bitmask != 0) {
            unsigned i = __builtin_ctzll(bitmask);
            bitmask &= bitmask - 1;

            unsigned int label = 64 * k + i + 1;
            if(smallestLabel == 0)
            {
                smallestLabel = label;
            }

            int index = forest->LabelToTerminalIndex()[label];
            collapsedLabelToTerminals.emplace(label, index);
            forest->LabelToTerminalIndex()[label] = nodeIndex;
            forest->Terminals().erase(index);
        }
        k++;
    }
    forest->Terminals().emplace(nodeIndex, smallestLabel);
}

void solver::CollapseSubtreeAction::undoAction()
{
    graph::Node& node = forest->Nodes()[nodeIndex];
    node.leftChildIndex = leftChildIndex;
    node.rightChildIndex = rightChildIndex;

    for(const auto& [label, index] : collapsedLabelToTerminals)
    {
        forest->LabelToTerminalIndex()[label] = index;
        forest->Terminals()[index] = label;
    }
    forest->Terminals().erase(nodeIndex);
}
