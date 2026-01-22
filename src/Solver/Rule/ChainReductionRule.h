//
// Created by kaufm on 11.12.2025.
//

#ifndef PACE2026_CHAINREDUCTIONRULE_H
#define PACE2026_CHAINREDUCTIONRULE_H
#include "AbstractRule.hpp"
#include "../Action/DeleteEdgeAction.hpp"
#include <vector>

namespace solver
{
    class ChainReductionRule : public AbstractRule
    {
    protected:
        std::shared_ptr<graph::Forest> T1;
        std::shared_ptr<graph::Forest> T2;
        std::vector<std::vector<std::vector<int>>> chains;

    public:
        ChainReductionRule(const std::shared_ptr<graph::Forest>& T1, const std::shared_ptr<graph::Forest>& T2,
                           const std::vector<std::vector<std::vector<int>>>& chains);
        void apply() override;

        void unapply() override;

        static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Forest>& T1,
            const std::shared_ptr<graph::Forest>& T2);
    };


}
#endif  //PACE2026_CHAINREDUCTIONRULE_H
