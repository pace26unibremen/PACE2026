#ifndef PACE2026_CONTEXT_HPP
#define PACE2026_CONTEXT_HPP

#include "../Graph/Instance.hpp"

#include <algorithm>
#include <atomic>
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

    // -----------------------------------------------------------------------
    // Lower-bound track: certified early-exit against the "#a {a} {b}" line.
    //
    // On the lower-bound track a submission of size U is valid iff
    // U <= floor(a * k*) + b, where a,b come from the instance's "#a {a} {b}"
    // line and k* (the true optimum) is never given. Because any lower bound
    // L <= k* satisfies floor(a*L)+b <= floor(a*k*)+b (a >= 1), stopping the
    // search as soon as the incumbent U satisfies U <= floor(a*L)+b yields a
    // *provably valid* answer. Whether the search actually uses this is gated by
    // BranchingSolverConfiguration::certifiedEarlyExit (so a/b being present does
    // not by itself force a lower-bound strategy). a/b default to -1 = "unset".
    // -----------------------------------------------------------------------

    /// \brief The validity multiplier a from the "#a {a} {b}" line (a >= 1). -1 if unset.
    double a = -1.0;

    /// \brief The validity offset b from the "#a {a} {b}" line (b >= 0). -1 if unset.
    int b = -1;

    /// \brief The certified acceptance threshold floor(a*L)+b for a certified lower bound L <= k*.
    /// The branching search may stop and emit its incumbent the moment the incumbent's size is <= this,
    /// when \ref BranchingSolverConfiguration::certifiedEarlyExit is on. Defaults to -1: since a real
    /// threshold is always >= 1 (L >= 1, a >= 1, b >= 0), the default can never fire, so an instance
    /// without a computed bound never certifies even if the flag is on.
    ///
    /// Atomic because the lower-bound track may raise it from a background thread (the tighter 2-approx
    /// dual is computed concurrently with the search; when it lands it raises this threshold and the
    /// search picks it up on its next iteration). All accesses are single loads/stores, so relaxed
    /// ordering suffices, but the default (seq_cst) is used for simplicity -- the read is off the hot path.
    std::atomic<long> certifiedThreshold = -1;

    /// \brief floor(a * value) + b for a certified lower bound `value` (= L, in component units).
    /// \param value a non-negative integer.
    /// \return floor(a * value) + b.
    [[nodiscard]] long certifiedCeiling(long value) const
    {
        // a is a binary double, so a*value can land just below an exact integer (e.g. 1.15*20 = 23
        // evaluates to 22.999...). This is validity-determining: an under-count silently rejects a valid
        // answer, an over-count could accept an invalid one. a is given with at most two decimal digits,
        // so a*value is a multiple of 0.01 and is never within 1e-9 *below* a true integer unless it *is*
        // that integer; adding a 1e-9 nudge before flooring therefore recovers the exact floor without
        // ever over-counting past a genuine 0.01 gap.
        return static_cast<long>(std::floor(a * static_cast<double>(value) + 1e-9)) + b;
    }

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
