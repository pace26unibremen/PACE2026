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
    /// constructor
    AbstractBranchingRule(int _maxBranch);

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
