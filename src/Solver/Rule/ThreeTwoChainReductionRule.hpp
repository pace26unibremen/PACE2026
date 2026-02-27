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
        std::pair<std::vector<unsigned int>,std::vector<std::shared_ptr<graph::Forest>>> twoChain;

        std::stack<solver::AbstractAction> changes;

    public:
      ThreeTwoChainReductionRule(
          const std::shared_ptr<graph::Instance>& instance, const std::shared_ptr<Context>& context,
          const std::pair<std::vector<unsigned int>, std::vector<std::shared_ptr<graph::Forest>>>& twoChain);
      int apply() override;

        void unapply() override;

        static std::shared_ptr<solver::AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                           const std::shared_ptr<Context>& context);
        [[nodiscard]]
        std::string name() const override;
    };

}  //namespace solver

#endif  //PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
