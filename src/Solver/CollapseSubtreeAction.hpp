#ifndef PACE2026_Collapse_SUBTREE_ACTION_HPP
#define PACE2026_Collapse_SUBTREE_ACTION_HPP

#include "AbstractAction.hpp"
#include "../Graph/Forest.hpp"

namespace solver
{

/// \brief collapses a subtree into a single new leaf node
class CollapseSubtreeAction : AbstractAction
{
  private:
    /// \brief forest on which the action will be performed
    std::shared_ptr<graph::Forest> forest;

    /// \brief index of the node from which the subtree will be collapsed into single leaf
    int nodeIndex;

    /// \brief left child of node
    int leftChildIndex;

    /// \brief left child of node
    int rightChildIndex;

  public:
    /// \param nodeIndex The index of the node from which the subtree will be collapsed into single leaf.
    /// \param forest A shared pointer to the forest on which the action will be performed.
    CollapseSubtreeAction(int nodeIndex, const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;

};

}  //namespace solver

#endif  //PACE2026_Collapse_SUBTREE_ACTION_HPP
