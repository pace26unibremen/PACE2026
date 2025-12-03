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
}

void solver::CollapseSubtreeAction::undoAction()
{
    graph::Node& node = forest->Nodes()[nodeIndex];
    node.leftChildIndex = leftChildIndex;
    node.rightChildIndex = rightChildIndex;
}
