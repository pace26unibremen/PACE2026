#ifndef PACE2026_DELETE_EDGE_ACTION_HPP
#define PACE2026_DELETE_EDGE_ACTION_HPP

#include "AbstractAction.hpp"
#include "../Graph/Forest.hpp"

namespace solver
{

class DeleteEdgeAction : AbstractAction
{
  protected:
    std::shared_ptr<graph::Forest> forest;
    int childIndex;
    int siblingIndex;
    int parentIndex;
    int leftIndex;
    int rightIndex;
    unsigned int leftRoot_RootsIndex;
    unsigned int rightRoot_RootsIndex;
    bool parentIsRoot;

    graph::Node parentCopy;

    void doParentIsRoot();
    void doParentIsInner();
    void undoParentIsRoot();
    void undoParentIsInner();

  public:
    DeleteEdgeAction(int childIndex, const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_DELETE_EDGE_ACTION_HPP
