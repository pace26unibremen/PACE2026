#ifndef PACE2026_NODE_HPP
#define PACE2026_NODE_HPP

#include <vector>
#include <cstdint>

namespace graph
{

/// \brief Represents a node in a tree.
struct Node
{
    /// \brief Index of parent node.\n
    /// \c -1 indicates that \c this is the root node.
    int parentIndex = -1;

    /// \brief Index of the sibling.
    /// \c -1 indicates that \c this is the last sibling.
    int siblingIndex = -1;

    /// \brief Index of the first child in.
    /// \c -1 indicates that \c this has no first child.
    int leftChildIndex = -1;

    /// \brief Index of the second child in.
    /// \c -1 indicates that \c this has no second child.
    int rightChildIndex = -1;

    /// \brieg a bitmask that stores all terminals that in the subtree of this node
    std::vector<uint64_t> subtreeTerminals = {};

    /// \brief Constructor.
    /// \param parentIndex Index of parent.
    /// \param siblingIndex Index of the sibling.
    /// \param leftChildIndex Index of the left child.
    /// \param rightChildIndex Index of the second child.
    Node(int parentIndex, int siblingIndex, int leftChildIndex, int rightChildIndex);

    /// \brief Default Constructor.\n
    Node() = default;

    /// Checks if two nodes have the same terminals in their subtrees.
    /// \param other
    /// \return true if the terminals are identical, else false
    [[nodiscard]]
    bool hasSameTerminals(const Node& other) const;

    /// Checks if the terminals in the subtree of a node is a \b subset of
    /// the terminals of another node.
    /// \param other
    /// \return true if terminals of \c are a subset of the terminals of \c other, else false
    [[nodiscard]]
    bool hasSubsetTerminals(const Node& other) const;

    /// Checks if a node has a the minimal terminal in its subtree, compared to another node.
    /// \param other
    /// \return true if \c this has the minimal terminal, else false
    [[nodiscard]]
    bool hasSmallestTerminal(const Node& other) const;

};

}  // namespace graph

#endif  //PACE2026_NODE_HPP
