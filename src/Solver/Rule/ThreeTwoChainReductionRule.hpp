//
// Created by kaufm on 12.02.2026.
//

#ifndef PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
#define PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
#include "AbstractRule.hpp"

namespace solver
{

    class ThreeTwoChainReductionRule : public AbstractRule
    {

        ThreeTwoChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
                                   const std::shared_ptr<Context>& context);
        int apply();
        void unapply();
        std::shared_ptr<solver::AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                           const std::shared_ptr<Context>& context);
        std::string name() const;
    };

}  //namespace solver

#endif  //PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
