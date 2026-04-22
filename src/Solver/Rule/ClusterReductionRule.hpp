#ifndef PACE2026_CLUSTER_REDUCTION_RULE_HPP
#define PACE2026_CLUSTER_REDUCTION_RULE_HPP

#include "AbstractRule.hpp"

#include <stack>
#include "../Action/DecoupleSubtreeAction.hpp"


namespace solver
{

class ClusterReductionRule : public AbstractRule
{
  private:
    std::list<std::list<std::pair<std::shared_ptr<graph::Forest>, graph::Node*>>> pointsAndForests_PerCluster;

    std::stack<DecoupleSubtreeAction> changes = std::stack<DecoupleSubtreeAction>();

  public:
    /// \param instance the problem instance
    /// \param context information about the instance and the solver state
    ClusterReductionRule(
        const std::shared_ptr<graph::Instance>& instance, 
        const std::shared_ptr<Context>& context,
        const std::list<std::list<std::pair<std::shared_ptr<graph::Forest>, graph::Node*>>>& pointsAndForests_PerCluster);

    RuleReturnCode apply() override;

    void unapply() override;

    static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

    [[nodiscard]]
    std::string name() const override;
};

}  //namespace solver
#endif  //PACE2026_CLUSTER_REDUCTION_RULE_HPP
