#ifndef PACE2026_ABSTRACT_RULE_HPP
#define PACE2026_ABSTRACT_RULE_HPP

#include "../../Graph/Instance.hpp"
#include "Context.hpp"
#include <list>

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
    /// \returns return code
    /// - \c 0 default, continue solving
    /// - \c 1 rule solves the instance
    /// - \c 2 branch can be cutted
    /// - \c 3 continue solving, but rule suggests further next rules
    /// - \c -1 stop the solver (without valid solution)
    virtual int apply() = 0;

    /// \brief reverts the changes of the rule
    virtual void unapply() = 0;

    /// \brief returns true if the rule is applied on the instance
    [[nodiscard, maybe_unused]]
    bool IsApplied() const;

    /// \brief name of the rule
    [[nodiscard]]
    virtual std::string name() const = 0;

    /// \brief A list of rules that should be applied next.
    ///
    /// If the \ref apply method returns \c 3,
    /// this indicates that the list contains items and the solver should adopt them.
    ///
    /// The first element of the list should be applied first, followed by the next element, and so on.
    virtual std::shared_ptr<std::list<std::shared_ptr<AbstractRule>>> NextRuleSuggestion();
};

} // namespace solver

#endif  //PACE2026_ABSTRACT_RULE_HPP
