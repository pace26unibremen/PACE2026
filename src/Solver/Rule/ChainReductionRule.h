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
        /// \brief First Chain found within two trees of a problem instance
        std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees;

        /// \brief Amount of changes done
        std::stack<solver::DeleteNodeActionInChains> changes;

    public:
        /// \brief Chain Reduction Rule implementation that identifies the first chain out of the two forests given
        /// \param instance The problem instance
        /// \param chainWithTrees First found chain between two trees within the problem instance
        /// \param context information about the instance and the solver state
        /// \return The first chain between the two forests
        ChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
        const std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>>
        chainWithTrees,
        const std::shared_ptr<Context>& context);

        /// \brief Apply the Chain Reduction rule onto the two Trees
        int apply() override;

        /// \brief Restore the original state before the application of the chain reduction rule
        void unapply() override;

        /// \brief Determine if there are chains between the two trees.
        static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

        std::string name() const;
    };


}
#endif  //PACE2026_CHAINREDUCTIONRULE_H
