#include "Node.hpp"

graph::Node::Node(int parentIndex, int siblingIndex, int firstChildIndex, int secondChildIndex) :
        parentIndex(parentIndex),
        siblingIndex(siblingIndex),
        firstChildIndex(firstChildIndex),
        secondChildIndex(secondChildIndex)
{}
