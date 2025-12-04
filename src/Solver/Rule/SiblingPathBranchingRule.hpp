#ifndef PACE2026_SIBLING_PATH_BRANCHING_RULE_HPP
#define PACE2026_SIBLING_PATH_BRANCHING_RULE_HPP

#include "../DeleteEdgeAction.hpp"
#include "AbstractBranchingRule.hpp"

#include <list>
#include <stack>

namespace solver
{

class SiblingPathBranchingRule : public AbstractBranchingRule
{
  protected:
    unsigned int label1;
    unsigned int label2;
    std::unordered_map<std::shared_ptr<graph::Forest>, std::list<int>> forestToPathDeletions;
    std::stack<DeleteEdgeAction> changes;

  public:
    /// \param context the context where in instance we can apply the rule\n
    /// 1. fst label
    /// 2. snd label
    /// 3. a mapping: forest to each subtree on the path that can be deleted
    SiblingPathBranchingRule(
        const std::shared_ptr<graph::Instance>& instance,
        const std::tuple<unsigned int, unsigned int,
                         std::unordered_map<std::shared_ptr<graph::Forest>, std::list<int>>>& context);

    void apply() override;

    void unapply() override;

    [[nodiscard, maybe_unused]]
    bool isFullyExplored() const override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);
};

}  //namespace solver

#endif  //PACE2026_SIBLING_PATH_BRANCHING_RULE_HPP
