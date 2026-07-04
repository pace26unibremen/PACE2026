#ifndef PACE2026_NODE_HPP
#define PACE2026_NODE_HPP

#include <vector>
#include <cstdint>
#include <unordered_set>

namespace graph
{

/// \brief Represents a node in a tree.
struct Node
{
    /// \brief Pointer to parent node.\n
    /// \default is nullptr
    Node* parent = nullptr;

    /// \brief Pointer to the sibling.
    /// \default is nullptr
    Node* sibling = nullptr;

    /// \brief Pointer to the left child in.
    /// \default is nullptr
    Node* leftChild = nullptr;

    /// \brief Pointer to the right child in.
    /// \default is nullptr
    Node* rightChild = nullptr;

    /// \brief a bitmask that stores all terminals that in the subtree of this node
    /// \note Since there is no label '0', the bit (1 << i) encodes 'i+1', not 'i'.
    std::vector<uint64_t> subtreeTerminals = {};

    /// \brief The terminal label of this node, or 0 if the node is not currently a
    /// terminal (an internal node). For a collapsed node this is the smallest label
    /// of the collapsed subtree. This replaces the old terminalToLabel map; access
    /// it through Forest::TerminalToLabel() (a \ref graph::TerminalToLabelView).
    unsigned int label = 0;

    /// \brief Constructor.
    /// \param parent Pointer to parent.
    /// \param sibling Pointer to the sibling.
    /// \param leftChild Pointer to the left child.
    /// \param rightChild Pointer to the right child.
    Node(Node* parent, Node* sibling, Node* leftChild, Node* rightChild);

    /// \brief Default Constructor.
    Node() = default;

    /// Checks if two nodes have the same terminals in their subtrees.
    /// \param other
    /// \return true if the terminals are identical, else false
    [[nodiscard]]
    bool hasSameTerminals(const Node* other) const;

    /// Checks if a node has a terminal label in its subtree.
    /// \param label
    /// \return true if the subtreeTerminals list of this node contains the label
    [[nodiscard]]
    bool hasTerminal(unsigned int label) const;

    // hasSubsetTerminals / hasSmallestTerminal are defined inline here (rather than
    // in Node.cpp) because they are hot: hasSubsetTerminals drives getSiblings and
    // hasSmallestTerminal drives the child/root sorting in DeleteEdgeAction. With no
    // LTO in the build, keeping them out-of-line cost a cross-TU call each time.

    /// Checks if the terminals in the subtree of a node is a \b subset of
    /// the terminals of another node.
    /// \param other
    /// \return true if terminals of \c this are a subset of the terminals of \c other, else false
    [[nodiscard]]
    bool hasSubsetTerminals(const Node* other) const
    {
        for (unsigned int i = 0; i < subtreeTerminals.size(); i++)
        {
            if ((subtreeTerminals[i] | other->subtreeTerminals[i]) != other->subtreeTerminals[i])
            {
                return false;
            }
        }
        return true;
    }

    /// Checks if a node has the minimal terminal in its subtree, compared to another node.
    /// \param other
    /// \return true if \c this has the minimal terminal, else false
    [[nodiscard]]
    bool hasSmallestTerminal(const Node* other) const
    {
        for (unsigned int i = 0; i < subtreeTerminals.size(); i++)
        {
            if (subtreeTerminals[i] == other->subtreeTerminals[i])
            {
                continue;
            }
            // __builtin_ctzll counts trailing zeros
            return __builtin_ctzll(subtreeTerminals[i]) < __builtin_ctzll(other->subtreeTerminals[i]);
        }
        return false;
    }

    /// \brief returns the smallest label in the nodes' subtree.
    /// \returns the smallest label
    [[nodiscard, maybe_unused]]
    unsigned int smallestTerminal() const;

    /// \bried checks whether the node is a true terminal (and not a collapsed node that represents a whole subtree)
    [[nodiscard]]
    bool isTrueTerminal() const;

    /// \brief Calculates (!) all labels of the subtree as an unordered set.
    /// \returns the set of all subtree labels
    [[nodiscard]]
    std::unordered_set<unsigned int> SubtreeLabels() const;

};

}  // namespace graph

#endif  //PACE2026_NODE_HPP
