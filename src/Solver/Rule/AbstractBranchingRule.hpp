#ifndef PACE2026_ABSTRACT_BRANCHING_RULE_HPP
#define PACE2026_ABSTRACT_BRANCHING_RULE_HPP

#include "AbstractRule.hpp"

namespace solver
{

/// \brief Abstract base class for branching rules.
/// A branching rule defines how multiple branches are generated from the current instance.
/// When applying the rule, the solver divides the current problem into alternative subproblems (branches)
/// that are solved independently.
///
/// Technical the branching rules are \ref AbstractRule "AbstractRules",
/// and they work by repeatedly calling \ref apply and \ref unapply.
/// An apply opens a new branch and increases the \ref branch counter by one.
/// An unapply revokes the changes from the last apply and returns the instance to its previous state:
/// \code
/// auto rule = XYBranchingRule(...);
/// // imaginary branch 0
/// rule.apply();    // enter branch 1
/// rule.unapply();
/// rule.apply();    // enter branch 2
/// rule.unapply();
/// rule.apply();    // enter branch 3
/// rule.unapply();
/// \endcode
class AbstractBranchingRule : public AbstractRule
{
  protected:
    /// the current branch
    int branch = 0;
    /// the last branch / equals the number of branches
    const int maxBranch;

  public:
    /// \brief an abstract class for rules that branch the solution in \c _maxBranch different cases.
    /// \param _instance the problem instance
    /// \param _context information about the instance and the solver state
    /// \param _maxBranch the number of different branches of the rule
    explicit AbstractBranchingRule(const std::shared_ptr<graph::Instance>& _instance,
                                   const std::shared_ptr<Context>& _context,
                                   int _maxBranch);

    /// destructor
    ~AbstractBranchingRule() override = default;

    /// \brief Checks if all branches of this rule are processed
    [[nodiscard, maybe_unused]]
    virtual bool isFullyExplored() const;

    /// \brief the current branch
    [[nodiscard]]
    int Branch() const;

    /// \brief the last branch / equals the number of branches
    [[nodiscard]]
    int MaxBranch() const;
};

}  //namespace solver

#endif  //PACE2026_ABSTRACT_BRANCHING_RULE_HPP
