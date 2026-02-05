//
// Created by kaufm on 25.01.2026.
//

#include "DeleteNodeActionInChains.h"
using namespace graph;
using namespace solver;

solver::DeleteNodeActionInChains::DeleteNodeActionInChains(graph::Node node, const std::shared_ptr<graph::Forest>& forest):
    node(node),
    forest(forest)
{   //Bestimme Seite, welche entfernt wird anhand davon, ob es den Blatt hat.
    int leftSideIndex = node.leftChildIndex;

    if (forest->Nodes().at(leftSideIndex).leftChildIndex == -1 && forest->Nodes().at(leftSideIndex).rightChildIndex == -1)
    {
        terminalOnRightSide = false;
    }
    else
    {
        terminalOnRightSide = true;
    }
    //Bestimme den Index des Blattes.
    if (terminalOnRightSide)
    {
        toBeRemovedChildNodeIndex = forest->Nodes().at(leftSideIndex).rightChildIndex;
        childNode = forest->Nodes().at(toBeRemovedChildNodeIndex);
    }
    else
    {
        toBeRemovedChildNodeIndex = forest->Nodes().at(leftSideIndex).leftChildIndex;
        childNode = forest->Nodes().at(toBeRemovedChildNodeIndex);
    }
}
 //namespace solver

void solver::DeleteNodeActionInChains::doAction()
{
    std::vector<Node> nodes = forest->Nodes();
    std::unordered_map<int,unsigned int> termIndexTree= forest->Terminals();
    std::unordered_map<unsigned int, int> labels = forest->LabelToTerminalIndex();

    int parentIndex = node.parentIndex;
    int siblingIndex = node.siblingIndex;
    int leftChildIndex = node.leftChildIndex;
    int rightChildIndex = node.rightChildIndex;

    int nodeIndex;
    //Change every related nodes befitting of the removal

    //Determine the actual index of the node
    if (leftChildIndex >= 0)
    {
        nodeIndex = nodes.at(leftChildIndex).parentIndex;
    }
    else if (rightChildIndex >= 0)
    {
        nodeIndex = nodes.at(rightChildIndex).parentIndex;
    }
    else if (siblingIndex >= 0)
    {
        nodeIndex = nodes.at(siblingIndex).siblingIndex;
    }
    else
    {
        if (nodes.at(parentIndex).leftChildIndex == -1 && nodes.at(parentIndex).rightChildIndex != -1)
        {
            nodeIndex = nodes.at(parentIndex).rightChildIndex;

        }
        else if (nodes.at(parentIndex).rightChildIndex == -1 && nodes.at(parentIndex).leftChildIndex != -1)
        {
            nodeIndex = nodes.at(parentIndex).leftChildIndex;

        }
    }

    //Remove child node out of Nodes List
    forest->Nodes().erase(forest->Nodes().begin() + toBeRemovedChildNodeIndex -1);

    int toBeRemovedTerminalIndex;

    for(const auto& pair: forest->Terminals())
    {
        if (pair.first == toBeRemovedChildNodeIndex)
        {
            toBeRemovedTerminalIndex = pair.first;
            break;

        }
    }

    //Remove the child node in the terminal list(-s)
    forest->Terminals().erase(toBeRemovedTerminalIndex);

    int toBeRemovedLabelIndex;
    for (const auto& labelPair : forest->LabelToTerminalIndex())
    {
        if (labelPair.second == toBeRemovedChildNodeIndex)
        {
            toBeRemovedLabelIndex = labelPair.second;
            break;
        }
    }

    forest->LabelToTerminalIndex().erase(toBeRemovedLabelIndex);

    //Child terminal now removed, onto main node from param
    //Reconnect edge between the root/parent node of param node and the not-empty side
    if (terminalOnRightSide)
    {
        //Set parent left child to be the left child of the param node
        forest->Nodes().at(parentIndex).leftChildIndex = forest->Nodes().at(nodeIndex).leftChildIndex;
        //Set parent node of param nodes left child to be the parent node of the param node
        forest->Nodes().at(forest->Nodes().at(nodeIndex).leftChildIndex).parentIndex =
            forest->Nodes().at(nodeIndex).parentIndex;
    }
    else
    {
        forest->Nodes().at(parentIndex).rightChildIndex = forest->Nodes().at(nodeIndex).rightChildIndex;
        forest->Nodes().at(forest->Nodes().at(nodeIndex).rightChildIndex).parentIndex =
            forest->Nodes().at(nodeIndex).parentIndex;
    }
    //Delete the node now
    forest->Nodes().erase(forest->Nodes().begin() + nodeIndex -1);


}

void solver::DeleteNodeActionInChains::undoAction()
{
    //
    forest->Nodes().push_back(node);

    //Determine the index of parent node and child nodes of the param node (They've most likely changed)

    int parentIndex;
    int leftChildIndex;
    int rightChildIndex;

    //Determine parent node index
    for (const auto& node : forest->Nodes())
    {
        if (node.parentIndex == parentIndex)
        {
            parentIndex = node.parentIndex;
            break;
        }
    }

    if (terminalOnRightSide)
    {
        //Blatt Rechts -> Attachment links
        //Parent node

    }
    else
    {
        //Blatt links -> Attachment rechts
    }
}
