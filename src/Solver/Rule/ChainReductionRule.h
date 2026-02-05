//
// Created by kaufm on 11.12.2025.
//

#ifndef PACE2026_CHAINREDUCTIONRULE_H
#define PACE2026_CHAINREDUCTIONRULE_H
#include "AbstractRule.hpp"
#include "../Action/DeleteNodeActionInChains.h"
#include "../Action/DeleteEdgeAction.hpp"
#include <vector>
#include <stack>

namespace solver
{
    class ChainReductionRule : public AbstractRule
    {
    protected:
        /// \brief First Tree
        std::shared_ptr<graph::Forest> T1;

        /// \brief Second Tree
        std::shared_ptr<graph::Forest> T2;

        /// \brief First Chain found within the two trees
        std::vector<std::vector<int>> chain;

        /// \brief Amount of changes done
        std::stack<solver::DeleteNodeActionInChains> changes;

    public:
        /// \param T1 First Tree
        /// \param T2 Second Tree
        /// \param T3 Chains between those two trees in the paramater

       ChainReductionRule(const std::shared_ptr<graph::Forest>& T1, const std::shared_ptr<graph::Forest>& T2,
                         const std::vector<std::vector<int>>& chains);
      /// \brief Apply the Chain Reduction rule onto the two Trees
        void apply() override;
        /// \brief Restore the original state before the application of the chain reduction rule
        void unapply() override;

        /// \brief Determine if there are chains between the two trees.
        static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Forest>& T1,
            const std::shared_ptr<graph::Forest>& T2);
    };


}
#endif  //PACE2026_CHAINREDUCTIONRULE_H
