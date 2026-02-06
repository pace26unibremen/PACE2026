#ifndef PACE2026_ABSTRACT_RULE_HPP
#define PACE2026_ABSTRACT_RULE_HPP

#include "../../Graph/Instance.hpp"
#include "Context.hpp"

namespace solver
{

/// \brief A rule changes a problem instance
/// (generally without changing the solution(-size)).
/// An instance of a rule holds information about the specific part of the problem instance that can be changed.
class AbstractRule
{
  protected:
    /// \brief The problem instance to be modified
    std::shared_ptr<graph::Instance> instance;

    /// \brief Context information about the instance and the solver state
    std::shared_ptr<Context> context;

    /// \brief Stores if the rule is already applied
    bool isApplied = false;

  public:
    /// constructor
    AbstractRule(const std::shared_ptr<graph::Instance>&, const std::shared_ptr<Context>& context);

    /// destructor
    virtual ~AbstractRule() = default;

    /// \brief applies rule
    virtual void apply() = 0;

    /// \brief reverts the changes of the rule
    virtual void unapply() = 0;

    /// \brief returns true if the rule is applied on the instance
    [[nodiscard, maybe_unused]]
    bool IsApplied() const;

    /// \brief name of the rule
    [[nodiscard]]
    virtual std::string name() const = 0;
};

} // namespace solver

#endif  //PACE2026_ABSTRACT_RULE_HPP
