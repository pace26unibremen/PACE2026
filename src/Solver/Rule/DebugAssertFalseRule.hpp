#ifndef PACE2026_DEBUG_ASSERT_FALSE_RULE_HPP
#define PACE2026_DEBUG_ASSERT_FALSE_RULE_HPP

#include "../../Graph/Instance.hpp"
#include "AbstractRule.hpp"

namespace solver
{

class DebugAssertFalseRule : public AbstractRule
{
  protected:
    std::string pathToWriteInstance = "./DebugAssertFalseRule_instance.nw";

  public:
    explicit DebugAssertFalseRule(const std::shared_ptr<graph::Instance>& instance);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);
};

}  //namespace solver

#endif  //PACE2026_DEBUG_ASSERT_FALSE_RULE_HPP
