#include "Node.hpp"

#include <cassert>

graph::Node::Node(Node* parent, Node* sibling, Node* leftChild, Node* rightChild) :
        parent(parent),
        sibling(sibling),
        leftChild(leftChild),
        rightChild(rightChild)
{}

bool graph::Node::hasSameTerminals(const Node* other) const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if(subtreeTerminals[i] != other->subtreeTerminals[i])
        {
            return false;
        }
    }
    return true;
}

bool graph::Node::hasTerminal(unsigned int label) const
{
    return (subtreeTerminals[(label -1) / 64] & (uint64_t) 1 << (label -1) % 64) > 0;
}

bool graph::Node::hasSmallestTerminal(const graph::Node* other) const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if(subtreeTerminals[i] == other->subtreeTerminals[i])
        {
            continue;
        }
        else if(__builtin_ctzll(subtreeTerminals[i]) < __builtin_ctzll(other->subtreeTerminals[i]))
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

bool graph::Node::hasSubsetTerminals(const graph::Node* other) const
{
    for(unsigned int i = 0; i < subtreeTerminals.size(); i++ )
    {
        if((subtreeTerminals[i] | other->subtreeTerminals[i]) != other->subtreeTerminals[i])
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

bool graph::Node::isTrueTerminal() const
{
    bool foundLabel = false;
    for (unsigned long value : subtreeTerminals)
    {
        if (value == 0)
        {
            continue;
        }
        if (not foundLabel)
        {
            uint64_t t = value & -value;
            value ^= t;
            foundLabel = true;
        }
        if (value != 0)
        {
            return false;
        }
    }
    return true;
}

std::unordered_set<unsigned int> graph::Node::SubtreeLabels() const
{
    std::unordered_set<unsigned int> result;
    for (size_t block = 0; block < subtreeTerminals.size(); ++block) {
        uint64_t value = subtreeTerminals[block];

        while (value != 0) {
            uint64_t t = value & -value;
            int bit_index = __builtin_ctzll(value);
            unsigned int number = block * 64 + bit_index + 1;
            result.insert(number);
            value ^= t;
        }
    }
    return result;
}