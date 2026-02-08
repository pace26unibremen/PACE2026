#ifndef PACE2026_PAIR_EQUAL_RULE_HPP
#define PACE2026_PAIR_EQUAL_RULE_HPP

#include "../Action/CollapseSubtreeAction.hpp"
#include "AbstractRule.hpp"

#include <stack>

namespace solver
{

/// \brief Collapses subtree of a terminal pair if it is a pair in every forest of the instance.
class PairEqualRule : public AbstractRule
{
  protected:
    /// \brief A mapping of each forest to the subtree in this forest that can be collapsed
    std::unordered_map<std::shared_ptr<graph::Forest>, graph::Node*> forestToSubtree;

    /// \brief Stack of action that modify the instance,
    /// filled in the apply method and unfilled in the unapply method
    std::stack<CollapseSubtreeAction> changes = std::stack<solver::CollapseSubtreeAction>();

  public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    /// \param forestToSubtree the position where the rule can be applied,\n
    /// which is a mapping of each forest to the subtree in this forest that can be collapsed
    PairEqualRule(const std::shared_ptr<graph::Instance>& instance,
                  const std::shared_ptr<Context>& context,
                  const std::unordered_map<std::shared_ptr<graph::Forest>, graph::Node*>& forestToSubtree);

    int apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_PAIR_EQUAL_RULE_HPP
