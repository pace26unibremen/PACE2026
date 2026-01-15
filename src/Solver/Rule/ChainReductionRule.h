//
// Created by kaufm on 11.12.2025.
//

#ifndef PACE2026_CHAINREDUCTIONRULE_H
#define PACE2026_CHAINREDUCTIONRULE_H
#include "AbstractRule.hpp"

#include <iosfwd>
#include <list>
#include <vector>

namespace solver
{
    class ChainReductionRule : public AbstractRule
    {
        std::shared_ptr<graph::Forest> T1;
        std::shared_ptr<graph::Forest> T2;
        std::vector<std::list<int>> chains;

        ChainReductionRule(const std::shared_ptr<graph::Forest>& T1, const std::shared_ptr<graph::Forest>& T2,
                           const std::vector<std::list<int>>& chains);
        void apply();

        void unapply();

        std::shared_ptr<solver::AbstractRule> isApplicable(const std::shared_ptr<graph::Forest>& T1,
    const std::shared_ptr<graph::Forest>& T2);
    };


}
#endif  //PACE2026_CHAINREDUCTIONRULE_H
