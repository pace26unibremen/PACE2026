#ifndef PACE2026_DELETE_EDGE_ACTION_HPP
#define PACE2026_DELETE_EDGE_ACTION_HPP

#include "AbstractAction.hpp"
#include "../../Graph/Forest.hpp"

namespace solver
{

/// \brief Deletes an edge in a forest
class DeleteEdgeAction : AbstractAction
{
  protected:
    /// \brief forest on which the action will be performed
    std::shared_ptr<graph::Forest> forest;

    /// \brief the child on which the edge points
    int childIndex;

    /// \brief the sibling of the child
    int siblingIndex;

    /// \brief the parent of the child
    int parentIndex;

    /// \brief the left child of the parent (should be always the sibling or the child)
    int leftIndex;

    /// \brief the right child of the parent (should be always the sibling or the child)
    int rightIndex;

    /// \brief index of rootIndex int the rootIndices vector - with smallest terminals
    unsigned int leftRoot_RootsIndex;

    /// \brief Index of an entry in the `rootIndices` vector of the forest.
    /// Points to one of the two affected root indices (either the root of the new tree or the old tree).
    /// This is the index with the smaller order in the `rootIndices` vector.
    unsigned int rightRoot_RootsIndex;

    /// \brief Index of an entry in the rootIndices vector of forest.
    /// Points to one of the two affected root indices (either the root of the new tree or the old tree).
    /// This is the index with the higher order in the `rootIndices` vector.
    bool parentIsRoot;

    /// \brief a copy of the parent node (can be removed when the new isValid method is active)
    graph::Node parentCopy;

    /// \brief do methode for the 'parent is a root node' case
    void doParentIsRoot();

    /// \brief do methode for the 'parent is not a root node' case
    void doParentIsInner();

    /// \brief undo methode for the 'parent was a root node' case
    void undoParentIsRoot();

    /// \brief undo methode for the 'parent was a root node' case
    void undoParentIsInner();

  public:
    /// \param childIndex The index of the child node to which the edge points.
    /// \param forest A shared pointer to the forest on which the action will be performed.
    DeleteEdgeAction(int childIndex, const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_DELETE_EDGE_ACTION_HPP
