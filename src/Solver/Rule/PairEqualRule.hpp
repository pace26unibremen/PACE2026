#ifndef PACE2026_PAIR_EQUAL_RULE_HPP
#define PACE2026_PAIR_EQUAL_RULE_HPP

#include "../CollapseSubtreeAction.hpp"
#include "AbstractRule.hpp"

#include <stack>

namespace solver
{

class PairEqualRule : public AbstractRule
{
  protected:
    std::unordered_map<std::shared_ptr<graph::Forest>, int> forestToSubtree;
    std::stack<CollapseSubtreeAction> changes;

  public:
    /// \param context the context where in instance we can apply the rule\n
    /// 1. fst label
    /// 2. snd label
    /// 3. a mapping: forest to each subtree on the path that can be deleted
    PairEqualRule(const std::shared_ptr<graph::Instance>& instance,
                  const std::unordered_map<std::shared_ptr<graph::Forest>, int>& forestToSubtree);

    void apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);
};

}  //namespace solver

#endif  //PACE2026_PAIR_EQUAL_RULE_HPP
