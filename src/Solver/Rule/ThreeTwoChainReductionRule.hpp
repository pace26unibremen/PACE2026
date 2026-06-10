//
// Created by kaufm on 12.02.2026.
//

#ifndef PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
#define PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
#include "AbstractRule.hpp"
#include <vector>
#include <stack>
#include "../Action/DeleteEdgeAction.hpp"


namespace solver
{

    class ThreeTwoChainReductionRule : public AbstractRule
    {
    protected:
        /// \brief The nodes which were identified to be the B-Node for each forest within the instance
        std::vector<graph::Node*> nodes;
        /// \brief A stack entailing all edge deletion actions between the B-Nodes and their respective forests
        std::stack<DeleteEdgeAction> changes;

    public:
        /// \brief Constructor for the B-Rule
        /// \see Whidden et al. 2013
        /// \param instance The instance which is to be analyzed
        /// \param context Information storage for the branchen
        /// \param nodes The B-Nodes identified to be removed in accordance to the B-Rule
        ThreeTwoChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
                                 const std::shared_ptr<Context>& context,
                                 std::vector<graph::Node*> nodes);

        /// \brief Applies the B-Rule onto all forests within the instance
        RuleReturnCode apply() override;

        /// \brief Reverses the B-Rule appliance on the instance
        void unapply() override;

        /// \brief Helper Function used to identify if all elements of the parameter list entail true
        /// \param list Boolean vector
        static bool allBoolsSayTrue(const std::vector<bool>& list);

        /// \brief Helper Function used to identify if a node is contained within the root list of the forest parameter
        /// \param node The node who's to be checked for
        /// \param forest The forest instance in which the node resides
        static bool checkIfContainedInRoot(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest);

        /// Helper function which checks if for two nodes the B-Rule condition is reached.
        /// \param node1 the A-Node
        /// \param node2 the C-Node
        /// \param forest The Forest instance which contains both node1 and node2
        /// \returns A Integer corresponding to the type of B-Rule-Structure identified given these parameters
        static int bCheck(const graph::Node* node1, const graph::Node* node2,
                          const std::shared_ptr<graph::Forest>& forest);

        /// \brief Main Checking function that attempts to identify if the B-Rule is true for a node across all forests
        /// within the instance.
        /// \param instance The Instance in which all forests are listed
        /// \param context The Information carried across rules for the instance parameter
        static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                           const std::shared_ptr<Context>& context);
        [[nodiscard]]
        std::string name() const override;
    };

}  //namespace solver

#endif  //PACE2026_THREETWOCHAINREDUCTIONRULE_HPP
