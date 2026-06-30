#ifndef PACE2026_CHAINREDUCTIONRULE_H
#define PACE2026_CHAINREDUCTIONRULE_H
#include "../Action/AddEdgeAction.hpp"
#include "../Action/DeleteEdgeAction.hpp"
#include "../Action/ShortenChainAction.hpp"
#include "AbstractRule.hpp"

#include <stack>
#include <vector>

namespace solver
{
    /// \brief This Rule removes a portion of the tree if a certain pattern of terminals and their parent nodes occours
    /// \see
    /// <a href="https://gitlab.informatik.uni-bremen.de/pace-2026/orga/-/wikis/Branching/ChainReductionRule">
    /// GitLab Documentation
    /// </a>
    class ChainReductionRule : public AbstractRule
    {
    protected:
        std::list<std::pair<unsigned int, unsigned int>> reducedChains;

        std::stack<ShortenChainAction> changes;

    public:
        /// \brief Chain Reduction Rule implementation that identifies the first chain out of the two forests given
        /// \param instance The problem instance
        /// \param context information about the instance and the solver state
        /// \param reducedChains
        /// \return The first chain between the two forests
        ChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
        const std::shared_ptr<Context>& context, const std::list<std::pair<unsigned int, unsigned int>>& reducedChains);

        /// \brief Apply the Chain Reduction rule onto the two Trees
        RuleReturnCode apply() override;

        /// \brief Restore the original state before the application of the chain reduction rule
        void unapply() override;

        /// \brief Determine if there are chains between the two trees.
        static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

        [[nodiscard]] std::string name() const override;

        [[nodiscard]] std::shared_ptr<AbstractRule> clone() const override;
    };


}
#endif  //PACE2026_CHAINREDUCTIONRULE_H
