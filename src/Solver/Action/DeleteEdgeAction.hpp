#ifndef PACE2026_DELETE_EDGE_ACTION_HPP
#define PACE2026_DELETE_EDGE_ACTION_HPP

#include "AbstractAction.hpp"
#include "../../Graph/Forest.hpp"
#include "../../Graph/Node.hpp"

namespace solver
{

/// \brief Deletes an edge in a forest
class DeleteEdgeAction : AbstractAction
{
  protected:
    /// \brief forest on which the action will be performed
    std::shared_ptr<graph::Forest> forest;

    /// \brief the child on which the edge points
    graph::Node* child;

    /// \brief the sibling of the child
    graph::Node* sibling;

    /// \brief the parent of the child
    graph::Node* parent;

    /// \brief the left child of the parent (should be always the sibling or the child)
    graph::Node* left;

    /// \brief the right child of the parent (should be always the sibling or the child)
    graph::Node* right;

    /// \brief index of rootIndex in the roots vector - with smallest terminals
    /// Initialized with 0 to avoid unexpected behavior.
    unsigned int leftRoot_RootsIndex = 0;

    /// \brief Index of an entry in the `roots` vector of the forest.
    /// Points to one of the two affected roots (either the root of the new tree or the old tree).
    /// This is the index with the smaller order in the `roots` vector.
    /// Initialized with 0 to avoid unexpected behavior.
    unsigned int rightRoot_RootsIndex = 0;

    /// \brief whether the parent is a root node
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
    /// \param child A Pointer to the child node to which the edge points.
    /// \param forest A shared pointer to the forest on which the action will be performed.
    DeleteEdgeAction(graph::Node* child, const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_DELETE_EDGE_ACTION_HPP
