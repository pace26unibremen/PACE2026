#ifndef PACE2026_DEBUG_RULE_HPP
#define PACE2026_DEBUG_RULE_HPP

#include "AbstractRule.hpp"
#include "../../Graph/Instance.hpp"

namespace solver
{

class DebugTrivialSolutionRule : public AbstractRule
{
  protected:
    graph::Instance instanceBackUp;

  public:
    explicit DebugTrivialSolutionRule(const std::shared_ptr<graph::Instance>& instance);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);

};

}  //namespace solver

#endif  //PACE2026_DEBUG_RULE_HPP
