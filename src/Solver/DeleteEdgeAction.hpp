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
  public:
    DeleteEdgeAction(int childIndex, const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_DELETE_EDGE_ACTION_HPP
