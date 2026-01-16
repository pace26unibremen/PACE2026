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
    Node* parent = nullptr;

    /// \brief Index of the sibling.
    /// \c -1 indicates that \c this is the last sibling.
    Node* sibling = nullptr;

    /// \brief Index of the left child in.
    /// \c -1 indicates that \c this has no left child.
    Node* leftChild = nullptr;

    /// \brief Index of the right child in.
    /// \c -1 indicates that \c this has no right child.
    Node* rightChild = nullptr;

    /// \brief a bitmask that stores all terminals that in the subtree of this node
    std::vector<uint64_t> subtreeTerminals = {};

    /// \brief Constructor.
    /// \param parent Index of parent.
    /// \param sibling Index of the sibling.
    /// \param leftChild Index of the left child.
    /// \param rightChild Index of the right child.
    Node(Node* parent, Node* sibling, Node* leftChild, Node* rightChild);

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
    /// \return true if terminals of \c this are a subset of the terminals of \c other, else false
    [[nodiscard]]
    bool hasSubsetTerminals(const Node& other) const;

    /// Checks if a node has a the minimal terminal in its subtree, compared to another node.
    /// \param other
    /// \return true if \c this has the minimal terminal, else false
    [[nodiscard]]
    bool hasSmallestTerminal(const Node& other) const;

    /// \brief returns the smallest terminal in the nodes subtree.
    [[nodiscard, maybe_unused]]
    unsigned int smallestTerminal() const;

};

}  // namespace graph

#endif  //PACE2026_NODE_HPP
