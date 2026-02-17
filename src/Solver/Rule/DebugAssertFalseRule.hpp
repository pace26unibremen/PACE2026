#ifndef PACE2026_DEBUG_ASSERT_FALSE_RULE_HPP
#define PACE2026_DEBUG_ASSERT_FALSE_RULE_HPP

#include "AbstractRule.hpp"

namespace solver
{

/// \brief This rule exists only for debugging purposes.
/// It can always be applied, but it terminates the run when applied.
class DebugAssertFalseRule : public AbstractRule
{
  public:
    explicit DebugAssertFalseRule(const std::shared_ptr<graph::Instance>& instance,
                                  const std::shared_ptr<Context>& context);

    /// \brief applies rule
    /// \returns always \ref RuleReturnCode::ImidateReturn
    RuleReturnCode apply() override;

    void unapply() override;

    /// \brief The DebugAssertFalseRule is always applicable.
    /// \returns shared_pointer to DebugAssertFalseRule
    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver

#endif  //PACE2026_DEBUG_ASSERT_FALSE_RULE_HPP
