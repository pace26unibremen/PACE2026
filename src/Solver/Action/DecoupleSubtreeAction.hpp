#ifndef PACE2026_DECOUPLE_SUBTREE_ACTION_HPP
#define PACE2026_DECOUPLE_SUBTREE_ACTION_HPP

#include "../../Graph/Forest.hpp"
#include "AbstractAction.hpp"

namespace solver
{

class DecoupleSubtreeAction : public solver::AbstractAction
{
  private:
    std::shared_ptr<graph::Forest> forest;

    graph::Node* decouplingPoint;

    graph::Node decoupledSubtreeRoot;

    unsigned int newLabel;

    int smallestLabelOfSubtree;

    static void propagateXORLabelUp(const std::vector<u_int64_t>& labels, graph::Node* start);

  public:
    DecoupleSubtreeAction(graph::Node* decouplingPoint, unsigned int newLabel,
                          const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_DECOUPLE_SUBTREE_ACTION_HPP
