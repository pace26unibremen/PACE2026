#ifndef PACE2026_REVERSE_CASE_B_RULE_HPP
#define PACE2026_REVERSE_CASE_B_RULE_HPP

#include "../Action/DeleteEdgeAction.hpp"
#include "AbstractRule.hpp"

#include <stack>

namespace solver
{

/// \brief This rule informs the solver to cut the current branch,
/// if the current instance is already equal or worse to the current solution.
class ReverseCaseBRule : public  AbstractRule
{
  protected:

    unsigned int toCutLabel;

    /// \brief Stack of action that modify the instance,
    /// filled in the apply method and unfilled in the unapply method
    std::stack<DeleteEdgeAction> changes = std::stack<DeleteEdgeAction>();

public:
    ReverseCaseBRule(const std::shared_ptr<graph::Instance>& instance,
                  const std::shared_ptr<Context>& context,
                  const unsigned int& toCutLabel);

    /// \brief applies rule
    /// \returns always \ref RuleReturnCode::CutBranch
    RuleReturnCode apply() override;

    void unapply() override;

    /// \brief It checks whether the CutBranchRule is applicable and generates an instance of this rule if so.
    /// This method considers the CutBranchRule applicable if the `bestSolutionSize` of the `context`
    /// is at most the highest number of trees in a forest.
    /// \param instance on which the rule should be applied
    /// \param context contains additional information to the instance and the solver state
    /// \returns shared_pointer to CutBranchRule if rule is applicable, else null pointer
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_REVERSE_CASE_B_RULE_HPP
