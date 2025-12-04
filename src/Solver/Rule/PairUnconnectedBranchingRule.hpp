#ifndef PACE2026_PAIR_UNCONNECTED_BRANCHING_RULE_HPP
#define PACE2026_PAIR_UNCONNECTED_BRANCHING_RULE_HPP

#include "AbstractRule.hpp"
#include "../DeleteEdgeAction.hpp"

#include <list>
#include <stack>

namespace solver
{

/// \brief
class PairUnconnectedBranchingRule : public AbstractRule
{
  protected:
    unsigned int label1;
    unsigned int label2;
    std::list<std::shared_ptr<graph::Forest>> forestsConnectedLabels;
    unsigned int branch = 0;
    std::stack<DeleteEdgeAction> changes;

  public:
    /// \param context the context where in instance we can apply the rule\n
    /// 1. fst label
    /// 2. snd label
    /// 3. list of forests that are affected.
    PairUnconnectedBranchingRule(
        const std::shared_ptr<graph::Instance>& instance,
        const std::tuple<unsigned int, unsigned int,
                         std::list<std::shared_ptr<graph::Forest>>>& context);

    void apply() override;

    void unapply() override;

    /// \brief Checks if all branches of this rule are processed
    [[nodiscard, maybe_unused]]
    bool isFullyExplored() const;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance);
};

}  //namespace solver

#endif  //PACE2026_PAIR_UNCONNECTED_BRANCHING_RULE_HPP
