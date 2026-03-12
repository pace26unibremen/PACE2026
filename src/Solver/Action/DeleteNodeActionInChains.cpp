#include "DeleteNodeActionInChains.hpp"
#include <iostream>

using namespace graph;
using namespace solver;

solver::DeleteNodeActionInChains::DeleteNodeActionInChains(graph::Node* node, const std::shared_ptr<graph::Forest>& forest):
    node(*node),
    forest(forest)
{   //Determine the child terminal of the param node
    graph::Node* leftSide = node->leftChild;

    if (leftSide->leftChild == nullptr && leftSide->rightChild == nullptr )
    {
        terminalOnRightSide = false;
    }
    else
    {
        terminalOnRightSide = true;
    }

    //Assign reference to the child
    if (terminalOnRightSide)
    {
        toBeRemovedChildNode = node->rightChild;
    }
    else
    {
        toBeRemovedChildNode =  node->leftChild;
    }
}

void solver::DeleteNodeActionInChains::doAction()
{
    std::cout << "Node to be deleted :" << std::addressof(node) << std::endl;
    std::vector<Node> nodes = forest->Nodes();
    std::unordered_map<graph::Node*,unsigned int> terminals= forest->TerminalToLabel();
    std::unordered_map<unsigned int, graph::Node*> labels = forest->LabelToTerminal();

    graph::Node* parent = node.parent;
    std::cout << "Parent node to be deleted :" << std::addressof(parent) << std::endl;
    //Remove Child terminal from node
    //Done because removing child node first is easier.

    //Determine Index of child reference within Nodes for deletion
    for (int index=0; index<nodes.size(); index++)
    {   //
        graph::Node* currentNode = &nodes.at(index);
        if (currentNode == toBeRemovedChildNode)
        {
            nodes.erase(nodes.begin()+index-1);
            break;
        }
    }
    //Delete terminal reference within the terminal list
    for (const auto& terminal : terminals)
    {
        if (terminal.first == toBeRemovedChildNode)
        {
            terminals.erase(toBeRemovedChildNode);
            break;
        }
    }
    //Delete label reference within the terminal list
    for (const auto& label : labels)
    {
        if (label.second == toBeRemovedChildNode)
        {
            labels.erase(label.first);
            break;
        }
    }

    //Remove param node now
    //First, change remaining child and parent of param node
    graph::Node* remainingChild;

    if (terminalOnRightSide)
    {
        remainingChild = node.leftChild;
        parent->leftChild = remainingChild;
        remainingChild->parent = parent;

    }
    else
    {
        remainingChild = node.rightChild;
        parent->rightChild = remainingChild;
        remainingChild->parent = parent;
    }

    //Remove the node now.
    for (int index=0; index<nodes.size(); index++)
    {   //
        graph::Node* currentNode = &nodes.at(index);
        if (currentNode == &node)
        {
            nodes.erase(nodes.begin()+index-1);
            break;
        }
    }
    //Since node cannot be a terminal-> no need for terminal and label.
    //End
}

void solver::DeleteNodeActionInChains::undoAction()
{
    //Reintroduce the nodes back in and fix back the edge changes back to the original node
    forest->Nodes().push_back(node);

    if (terminalOnRightSide)
    {
        forest->Nodes().push_back(*node.rightChild);
        graph::Node* parent = node.parent;
        graph::Node* leftChild = node.leftChild;

        leftChild->parent = &node;
        parent->leftChild = &node;

    }
    else
    {
        forest->Nodes().push_back(*node.leftChild);
        graph::Node* parent = node.parent;
        graph::Node* rightChild = node.rightChild;
        rightChild->parent = &node;
        parent->rightChild = &node;
    }
}
