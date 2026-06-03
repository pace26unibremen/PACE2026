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
        graph::Node* node;

    public:

      ThreeTwoChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
                                 const std::shared_ptr<Context>& context, graph::Node* node);
      solver::RuleReturnCode apply() override;

        void unapply() override;

        static bool allBoolsSayTrue(std::vector<bool> list);

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
