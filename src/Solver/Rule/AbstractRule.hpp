#ifndef PACE2026_ABSTRACT_RULE_HPP
#define PACE2026_ABSTRACT_RULE_HPP

#include "../../Graph/Instance.hpp"

namespace solver
{

/// \brief A rule changes a problem instance
/// (generally without changing the solution(-size)).
/// An instance of a rule holds the specific context what can be changed in the problem instance.
class AbstractRule
{
  protected:
    /// \brief The problem instance to be modified
    std::shared_ptr<graph::Instance> instance;

    /// \brief Stores if the rule is already applied
    bool isApplied = false;

  public:
    /// destructor
    virtual ~AbstractRule() = default;

    /// \brief applies rule
    virtual void apply() = 0;

    /// \brief reverts the changes of the rule
    virtual void unapply() = 0;

    /// \brief returns true if the rule is applied on the instance
    [[nodiscard, maybe_unused]]
    bool IsApplied() const;

};

} // namespace solver

#endif  //PACE2026_ABSTRACT_RULE_HPP
