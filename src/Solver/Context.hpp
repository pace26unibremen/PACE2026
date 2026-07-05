#ifndef PACE2026_CONTEXT_HPP
#define PACE2026_CONTEXT_HPP

#include "../Graph/Instance.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

namespace solver
{
// forward declaration of \ref BranchingSolverConfiguration
struct BranchingSolverConfiguration;

/// \brief A context stores information about the instance and the state of the branching solver.
struct Context
{
    /// \brief The weight of the best solution, that the solver found so far.
    /// (i.e., this is mostly equal to the number of components in the agreement forest.)
    /// \see weightFunction
    /// Default value is +infinity
    float bestSolutionWeight = INFINITY;

    /// \brief The maximum size (number of trees) of the best solution, that the solver
    /// searches in a bounded depth search. \ref BranchingSolverConfiguration::boundedDephtSearch
    /// Default value is 1.
    unsigned int maxSolutionSize = 1;

    /// \brief The configuration of the branching solver.
    /// This should be set in the constructor of the branching solver.
    std::shared_ptr<BranchingSolverConfiguration> branchingSolverConfiguration = nullptr;

    /// \brief A set of all protected edges (edges that must never be cut).
    /// An edge is identified by the node it points to (the child-node).
    std::unordered_set<graph::Node*> protectedEdges = std::unordered_set<graph::Node*>();

    /// \brief The label of the terminal that annotates the root of a cluster.
    /// If the instance is not a cluster or is the root-cluster (and has no annotated root),
    /// then the label is 0.
    unsigned int clusterRootLabel = 0;

    std::function<float(std::shared_ptr<graph::Forest>)> weightFunction =
        [this](const std::shared_ptr<graph::Forest>& forest)
        {
            const float numberOfComponents = (float) forest->Roots().size();

            // We prefer a solution where the cluster root is a singelton
            // and this singleton is not a collapsed subtree.
            // We give an incentive for such solutions by subtractting
            // 0.5 from the weight.
            if (clusterRootLabel != 0)
            {
                auto rn = forest->LabelToTerminal()[clusterRootLabel];
                if (rn->parent == nullptr and rn->isTrueTerminal())
                {
                    return numberOfComponents - (float) 0.5;
                }
            }
            return numberOfComponents;
        };


    /// \brief A heuristic order of labels for solver to find applicable rules.
    /// It should be initialized by the solver.
    std::list<unsigned int> heuristicLabelOrder = {};

    // ---------------------------------------------------------------------- //
    // ---- Single-vertex-tree tracking (perf, see SingleVertexTreePropagationRule)
    // ---------------------------------------------------------------------- //
    // A label is a "single-vertex tree" (SVT) in a forest when its terminal node
    // is a childless root. SingleVertexTreePropagationRule reduces labels that are
    // an SVT in some but not all forests. Rather than rescanning the whole instance
    // on every solver iteration to find those labels, we maintain the information
    // incrementally: DeleteEdgeAction and CollapseSubtreeAction call
    // \ref adjustSingleVertexForNode on their do/undo, so applicability becomes an
    // O(1) check of whether \ref singleVertexMismatchCount is zero.

    /// \brief Per label, the number of forests in which it is currently an SVT.
    /// Index is the label; entry 0 is unused (labels start at 1).
    std::vector<uint16_t> singleVertexForestCount = {};

    /// \brief Number of labels that are an SVT in some but not all forests, i.e.
    /// with 0 < \ref singleVertexForestCount < \ref singleVertexNumForests. When
    /// this is non-zero \ref SingleVertexTreePropagationRule is applicable; its
    /// reduction set (those labels) is materialised on demand by a single scan of
    /// \ref singleVertexForestCount, which only happens when the rule actually fires.
    unsigned int singleVertexMismatchCount = 0;

    /// \brief Number of forests in the instance; set by \ref initSingleVertexTracking.
    unsigned int singleVertexNumForests = 0;

    /// \brief Whether the SVT tracking has been initialised from a full scan yet.
    bool singleVertexTrackingInitialised = false;

    /// \brief Adjusts the SVT forest-count of \p label by \p delta (+1 / -1) and
    /// keeps \ref singleVertexMismatchCount in sync.
    void adjustSingleVertexCount(unsigned int label, int delta)
    {
        if (label >= singleVertexForestCount.size())
        {
            singleVertexForestCount.resize(label + 1, 0);
        }
        const int oldCount = singleVertexForestCount[label];
        const int newCount = oldCount + delta;
        singleVertexForestCount[label] = static_cast<uint16_t>(newCount);

        const int forests = static_cast<int>(singleVertexNumForests);
        const bool wasMismatch = (oldCount > 0 and oldCount < forests);
        const bool isMismatch = (newCount > 0 and newCount < forests);
        if (isMismatch and not wasMismatch)
        {
            ++singleVertexMismatchCount;
        }
        else if (not isMismatch and wasMismatch)
        {
            --singleVertexMismatchCount;
        }
    }

    /// \brief Applies \ref adjustSingleVertexCount to every label represented by a
    /// (childless) terminal node — the set bits of its \c subtreeTerminals bitmask.
    void adjustSingleVertexForNode(const graph::Node* node, int delta)
    {
        unsigned int word = 0;
        for (uint64_t bitmask : node->subtreeTerminals)
        {
            while (bitmask != 0)
            {
                const unsigned int bit = static_cast<unsigned int>(__builtin_ctzll(bitmask));
                bitmask &= bitmask - 1;
                adjustSingleVertexCount(64u * word + bit + 1, delta);
            }
            ++word;
        }
    }

    /// \brief Initialises the SVT tracking from a single full scan of the instance.
    /// Called once, lazily, before the first applicability check of the branching run.
    void initSingleVertexTracking(const std::shared_ptr<graph::Instance>& instance)
    {
        singleVertexNumForests = static_cast<unsigned int>(instance->size());
        unsigned int maxLabel = 0;
        for (const auto& forest : *instance)
        {
            maxLabel = std::max(maxLabel, forest->LabelToTerminal().maxLabel());
        }
        singleVertexForestCount.assign(maxLabel + 1, 0);
        singleVertexMismatchCount = 0;
        for (const auto& forest : *instance)
        {
            for (const graph::Node* root : forest->Roots())
            {
                if (root->leftChild == nullptr)  // childless root == single-vertex tree
                {
                    adjustSingleVertexForNode(root, +1);
                }
            }
        }
        singleVertexTrackingInitialised = true;
    }
};

}  //namespace solver

#endif  //PACE2026_CONTEXT_HPP
