//
// Created by kaufm on 15.06.2026.
//

#include "AddEdgeAction.hpp"
#include <algorithm>

using namespace graph;
using namespace solver;


solver::AddEdgeAction::AddEdgeAction(graph::Node* child, graph::Node* parent,
                                     const std::shared_ptr<graph::Forest>& forest) :
        child(child),
        parent(parent),
        forest(forest),
        parentFreeSideIsLeft(false)
{}

void solver::AddEdgeAction::doAction()
{
    //Adds a new edge from the child to the parent
    //Assumption: The Edge is not present -> child has no parent and no sibling,
    //parent has at least one spot open
    if (!parent->leftChild) parentFreeSideIsLeft = true;
    const unsigned int subtreeTerminalsSize = child->subtreeTerminals.size();

    if (parentFreeSideIsLeft)
    {
        //Connect the nodes together
        child->parent = parent;
        parent->leftChild = child;
        child->sibling = parent->rightChild;
        parent->rightChild->sibling = child;

        //Update the subtree bitmask from the parent onwards
        auto traverseNode = parent;

        while (traverseNode)
        {
            for (int i = 0; i < subtreeTerminalsSize; i++)
            {
                //OR operant to add all subtreeTerminal bits to the parent
                traverseNode->subtreeTerminals[i] |= child->subtreeTerminals[i];
            }
            traverseNode = traverseNode->parent;
        }
    }
    else if (not parentFreeSideIsLeft)
    {
        //Connect the nodes together
        child->parent = parent;
        parent->rightChild = child;
        child->sibling = parent->leftChild;
        parent->leftChild->sibling = child;

        //Update the subtree bitmask from the parent onwards
        auto traverseNode = parent;
        while (traverseNode)
        {
            for (int i = 0; i < subtreeTerminalsSize; i++)
            {
                //OR operant to add all subtreeTerminal bits to the parent
                traverseNode->subtreeTerminals[i] |= child->subtreeTerminals[i];
            }
            traverseNode = traverseNode->parent;
        }
    }
}

void solver::AddEdgeAction::undoAction()
{
    const unsigned int subtreeTerminalsSize = child->subtreeTerminals.size();

    if (parentFreeSideIsLeft)
    {
        //Remove connection between the parent and the node
        parent->leftChild = nullptr;
        child->parent = nullptr;
        child->sibling = nullptr;
        parent->rightChild->sibling = nullptr;

        //Remove subtree bits of child in parent and its nodes above it.
        auto traverseNode = parent;
        while (traverseNode)
        {
            //XOR removal
            for (int i = 0; i < subtreeTerminalsSize; i++)
                traverseNode->subtreeTerminals[i] ^= child->subtreeTerminals[i];
            traverseNode = traverseNode->parent;
        }

    }
    else if (not parentFreeSideIsLeft)
    {
        //Remove connection between the parent and the node
        parent->rightChild = nullptr;
        child->parent = nullptr;
        child->sibling = nullptr;
        parent->leftChild->sibling = nullptr;

        //Remove subtree bits of child in parent and its nodes above it.
        auto traverseNode = parent;
        while (traverseNode)
        {
            //XOR removal
            for (int i = 0; i < subtreeTerminalsSize; i++)
                traverseNode->subtreeTerminals[i] ^= child->subtreeTerminals[i];
            traverseNode = traverseNode->parent;
        }
    }
}