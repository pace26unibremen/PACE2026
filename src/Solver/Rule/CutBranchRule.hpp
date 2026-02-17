#ifndef PACE2026_CUT_BRANCH_RULE_HPP
#define PACE2026_CUT_BRANCH_RULE_HPP

#include "AbstractRule.hpp"

namespace solver
{

/// \brief This rule informs the solver to cut the current branch,
/// if the current instance is already equal or worse to the current solution.
class CutBranchRule : public  AbstractRule
{
  public:
    CutBranchRule(const std::shared_ptr<graph::Instance>& instance,
                  const std::shared_ptr<Context>& context);

    /// \brief applies rule
    /// \see AbstractRule::apply
    /// \returns always return code \c 2 (branch can be cutted)
    int apply() override;

    void unapply() override;

    /// \brief It checks whether the CutBranchRule is applicable and generates an instance of this rule if so.
    /// This method considers the CutBranchRule applicable if the `bestSolutionSize` of the `context`
    /// is at least the highest number of trees in a forest.
    /// \returns shared_pointer to CutBranchRule if rule is applicable, elso null pointer
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_CUT_BRANCH_RULE_HPP
