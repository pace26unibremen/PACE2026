#include "Node.hpp"

#include <cassert>

graph::Node::Node(int parentIndex, int siblingIndex, int leftChildIndex, int rightChildIndex) :
        parentIndex(parentIndex),
        siblingIndex(siblingIndex),
        leftChildIndex(leftChildIndex),
        rightChildIndex(rightChildIndex)
{}

bool graph::Node::hasSameTerminals(const Node& other) const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if(subtreeTerminals[i] != other.subtreeTerminals[i])
        {
            return false;
        }
    }
    return true;
}

bool graph::Node::hasSmallestTerminal(const graph::Node& other) const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if(subtreeTerminals[i] == other.subtreeTerminals[i])
        {
            continue;
        }
        else if(__builtin_ctzll(subtreeTerminals[i]) < __builtin_ctzll(other.subtreeTerminals[i]))
        {
            // __builtin_ctzll counts trailing zeros
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool graph::Node::hasSubsetTerminals(const graph::Node& other) const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if((subtreeTerminals[i] | other.subtreeTerminals[i]) != other.subtreeTerminals[i])
        {
            return false;
        }
    }
    return true;
}

unsigned int graph::Node::smallestTerminal() const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if(subtreeTerminals[i] == 0)
        {
            continue;
        }
        return (64 * i + __builtin_ctzll(subtreeTerminals[i]) + 1);
    }
    assert(false);
}
