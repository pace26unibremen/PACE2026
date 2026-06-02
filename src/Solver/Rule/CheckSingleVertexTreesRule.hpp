#ifndef PACE2026_CHECK_SINGLE_VERTEX_TREES_RULE_HPP
#define PACE2026_CHECK_SINGLE_VERTEX_TREES_RULE_HPP

#include "../Action/CollapseSubtreeAction.hpp"
#include "AbstractRule.hpp"

#include <stack>

namespace solver
{

/// \brief Collapses subtree of a terminal pair if it is a pair in every forest of the instance.
///
/// \see
/// <a href="https://gitlab.informatik.uni-bremen.de/pace-2026/orga/-/wikis/Branching/CheckSingleVertexTreesRule">
/// GitLab Documentation

/// </a>
class CheckSingleVertexTreesRule : public AbstractRule
{
  protected:
    /// \brief Stack of action that modify the instance,
    /// filled in the apply method and unfilled in the unapply method
    std::stack<CollapseSubtreeAction> changes = std::stack<solver::CollapseSubtreeAction>();

  public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    /// which is a mapping of each forest to the subtree in this forest that can be collapsed
    CheckSingleVertexTreesRule(const std::shared_ptr<graph::Instance>& instance,
                  const std::shared_ptr<Context>& context);

    /// \brief applies rule
    /// \returns always \ref RuleReturnCode::Continue
    RuleReturnCode apply() override;

    void unapply() override;

    /// \brief It checks whether the CheckSingleVertexTreesRule is applicable and generates an instance of this rule if so.
    /// This method only considers the CheckSingleVertexTreesRule applicable if the first pair found in forest 1
    /// has a corresponding pair of terminals in each of the other forests.
    /// \param instance on which the rule should be applied
    /// \param context contains additional information to the instance and the solver state
    /// \returns shared_pointer to CheckSingleVertexTreesRule if rule is applicable, else null pointer
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_CHECK_SINGLE_VERTEX_TREES_RULE_HPP
