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

        /// \brief Indices for all nodes prior to editing the first tree of the instance
        /// Left pair element denotes the indicies for each node, right contains the bitmask list for each node
        std::pair<std::vector<std::vector<int>>, std::vector<std::vector<uint64_t>>> deletedNodesT1Indices;

        /// \brief Indices for all nodes prior to editing the second tree of the instance
        /// Left pair element denotes the indicies for each node, right contains the bitmask list for each node
        std::pair<std::vector<std::vector<int>>, std::vector<std::vector<uint64_t>>> deletedNodesT2Indices;

        /// \brief Indices for all root nodes prior to applying the rule for the first tree
         std::pair<std::vector<std::vector<int>>, std::vector<std::vector<uint64_t>>> rootsT1Indices;

        /// \brief Indices for all root nodes prior to applying the rule for the second tree
         std::pair<std::vector<std::vector<int>>, std::vector<std::vector<uint64_t>>> rootsT2Indices;

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

        /// \brief Helper function that stores all root nodes within a forest
        /// \param forest The forest whose root nodes should be stored
        void storeRootNodes(const std::shared_ptr<graph::Forest>& forest);

        /// \brief Helper function which saves the connections of a node to his associated nodes
        /// within the corresponding forest through saving the indicies to the node address within the forest node list
        /// \param node The node whose connections should be saved
        /// \param forest The forest from which the node originates
        void storeEditedNodeIndices(const graph::Node* node, const std::shared_ptr<graph::Forest>& forest);

        /// \brief Helper function that deletes the connections between the parameter node with the terminal child as
        /// well as removing them from the tree they're in. Copies the node beforehand into a constructor list.
        /// \param node The Node to be edited
        /// \param forest The forest from which the node originates.
        void removeConnectionOfTerminalNode(graph::Node* node, std::shared_ptr<graph::Forest>& forest);

        /// \brief Helper function that determinates on which side from the viewpoint of the root node
        /// the parameter node is within its tree.
        /// \param node The node from which its side position is to be determined
        /// \return 0 if the node is on the left side, 1 if the node is on the right side
        int determineSideOfChain(graph::Node* node);

        std::vector<uint64_t> eraseTerminals(std::vector<uint64_t> target, std::vector<uint64_t> toBeErased);

        static int identifyDistanceToRoot(graph::Node* node, std::shared_ptr<graph::Forest>& forest);

        static std::vector<int> structureToRoot(graph::Node* node, std::shared_ptr<graph::Forest>& forest);

        /// \brief Helper function that updates the subtree terminal bitmasks of the nodes.
        void updateSubtreeTerminals();

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
