#ifndef PACE2026_ABSTRACT_PLUGIN_HPP
#define PACE2026_ABSTRACT_PLUGIN_HPP

#include "../Rule/AbstractRule.hpp"

#include <memory>

namespace solver::plugin
{

/// \brief Abstract base class for branching solver plugins.
///
/// Plugins observe solver events through a set of virtual hooks. All hooks
/// have default no-op implementations so subclasses only override what they need.
///
/// The solver holds a list of plugins and invokes each hook on every plugin
/// after the corresponding solver event occurs.
class AbstractPlugin
{
  public:
    virtual ~AbstractPlugin() = default;

    /// \brief Called once before solving starts.
    /// \param instance the problem instance that will be solved
    virtual void init(const std::shared_ptr<graph::Instance>& instance);

    /// \brief Called immediately before each rule application.
    virtual void beforeApply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief Called after each rule application.
    virtual void onApply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief Called after each rule unapplication during branch rollback.
    virtual void onUnapply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief Called when a reduction rule is temporarily unapplied to expose a solution candidate.
    /// \param lastRule true if this is the last reduction being unapplied in this pass
    virtual void onTempUnapply(const std::shared_ptr<solver::AbstractRule>& rule, bool lastRule);

    /// \brief Called when a reduction rule is re-applied after a solution candidate was recorded.
    virtual void onTempApply(const std::shared_ptr<solver::AbstractRule>& rule);

    /// \brief Called when a new best solution is found.
    /// \param score number of roots (trees) in the new best solution
    virtual void onNewBestSolution(std::size_t score);

    /// \brief Called once when solving is complete.
    virtual void onEnd();
};

} // namespace solver::plugin

#endif  //PACE2026_ABSTRACT_PLUGIN_HPP
