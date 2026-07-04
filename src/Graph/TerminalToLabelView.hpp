#ifndef PACE2026_TERMINAL_TO_LABEL_VIEW_HPP
#define PACE2026_TERMINAL_TO_LABEL_VIEW_HPP

#include "Node.hpp"

#include <cstddef>

namespace graph
{

/// \brief Presents the old \c terminalToLabel (Node* -> label) map interface, but
/// backed directly by \ref Node::label (0 == not a terminal).
///
/// The labels live on the nodes, so a node lookup is a plain field read instead of
/// a hash lookup, labels are copied for free when the node vector is copied, and
/// cluster sub-forests that share nodes share the labels naturally. Only the number
/// of terminals is stored here, so that \c size() stays O(1) (it is read on the hot
/// path by CheckSingleVertexTreesRule). The count is kept correct by routing every
/// label mutation through \c emplace / \c erase / \c setLabel.
class TerminalToLabelView
{
    std::size_t count_ = 0;

  public:
    TerminalToLabelView() = default;
    explicit TerminalToLabelView(std::size_t count) : count_(count) {}

    /// \brief Whether \p node is currently a terminal. Null-safe: the old
    /// unordered_map returned false for a null (absent) key, and several call sites
    /// pass a possibly-null sibling/parent pointer.
    [[nodiscard]] bool contains(const Node* node) const { return node != nullptr and node->label != 0; }

    /// \brief The label of \p node (0 if it is not a terminal).
    [[nodiscard]] unsigned int at(const Node* node) const { return node->label; }

    /// \brief Number of terminals.
    [[nodiscard]] std::size_t size() const { return count_; }

    /// \brief Mirrors \c unordered_map::emplace: sets the label only if \p node is
    /// not already a terminal.
    void emplace(Node* node, unsigned int label)
    {
        if (node->label == 0)
        {
            node->label = label;
            ++count_;
        }
    }

    /// \brief Removes \p node as a terminal.
    void erase(Node* node)
    {
        if (node->label != 0)
        {
            node->label = 0;
            --count_;
        }
    }

    /// \brief Count-safe assignment, replacing the former \c map[node] = label writes.
    void setLabel(Node* node, unsigned int label)
    {
        if (node->label == 0 and label != 0)
        {
            ++count_;
        }
        else if (node->label != 0 and label == 0)
        {
            --count_;
        }
        node->label = label;
    }

    /// \brief No-op; kept so the old \c reserve() call sites compile unchanged.
    void reserve(std::size_t) {}
};

}  // namespace graph

#endif  // PACE2026_TERMINAL_TO_LABEL_VIEW_HPP
