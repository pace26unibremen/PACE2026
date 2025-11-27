#include "CollapseSubtreeAction.hpp"

solver::CollapseSubtreeAction::CollapseSubtreeAction(int nodeIndex, const std::shared_ptr<graph::Forest>& forest) :
    forest(forest),
    nodeIndex(nodeIndex)
{}

void solver::CollapseSubtreeAction::doAction()
{
    graph::Node& node = forest->Nodes()[nodeIndex];
    leftChildIndex  = node.firstChildIndex;
    rightChildIndex = node.secondChildIndex;

    node.firstChildIndex = -1;
    node.secondChildIndex = -1;
}

void solver::CollapseSubtreeAction::undoAction()
{
    graph::Node& node = forest->Nodes()[nodeIndex];
    node.firstChildIndex = leftChildIndex;
    node.secondChildIndex = rightChildIndex;
}
