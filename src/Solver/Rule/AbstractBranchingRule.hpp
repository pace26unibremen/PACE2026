#ifndef PACE2026_ABSTRACT_BRANCHING_RULE_HPP
#define PACE2026_ABSTRACT_BRANCHING_RULE_HPP

#include "AbstractRule.hpp"

namespace solver
{

class AbstractBranchingRule : public AbstractRule
{
  protected:
    int branch = 0;

  public:
    /// destructor
    ~AbstractBranchingRule() override = default;

    /// \brief Checks if all branches of this rule are processed
    [[nodiscard, maybe_unused]]
    virtual bool isFullyExplored() const = 0;
};

}  //namespace solver

#endif  //PACE2026_ABSTRACT_BRANCHING_RULE_HPP
