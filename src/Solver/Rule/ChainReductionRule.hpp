#ifndef PACE2026_CHAINREDUCTIONRULE_H
#define PACE2026_CHAINREDUCTIONRULE_H
#include "AbstractRule.hpp"
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
        /// \brief First Chain found within two trees of a problem instance
        std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees;

        /// \brief Adresss storage for all deleted Nodes during the chain removal process in T1
        std::vector<graph::Node> deletedNodesT1;

        /// \brief Address storage for all deleted Nodes during the chain removal process in T2
        std::vector<graph::Node> deletedNodesT2;

        /// \brief Legacy for unapply
        std::pair<graph::Node*,graph::Node*> parentToXN;

        /// \brief The Labels for the Terminals from the removed portion of the chain.
        std::pair<std::unordered_map<unsigned int, graph::Node>,std::unordered_map<unsigned int, graph::Node>>
        chainLabels;


    public:
        /// \brief Chain Reduction Rule implementation that identifies the first chain out of the two forests given
        /// \param instance The problem instance
        /// \param chainWithTrees First found chain between two trees within the problem instance
        /// \param context information about the instance and the solver state
        /// \return The first chain between the two forests
        ChainReductionRule(const std::shared_ptr<graph::Instance>& instance,
        std::pair<std::vector<std::vector<graph::Node*>>,std::vector<std::shared_ptr<graph::Forest>>> chainWithTrees,
        const std::shared_ptr<Context>& context);

        /// \brief Helper function that checks if a node is contained within a list or not
        /// \param node The node to be checked for within the list
        /// \param list The list of nodes which is checked
        /// \return Boolean of if the node is within the list or not
        static bool isNodeInNodeVector(const graph::Node* node, const std::vector<graph::Node*>& list);

        /// \brief Helper function that deletes the connections between the parameter node with the terminal child as
        /// well as removing them from the tree they're in. Copies the node beforehand into a constructor list.
        /// \param node The Node to be edited
        void removeConnectionOfTerminalNode(graph::Node* node, std::shared_ptr<graph::Forest>& forest);
        void storeNode(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest);

        /// \brief Apply the Chain Reduction rule onto the two Trees
        RuleReturnCode apply() override;

        /// \brief Restore the original state before the application of the chain reduction rule
        void unapply() override;

        /// \brief Determine if there are chains between the two trees.
        static std::shared_ptr<AbstractRule> isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                      const std::shared_ptr<Context>& context);

        std::string name() const override;
    };


}
#endif  //PACE2026_CHAINREDUCTIONRULE_H
