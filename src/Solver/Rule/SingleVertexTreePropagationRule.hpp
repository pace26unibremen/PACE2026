#ifndef PACE2026_SINGLE_VERTEX_TREE_PROPAGATION_RULE_HPP
#define PACE2026_SINGLE_VERTEX_TREE_PROPAGATION_RULE_HPP

#include "AbstractRule.hpp"
#include "../Action/DeleteEdgeAction.hpp"
#include <unordered_set>
#include <stack>

namespace solver
{

class SingleVertexTreePropagationRule : public AbstractRule
{
  protected:
    std::unordered_set<unsigned int> labelsToBeReduced;
    std::stack<DeleteEdgeAction> changes;
  public:
    SingleVertexTreePropagationRule(const std::shared_ptr<graph::Instance>& instance,
                     const std::unordered_set<unsigned int>& labelsToBeReduced);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);

};

}  //namespace solver

#endif  //PACE2026_SINGLE_VERTEX_TREE_PROPAGATION_RULE_HPP
