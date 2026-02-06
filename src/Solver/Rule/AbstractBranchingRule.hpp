#ifndef PACE2026_ABSTRACT_BRANCHING_RULE_HPP
#define PACE2026_ABSTRACT_BRANCHING_RULE_HPP

#include "AbstractRule.hpp"

namespace solver
{

class AbstractBranchingRule : public AbstractRule
{
  protected:
    int branch = 0;
    const int maxBranch;

  public:
    /// \brief a abstract class for rules that branch the solution in \c _macBranch different cases.
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

    int Branch() const;

    int MaxBranch() const;
};

}  //namespace solver

#endif  //PACE2026_ABSTRACT_BRANCHING_RULE_HPP
