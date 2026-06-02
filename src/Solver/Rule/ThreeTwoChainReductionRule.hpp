//
// Created by kaufm on 12.02.2026.
//

#ifndef PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
#define PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
#include "AbstractRule.hpp"
#include <vector>
#include <stack>
#include "..\Action\AbstractAction.hpp"


namespace solver
{

    class ThreeTwoChainReductionRule : public AbstractRule
    {
    protected:
        std::pair<graph::Node*,std::vector<std::shared_ptr<graph::Forest>>> nodeAndTrees;

        unsigned int nodeLabel;

        std::stack<solver::AbstractAction> changes;

    public:
      ThreeTwoChainReductionRule(
          const std::shared_ptr<graph::Instance>& instance, const std::shared_ptr<Context>& context,
          const std::pair<graph::Node*, std::vector<std::shared_ptr<graph::Forest>>>& nodeAndTrees);

        solver::RuleReturnCode apply() override;

        void unapply() override;

        static bool checkIfContainedInRoot(graph::Node* node, std::shared_ptr<graph::Forest> forest);

        /// Check if for two nodes the B-Rule condition is reached.
        ///
        static int bCheck(graph::Node* node1, graph::Node* node2, std::shared_ptr<graph::Forest> forest);

        static std::shared_ptr<solver::AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                           const std::shared_ptr<Context>& context);
        [[nodiscard]]
        std::string name() const override;
    };

}  //namespace solver

#endif  //PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
