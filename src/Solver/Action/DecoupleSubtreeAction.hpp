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

    graph::Node* subtreeRoot;

    graph::Node newNode;

    int newLabel;

    int smallestLabelOfSubtree;

    unsigned int indexOfNewRoot;

    unsigned int indexOfOldRoot;

  public:
    DecoupleSubtreeAction(graph::Node* subtreeRoot, unsigned int newLabel,
                          const std::shared_ptr<graph::Forest>& forest);

    void doAction() override;

    void undoAction() override;
};

}  //namespace solver

#endif  //PACE2026_DECOUPLE_SUBTREE_ACTION_HPP
