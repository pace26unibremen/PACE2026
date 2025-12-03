#ifndef PACE2026_ABSTRACT_RULE_HPP
#define PACE2026_ABSTRACT_RULE_HPP

#include "../../Graph/Instance.hpp"

namespace solver
{

/// \brief A rule changes a problem instance
/// (generally without changing the solution(-size)).
/// An instance of a rule holds the specific context what can be changed in the problem instance.
template <typename Derived>
class AbstractRule
{
  protected:
    /// \brief The problem instance to be modified
    std::shared_ptr<graph::Instance> instance;

  public:
    /// \brief applies rule
    virtual void apply() = 0;

    /// \brief reverts the changes of the rule
    virtual void unapply() = 0;

    /// \brief Checks if the rule is applicable
    /// \return a pointer to instance of the rule, that can be applied, if the it is applicable\n
    /// otherwise a null pointer
    static std::shared_ptr<Derived> isApplicable(std::shared_ptr<graph::Instance> instance)
    {
        return Derived::isApplicable(instance);
    }
};

} // namespace solver

#endif  //PACE2026_ABSTRACT_RULE_HPP
