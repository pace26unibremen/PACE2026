#ifndef PACE2026_NODE_HPP
#define PACE2026_NODE_HPP

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
    int firstChildIndex = -1;

    /// \brief Index of the second child in.
    /// \c -1 indicates that \c this has no second child.
    int secondChildIndex = -1;

    /// \brief Constructor.
    /// \param parentIndex Index of parent.
    /// \param siblingIndex Index of the sibling.
    /// \param firstChildIndex Index of the first child.
    /// \param secondChildIndex Index of the second child.
    Node(int parentIndex, int siblingIndex, int firstChildIndex, int secondChildIndex);

    /// \brief Default Constructor.\n
    Node() = default;
};

}  // namespace graph

#endif  //PACE2026_NODE_HPP
